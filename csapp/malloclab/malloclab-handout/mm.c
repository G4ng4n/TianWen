/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// 书中给出的一些宏，用于操作freelist
// 带footer和header的chunk

#define WSIZE 4
#define DSIZE 8
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */
#define MINBLOCKSIZE 16

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define PACK(size, alloc) ((size) | (alloc)) /* Pack a size and allocated bit into a word */

#define GET(p)            (*(unsigned int *)(p))
#define PUT(p, val)       (*(unsigned int *)(p) = (val))

#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

#define HDRP(ptr) ((char *)(ptr) - WSIZE)
#define FTRP(ptr) ((char *)(ptr) + GET_SIZE(HDRP(ptr)) - DSIZE)

#define NEXT_BLKP(ptr) ((char *)(ptr) + GET_SIZE((char *)(ptr) - WSIZE))
#define PREV_BLKP(ptr) ((char *)(ptr) - GET_SIZE((char *)(ptr) - DSIZE))

#define PRED_PTR(ptr) ((char *)(ptr))
#define SUCC_PTR(ptr) ((char *)(ptr) + WSIZE)

#define PRED(ptr) (*(char **)(ptr))
#define SUCC(ptr) (*(char **)(SUCC_PTR(ptr)))

static void* extend_heap(size_t words);
static void* coalesce(void *bp);
static void* first_fit(size_t asize);
static void split(void* bp, size_t asize);
static void replace(void* bp, size_t asize);


static char *heap_listp; // 指向freed list的第一个chunk
static char *last_free;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    // 初始化freelist, 创建特殊的头尾空闲块来
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1) return -1;
    PUT(heap_listp, 0); // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); //prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); //prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); //epilogue header
    heap_listp += (2 * WSIZE);

    last_free = heap_listp;

    if (!extend_heap(CHUNKSIZE/WSIZE))
        return -1;
    return 0;
}

// 开辟新的heap空间
static void* extend_heap(size_t words)
{
    char* bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
        return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0)); //free block header
    PUT(FTRP(bp), PACK(size, 0)); //free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header

    return coalesce(bp); //coalesce if the previous block was free
}

// 合并前后chunk
static void* coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc)
        return bp;
    else if (prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    last_free = (char *)bp;
    return bp;
}

// 切割chunk
// 如果分配后原chunk剩余部分大于MIN_CHUNK_SIZE则切分chunk，否则连同可能存在的额外字节空间一并包括在内
static void split(void* bp, size_t new_size){
    size_t old_size = GET_SIZE(HDRP(bp));
    if ((old_size - new_size) >= MINBLOCKSIZE) {
        PUT(HDRP(bp), PACK(new_size, 1));
        PUT(FTRP(bp), PACK(new_size, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(old_size-new_size, 0));
        PUT(FTRP(bp), PACK(old_size-new_size, 0));
    }
}

// 在找到的freed chunk中重新设置新块的header和footer，以及相关的alloc位
static void replace(void* bp, size_t new_size){
    size_t old_size = GET_SIZE(HDRP(bp));
    PUT(HDRP(bp), PACK(old_size, 1));
    PUT(FTRP(bp), PACK(old_size, 1));
    split(bp, new_size);
}

// 利用firstfit原则查找已经释放的堆块
static void *first_fit(size_t chunk_size){
    for (char* bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= chunk_size){
            return bp;
        }
    }
    return NULL;
}

static void *next_fit(size_t chunk_size){
    // printf("call next_fit!\n");
    
    for (char* bp = last_free; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)){
        if (!GET_ALLOC(HDRP(bp)) && GET_SIZE(HDRP(bp)) >= chunk_size){
            return bp;
        }
    }
    return NULL;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize; // adjusted block size
    size_t extend_size; // amount to extend heap if no fit
    char* bp;

    if (size == 0) return NULL;

    // adjusted block size to include overhead and alignment reqs
    if (size <= DSIZE)
        asize = MINBLOCKSIZE;
    else
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);

    if ((bp = next_fit(asize))){
        replace(bp, asize);
        return bp;
    }

    // 未发现适合回收的空闲内存，开辟新的堆内存
    extend_size = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extend_size / WSIZE)) == NULL)
        return NULL;
    replace(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE((HDRP(ptr)));
    // 设置标志位
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    // 释放后立即合并
    coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t old_size;

    newptr = mm_malloc(size);
    if (newptr == NULL) return NULL;
    old_size = GET_SIZE(HDRP(oldptr));
    if (old_size < size)
      size = old_size;
    memcpy(newptr, oldptr, size - WSIZE);
    mm_free(oldptr);
    return newptr;
}














