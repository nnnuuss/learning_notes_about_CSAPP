#include "cachelab.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
int hit_count, miss_count, eviction_count;
typedef struct {
    int valid_bit,
	tag,
	LRU_counter;
}cache_line, *cache_set, **cache;

int h, v, s, E, b, S;
char t[100];
cache c = NULL;

void Print_message(){
	printf("Usage: ./csim-ref [-hv] -s <num> -E <num> -b <num> -t <file>\n");
	printf("Options:\n");
	printf("  -h         Print this help message.\n");
	printf("  -v         Optional verbose flag.\n");
	printf("  -s <num>   Number of set index bits.");
	printf("  -E <num>   Number of lines per set.\n");
	printf("  -b <num>   Number of block offset bits.\n");
	printf("  -t <file>  Trace file.\n\n");
	printf("Examples:\n");
	printf("  linux>  ./csim-ref -s 4 -E 1 -b 4 -t traces/yi.trace\n");
	printf("  linux>  ./csim-ref -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

void init_cache(){
	S=(1<<s);
	c = (cache)malloc(sizeof(cache_set)*S);
	for (int i = 0; i < S; ++i){
		c[i]=(cache_set)malloc(sizeof(cache_line)*E);
		for (int j = 0; j < E; ++j){
			c[i][j].LRU_counter=c[i][j].valid_bit=0;
			c[i][j].tag=-1;		//有部分数据的tag位为0，所以需要初始化成-1
		}
	}
}

void update_counter(){
	for (int i = 0; i < S; ++i){
		for (int j = 0; j < E; ++j){
			if (c[i][j].valid_bit==1)c[i][j].LRU_counter++;
		}
	}
}

void update(unsigned int address){
	int tar_tag = address >> (b+s);
	int tem = 0;
	for (int i = 0; i < s; ++i){
		tem+=(1<<i);
	}
	int setindex = (address>>b)&tem;
	for (int i = 0; i < E; ++i){
		if (c[setindex][i].tag==tar_tag){
			c[setindex][i].LRU_counter=0;
			hit_count++;		//匹配
			return;
		}
	}
	for (int i = 0; i < E; ++i){
		if (c[setindex][i].valid_bit==0){
			c[setindex][i].valid_bit=1;
			c[setindex][i].LRU_counter=0;
			c[setindex][i].tag=tar_tag;
			miss_count++;
			return;
		}
	}
	int max_counter = 0, max_index = 0;
	for (int i = 0; i < E; ++i){
		if (c[setindex][i].LRU_counter > max_counter){
			max_counter=c[setindex][i].LRU_counter;
			max_index=i;
		}
	}
	c[setindex][max_index].tag=tar_tag;
	c[setindex][max_index].LRU_counter=0;
	eviction_count++;
	miss_count++;
}

void parse_trace(){
	init_cache();
	FILE * pFile;
	pFile = fopen(t, "r");
	char identifier;
	unsigned address;
	int size;
	while(fscanf(pFile,"%c %x, %d", &identifier, &address, &size)>0){
		switch (identifier)
		{
		case 'I':
			continue;
		case 'L':
			update(address);
			break;
		case 'M':
			update(address);
		case 'S':
			update(address);
		default:
			break;
		}
		update_counter();
	}
	fclose(pFile);
	for (int i = 0; i < S; ++i)
		free(c[i]);
	free(c);
}

int main(int argc, char* argv[])
{
    int opt;
    while(-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))){
    	switch(opt){
			case 'h':
			Print_message();
			break;
			case 'v':
			Print_message();
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
			Print_message();
			break;
		}
    }
	if (s<0||E<0||b<0){
		return -1;
	}
	parse_trace();
	printSummary(hit_count, miss_count, eviction_count);

    return 0;
}
