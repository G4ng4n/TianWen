#include "cachelab.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <getopt.h>
#include <string.h>

int h,v,s,E,b,S; // 这个是我们模拟的参数，为了方便在函数里调用，设置成全局

int hit_count , 
    miss_count , 
    eviction_count;  // 三个在 printSummary 函数中的参数，需要不断更新
	
char t[1000]; // 存 getopt 中选项内容，表示的是验证中需使用的trace文件名

typedef struct{
    int valid_bits;
    int tag;
    int stamp;
}cache_line, *cache_set, **cache;  // cache 模拟器的结构。由合法位、标记位和时间戳组成

cache CACHE = NULL;  // 声明一个空的结构体类型二维数组

void printUsage()
{
    printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n"
            "Options:\n"
            "  -h         Print this help message.\n"
            "  -v         Optional verbose flag.\n"
            "  -s <num>   Number of set index bits.\n"
            "  -E <num>   Number of lines per set.\n"
            "  -b <num>   Number of block offset bits.\n"
            "  -t <file>  Trace file.\n\n"
            "Examples:\n"
            "  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n"
            "  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init_cache()
{
    CACHE = (cache)malloc(sizeof(cache_set) * S); 
	for(int i = 0; i < S; ++i)
	{
		CACHE[i] = (cache_set)malloc(sizeof(cache_line) * E);
		for(int j = 0; j < E; ++j)
		{
			CACHE[i][j].valid_bits = 0;
			CACHE[i][j].tag = -1;
			CACHE[i][j].stamp = -1;
		}
	}
}

void update(unsigned int address)
{
    int setindex_add = (address >> b) & ((-1U) >> (64 - s));
	int tag_add = address >> (b + s);
	
	int max_stamp = INT_MIN;
	int max_stamp_index = -1;

	for(int i = 0; i < E; ++i)
	{
		if(CACHE[setindex_add][i].tag == tag_add)
		{
			CACHE[setindex_add][i].stamp = 0;
			++hit_count;
			return ;
		}
	}
	
	for(int i = 0; i < E; ++i)
	{
		if(CACHE[setindex_add][i].valid_bits == 0)
		{
			CACHE[setindex_add][i].valid_bits = 1;
			CACHE[setindex_add][i].tag = tag_add;
			CACHE[setindex_add][i].stamp = 0;
			++miss_count;
			return ;
		}
	}

	++eviction_count;
	++miss_count;
	
	// LRU
	for(int i = 0; i < E; ++i)
	{
		if(CACHE[setindex_add][i].stamp > max_stamp)
		{
			max_stamp = CACHE[setindex_add][i].stamp;
			max_stamp_index = i;
		}
	}
	CACHE[setindex_add][max_stamp_index].tag = tag_add;
	CACHE[setindex_add][max_stamp_index].stamp = 0;
	return ;
}


void update_stamp()
{
	for(int i = 0; i < S; ++i)
		for(int j = 0; j < E; ++j)
			if(CACHE[i][j].valid_bits == 1)
				++CACHE[i][j].stamp;
}


void parse_trace()
{
	FILE* fp = fopen(t, "r");
	if(fp == NULL)
	{
		printf("open error");
		exit(-1);
	}
	
	char op;
	unsigned int address;
	int size;               // 大小
	while(fscanf(fp, " %c %xu,%d\n", &op, &address, &size) > 0)
	{
		switch(op)
		{
			case 'I': 
				continue;
			case 'L':
				update(address);
				break;
			case 'M':
				update(address);  // miss的话还要进行一次storage
			case 'S':
				update(address);
		}
		update_stamp();	//更新时间戳
	}
	fclose(fp);
	for(int i = 0; i < S; ++i)
		free(CACHE[i]);
	free(CACHE);            // malloc 完要记得 free 并且关文件
}

int main(int argc, char* argv[])
{
	h = 0; 
	v = 0; 
	hit_count = miss_count = eviction_count = 0;
	int opt;
        
    // getopt 第三个参数中，不可省略的选项字符后要跟冒号，这里h和v可省略
	while(-1 != (opt = (getopt(argc, argv, "hvs:E:b:t:"))))
	{
		switch(opt)
		{
			case 'h':
				h = 1;
				printUsage();
				break;
			case 'v':
				v = 1;
				printUsage();
				break;
			case 's':
				s = atoi(optarg);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				break;
			case 't':
				strcpy(t, optarg);
				break;
			default:
				printUsage();
				break;
		}
	}
	
	if(s<=0 || E<=0 || b<=0 || t==NULL) // 如果选项参数不合格就退出
	        return -1;
	S = 1 << s;                // S=2^s
	
	FILE* fp = fopen(t, "r");
	if(fp == NULL)
	{
		printf("open error");
		exit(-1);
	}
	
	init_cache();  // 初始化cache
	parse_trace(); // 更新最终的三个参数

    printSummary(hit_count, miss_count, eviction_count);
    
    return 0;
}