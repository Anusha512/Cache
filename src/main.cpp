#include <iostream>
#include "cache.h"
using namespace std;

Cache cache;
int main(int argc, char **argv) {

    if(argc!=7) {
        printf("\nUsage: ./sim_cache [BLOCKSIZE] [L1_SIZE] [L1_ASSOC] [L1_REPLACEMENT_POLICY]"
                       " [L1_WRITE_POLICY] [trace_file] > [output_file]\n");
        return 0;
    }

    unsigned int tag = 0;
    double missRate = 0, accessTime = 0, missPenalty = 0, cacheHitTime = 0;


    //check here for non-negative values

    unsigned int block = (unsigned)atoi(argv[1]);
    unsigned int size = (unsigned)atoi(argv[2]);
    unsigned int assoc = (unsigned)atoi(argv[3]);
    unsigned int replacement = (unsigned)atoi(argv[4]);
    unsigned int write = (unsigned)atoi(argv[5]);
    char *trace = argv[6];
    //FILE *trace_file = fopen(argv[6], "r");
    freopen(argv[6],"r",stdin);
    //freopen(argv[7],"w",stdout);
    int i,len=(int)strlen(trace);
    for(i=len;i>=0;i--) if(trace[i]=='/') break;
    trace=trace+i+1;

    cache.init(block, size, assoc, replacement, write, trace);



    tag = size/block;

    cache.c_tagArray = (unsigned int*)malloc( (tag*sizeof(unsigned int)) );
    cache.dirty_bit = (unsigned int*)malloc( (tag*sizeof(unsigned int)) );
    cache.valid_in_bit = (unsigned int*)malloc( (tag*sizeof(unsigned int)) );
    cache.LRUCounter = (int*)malloc( (tag*sizeof(int)) );
    cache.count_set = (int*)malloc( ((cache.SET)*sizeof(int)) );


    memset( cache.c_tagArray, 0, (sizeof(cache.c_tagArray[0])*tag) );
    memset( cache.dirty_bit, 0, (sizeof(cache.dirty_bit[0])*tag) );
    memset( cache.valid_in_bit, 0, (sizeof(cache.valid_in_bit[0])*tag) );
    memset( cache.LRUCounter, 0, (sizeof(cache.LRUCounter[0])*tag) );
    memset( cache.count_set, 0, (sizeof(cache.LRUCounter[0])*(cache.SET)) );

    cache.input();

    missRate = ( (double)( (int)cache.NUM_READ_MISS + (int)cache.NUM_WRITE_MISS )/(double)( (int)cache.NUM_READ + (int)cache.NUM_WRITE ) );

    missPenalty = (20 + 0.5*( ((double)cache.BLOCKSIZE/16) ) );

    cacheHitTime = ( 0.25 + 2.5*( (double)cache.SIZE/(512*1024) ) + 0.025*( (double)cache.BLOCKSIZE/16 ) + 0.025*( (double)cache.ASSOC ) );

    accessTime = ( cacheHitTime + ( missRate*missPenalty ) );

    cache.output(missRate,accessTime);
    return 0;
}


