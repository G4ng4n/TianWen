/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Rereplace this header comment with your own header
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

#define BINS_NUM 16

/* Basic constants and macros */
#define WSIZE 4 /* Word and header/footer size (bytes) */
#define DSIZE 8 /* Double word size (bytes) */
#define CHUNKSIZE (1<<12) /* Extend heap by this amount (bytes) */
#define MINCHUNKSIZE 32

#define MAX(x, y) ((x) > (y)? (x) : (y))
#define MIN(x, y) ((x) < (y)? (x) : (y))
/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and t_listvious blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* for explict linkedlist */
#define FDP(ptr) ((char *)(ptr))
#define BKP(ptr) ((char *)(ptr) + WSIZE)

#define FD(ptr) (*(char **)(ptr))
#define BK(ptr) (*(char **)(BKP(ptr)))


#define SET_PTR(p, ptr) (*(unsigned int *)(p) = (unsigned int)(ptr))

static void *extend_heap(size_t words);
static void replace(void *bp, size_t asize);
static void *coalesce(void *bp);
static void insert(void *bp, size_t size);
static void delete(void *bp);
void *bins[BINS_NUM];
char *heap_listp;

int mm_init(void)
{
    int i;
    for(i=0;i<BINS_NUM;i++){
        bins[i] = NULL;
    }
    // 初始化freelist, 创建特殊的头尾空闲块来
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1) return -1;
    PUT(heap_listp, 0); // alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); //prologue header
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); //prologue footer
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); //epilogue header
    heap_listp += (2 * WSIZE);

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

static void insert(void* bp, size_t size){
    int bin_num=0;
    size_t i;
    for(i=size; i>1 && bin_num<BINS_NUM-1; i>>=1){
        bin_num++;
    }
    //对应大小的分离链表
    char* list = bins[bin_num];
    //前一个节点
    char* t_list = NULL;

    // 遍历，找到适合插入的点
    while(list && size>GET_SIZE(HDRP(list))){
        t_list = list;
        list = BK(list);
    }

    if(!list && !t_list){
        //链表为空，直接放入
        bins[bin_num] = bp;
        SET_PTR(FDP(bp), NULL);
        SET_PTR(BKP(bp), NULL);
    }else if(!list && t_list){
        // 插入到末尾
        SET_PTR(FDP(bp), t_list); // 设置当前块的前驱指针
        SET_PTR(BKP(bp), NULL); // 设置当前块的后继指针
        SET_PTR(BKP(t_list), bp); // 设置前驱块的后继指针
    }else if(!t_list){
        // 插入到表头
        bins[bin_num] = bp;
        SET_PTR(FDP(list), bp); // 设置原表头块的前驱指针
        SET_PTR(BKP(bp), list); // 设置当前块的后继指针
        SET_PTR(FDP(bp), NULL); // 设置当前块的前驱指针
    }else{
        //设置前驱、后继、前驱块后继指针、后继块前驱指针
        SET_PTR(FDP(bp), t_list);
        SET_PTR(BKP(bp), list);
        SET_PTR(FDP(list), bp);
        SET_PTR(BKP(t_list), bp);
    }
}

static void delete(void* bp){
    size_t size = GET_SIZE(HDRP(bp));
    size_t i;
    int bin;
    for(i = size; bin < BINS_NUM-1 && i>1; i>>=1){
        // 计算所属链表
        bin++;
    }
    // 解除链表块前驱后继对删除块的引用
    if(FD(bp) == NULL) {
        // 链表首块
        bins[bin] = BK(bp);
        if(BK(bp)){
            SET_PTR(FDP(BK(bp)), NULL);
        }
    }else if(!BK(bp)){
        // 链表尾块
        SET_PTR(BKP(FD(bp)), NULL);
    }else{
        // FD->bk = BK, BK->fd = FD
        SET_PTR(BKP(FD(bp)), BK(bp));
        SET_PTR(FDP(BK(bp)), FD(bp));
    }
}

static void* coalesce(void *bp)
{
    size_t prev = GET_ALLOC(HDRP(PREV_BLKP(bp)));
    size_t next = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev && !next){
        // 只有后一块释放
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete(NEXT_BLKP(bp)); // 删除释放块，重新利用
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev && next){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    else if(!(prev||next)){
        // 合并前后块
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        delete(PREV_BLKP(bp));
        delete(NEXT_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    insert(bp, size);
}

static void replace(void* bp, size_t new_size){
    size_t old_size = GET_SIZE(HDRP(bp));
    delete(bp); // 从freelist中先删除chunk
    if(old_size-new_size < (1<<5)){
        // 剩余大小不足以分割新的freed chunk
        PUT(HDRP(bp),PACK(old_size,1));
        PUT(FTRP(bp),PACK(old_size,1));
    }else{
        // 剩余大小足够，切割出两块，后一块插入空闲链表
        PUT(HDRP(bp),PACK(new_size,1));
        PUT(FTRP(bp),PACK(new_size,1));
        PUT(HDRP(NEXT_BLKP(bp)),PACK(old_size - new_size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(old_size - new_size,0));
        insert(NEXT_BLKP(bp),old_size - new_size);
    }
}

void mm_free(void *ptr)
{
    size_t size = GET_SIZE((HDRP(ptr)));
    // 设置标志位
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    // 释放后立即合并
    coalesce(ptr);
}

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

void *mm_malloc(size_t size)
{
    size_t asize; // 实际需要的大小（对齐、最小块）
    size_t extend_size; // no fit申请新的空间大小
    char* bp = NULL; // 返回的指针
    char* list = NULL;

    if (size == 0) return NULL;

    // 1.搜索bins
    int bin_num = 0;
    asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    size_t i;

    // 2.选择对应大小范围的bin

    for(i = asize, bin_num=0; bin_num<BINS_NUM; bin_num++, i>>=1){
        // 3.first-fit找到合适大小的chunk
        // 首先简单判断筛选大致范围
        if((i>1) || bins[bin_num] == NULL) continue;
        
        for(list = bins[bin_num]; list; list=BK(list)){
            if(GET_SIZE(HDRP(list)) < asize) continue;
            bp = list;
            break;
        }
        if(bp) break;
    }
    
    if(!bp){
        // 未发现适合回收的空闲内存，开辟新的堆内存
        extend_size = MAX(asize, CHUNKSIZE);
        if ((bp = extend_heap(extend_size / WSIZE)) == NULL)
            return NULL;
    }
    replace(bp, asize);
    return bp;
}