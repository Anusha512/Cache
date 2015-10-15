#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "cache.h"

void Cache::init(unsigned int block, unsigned int size, unsigned int assoc, unsigned int replacement, unsigned int write, char *trace) {
    BLOCKSIZE = block;
    SIZE = size;
    ASSOC = assoc;
    REPLACEMENT_POLICY = replacement;
    WRITE_POLICY = write;
    TRACE_FILE = trace;
    TAG = SIZE/BLOCKSIZE;
    SET = TAG/ASSOC;

    //nextLevel = NULL;
    NUM_READ = 0;
    NUM_READ_MISS = 0;
    NUM_WRITE = 0;
    NUM_WRITE_MISS = 0;
    TOT_MEM_TRAFFIC = 0;
    NUM_WRITEBACK = 0;

    TAGS = (unsigned int*)malloc( TAG*sizeof(unsigned int) );
    DIRTY = (unsigned int*)malloc( TAG*sizeof(unsigned int) );
    VALID = (unsigned int*)malloc( TAG*sizeof(unsigned int) );
    NUM_TAG = (int*)malloc( TAG*sizeof(int) );
    NUM_SET = (int*)malloc( (SET)*sizeof(int) );

    memset(TAGS, 0, sizeof(TAGS[0])*TAG );
    memset(DIRTY, 0, sizeof(DIRTY[0])*TAG );
    memset(VALID, 0, sizeof(VALID[0])*TAG );
    memset(NUM_TAG, 0, sizeof(NUM_TAG[0])*TAG );
    memset(NUM_SET, 0, sizeof(NUM_TAG[0])*(SET) );
}

void Cache::input() {
    char rw;
    unsigned int address;
    while(scanf(" %c %x",&rw,&address)!=EOF) {
        transAddress(address);
        if( rw == 'r' || rw == 'R') {
            NUM_READ++;
            readFromAddress();
        }
        else if( rw == 'w' || rw == 'W') {
            NUM_WRITE++;
            writeToAddress();
        }
    }
    MISS_RATE = ( double(NUM_READ_MISS + NUM_WRITE_MISS )/( NUM_READ + NUM_WRITE ) );
    MISS_PENALTY = (20 + 0.5*( (double)BLOCKSIZE/16) );
    HIT_TIME = ( 0.25 + 2.5*( (double)SIZE/(512*1024) ) + 0.025*( (double)BLOCKSIZE/16 ) + 0.025*ASSOC );
    ACCESS_TIME = ( HIT_TIME + ( MISS_RATE*MISS_PENALTY ) );
}

void Cache::output() {
	unsigned int i,j;
    //print head
    printf("  ===== Simulator configuration =====\n");
    printf("  L1_BLOCKSIZE:%22d\n", BLOCKSIZE);
    printf("  L1_SIZE:%27d\n", SIZE);
    printf("  L1_ASSOC:%26d\n", ASSOC);
    printf("  L1_REPLACEMENT_POLICY:%13d\n", REPLACEMENT_POLICY);
    printf("  L1_WRITE_POLICY:%19d\n", WRITE_POLICY);
    printf("  trace_file:%24s\n", TRACE_FILE);
    printf("  ===================================\n");
    puts("");

    //print body
    puts("===== L1 contents =====");
    for(i=0; i<SET; i++) {
        printf("set%4d:", i);
        for(j=0; j<ASSOC; j++) {
            printf("%8x ",TAGS[i + (j*SET)]);
            printf( (WRITE_POLICY == 0 && DIRTY[i + (j*SET)] == 1) ? "D" : " " );
        }
        puts("");
    }
    puts("");

    //print tail
    printf("  ====== Simulation results (raw) ======\n");
    printf("  a. number of L1 reads:%16d\n", NUM_READ);
    printf("  b. number of L1 read misses:%10d\n", NUM_READ_MISS);
    printf("  c. number of L1 writes:%15d\n", NUM_WRITE);
    printf("  d. number of L1 write misses:%9d\n", NUM_WRITE_MISS);
    printf("  e. L1 miss rate:%22.4f\n", MISS_RATE);
    printf("  f. number of writebacks from L1:%6d\n", NUM_WRITEBACK);
    printf("  g. total memory traffic:%14d\n", TOT_MEM_TRAFFIC);
    puts("");
    printf("  ==== Simulation results (performance) ====\n");
    printf("  1. average access time:%15.4f ns", ACCESS_TIME);
}

void Cache::transAddress(unsigned int address) {
    int block = (int)log2(BLOCKSIZE);
    int index = (int)log2(SET);
    int tmp = 0;
    for( int i=0; i<index; i++)
        tmp = tmp<<1 | 1;
    INDEX = address>>block & tmp;
    TAG_LOC = address>>(block+index);
}

void Cache::readFromAddress() {
    TAG_ADD = 0;

    for(unsigned int i=0; i<ASSOC; i++) {
        int index = INDEX + (i * SET);
        if (TAGS[index] == TAG_LOC) {
            if (REPLACEMENT_POLICY)  //LFU
                NUM_TAG[index] = (NUM_TAG[index]) + 1;
            else  //LRU
                HIT(index);
            return;
        }
    }

    NUM_READ_MISS++, TOT_MEM_TRAFFIC++;

    if( REPLACEMENT_POLICY ) {  //LFU
        LFU();
        NUM_TAG[TAG_ADD] = NUM_SET[INDEX] + 1;
    }
    else  //LRU
        LRU();

    TAGS[TAG_ADD] = TAG_LOC;

    if( !WRITE_POLICY )  //WBWA
        if( DIRTY[TAG_ADD] == 1 ) {
            TOT_MEM_TRAFFIC ++;
            NUM_WRITEBACK ++;
            DIRTY[TAG_ADD] = 0;
        }
    return;
}

void Cache::writeToAddress() {
    for(unsigned int i=0; i<ASSOC; i++) {
        int index = INDEX + (i*SET);
        if( TAGS[index] == TAG_LOC ) {
            if( WRITE_POLICY == 0 )
                DIRTY[index] = 1;
            else if( WRITE_POLICY == 1 )
                TOT_MEM_TRAFFIC ++;

            if( REPLACEMENT_POLICY )  //LFU
                NUM_TAG[index] = (NUM_TAG[index]) + 1;
            else  //LRU
                HIT( index );
            return;
        }
    }

    //Cache Miss
    NUM_WRITE_MISS ++,TOT_MEM_TRAFFIC ++;

    if( !WRITE_POLICY ) {  //WBWA
        if( REPLACEMENT_POLICY ) {  //LFU
            LFU();
            NUM_TAG[TAG_ADD] = NUM_SET[INDEX] + 1;
        }
        else {  //LRU
            LRU();
            NUM_TAG[TAG_ADD] = 0;
        }

        if( DIRTY[TAG_ADD] )
            TOT_MEM_TRAFFIC ++, NUM_WRITEBACK ++;

        DIRTY[TAG_ADD] = 1;
        TAGS[TAG_ADD] = TAG_LOC;
    }
    return;
}

void Cache::HIT(int index) {
	unsigned int i;
    for(i=0; i<ASSOC; i++)
        if( NUM_TAG[INDEX + (i*SET)] < NUM_TAG[index] )
            NUM_TAG[INDEX + (i*SET)] = (NUM_TAG[INDEX + (i*SET)]) + 1;
    NUM_TAG[index] = 0;
}

void Cache::LRU() {
	int max=-1;
	unsigned int i;
    for(i=0; i<ASSOC; i++)
        if( NUM_TAG[INDEX + (i*SET)] > max ) {
            max = NUM_TAG[INDEX + (i*SET)];
            TAG_ADD = ( INDEX + (i*SET) );
        }

    for(i=0; i<ASSOC; i++)
        NUM_TAG[INDEX + (i*SET)] = (NUM_TAG[INDEX + (i*SET)]) + 1;

    NUM_TAG[TAG_ADD] = 0;
}

void Cache::LFU() {
	int min=1<<24;
	unsigned int i;
    for(i=0; i<ASSOC; i++)
        if( NUM_TAG[INDEX + (i*SET)] < min ) {
            min = NUM_TAG[INDEX + (i*SET)];
            TAG_ADD = ( INDEX + (i*SET) );
        }
    NUM_SET[INDEX] = NUM_TAG[TAG_ADD];
}

