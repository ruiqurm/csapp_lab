#include "cachelab.h"
#include  <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#define CACHE_NULL_VALUE -1

typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef char bool;

typedef struct{
    int valid;
    uint64_t tag;
    unsigned stamp;
}CacheLine;
typedef struct{
    CacheLine** mem;
    uint32_t b,s,E;
}Cache;
typedef struct{
    int hits;
    int misses;
    int evictions;
}statistic;

void visit(Cache* cache,uint64_t address,statistic* stat){
    uint32_t s,b,E;
    s = cache->s;
    b = cache->b;
    E = cache->E;
    CacheLine** mem = cache->mem;


    uint64_t tag = address >> (s+b);
    uint32_t set = (address >> s) & ((-1u)>>(sizeof(address)*8 - s));
    uint32_t empty = -1;
    uint32_t evict = 0;
    for(int i = 0;i<E;i++){
        CacheLine** data = &mem[set];
        if (data[i]->valid){
            if(data[i]->tag ==tag){
                stat->hits++;
                data[i]->stamp = 0;
                return;
            }else{
                data[i]->stamp++;
                if(data[evict]->stamp <=data[i]->stamp){
                    evict = i;
                }
            }
        }else{
            empty = i;
        }
    }
    stat->misses++;
   // 没有退出说明miss

    if (empty!=-1){
        mem[set][empty].valid = 1;
        mem[set][empty].tag = tag;
        mem[set][empty].stamp = 0;
    }else{
        mem[set][evict].valid = 1;
        mem[set][evict].tag = tag;
        mem[set][evict].stamp = 0;
        stat->evictions++;
    }


}


int main(int argc,char**argv){
    int opt,s,b,E;
    FILE	*	pFile;	//pointer	to	FILE	object	
    while(-1 != (opt = getopt(argc, argv,"s:E:b:t:"))){
        switch(opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
            case 't':
                pFile = fopen	(optarg,"r");
            default:
                printf("wrong argument\n");
                break;
            }
    }
    Cache cache;
    cache.s = s;cache.b = b;cache.E = E;
    int S= 1<<s;
    cache.mem = (CacheLine**)malloc(sizeof(CacheLine*)*S);
    for(int i=0;i<S;i++){
        cache.mem[i] = (CacheLine*) malloc(sizeof(CacheLine)*E);
        for(int j=0;j<E;j++){
            cache.mem[i][j].valid = 0;
            cache.mem[i][j].tag = cache.mem[i][j].stamp = CACHE_NULL_VALUE;
        }
    }
    statistic stat;
    stat.hits = 0;stat.evictions = 0;stat.misses = 0;
    char	identifier;	
    unsigned	address;	
    int	size;		
    while(fscanf(pFile,"%c %x,%d",	&identifier,	&address,	&size)>0)	
    {	
        switch (identifier){
        case 'I':
            break;
        case 'L': // load
            visit(&cache,address,&stat);
            break;
        case 'S': // save
            visit(&cache,address,&stat);
            break;
        case 'M':  // modify
            visit(&cache,address,&stat);
            stat.hits++;
            // modify 多一次访问拿数据
            break;
        default:
            // printf(usage, argv[0]);
            break;
        }
    }
    free(cache.mem);
    printSummary(stat.hits, stat.misses, stat.evictions);
    return 0;
}
