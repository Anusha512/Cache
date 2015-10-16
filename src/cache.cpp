#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
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

    c_tagArray = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    dirty_bit = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    valid_in_bit = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    LRUCounter = (int*)malloc( (TAG*sizeof(int)) );
    count_set = (int*)malloc( ((SET)*sizeof(int)) );


    memset(c_tagArray, 0, (sizeof(c_tagArray[0])*TAG) );
    memset(dirty_bit, 0, (sizeof(dirty_bit[0])*TAG) );
    memset(valid_in_bit, 0, (sizeof(valid_in_bit[0])*TAG) );
    memset(LRUCounter, 0, (sizeof(LRUCounter[0])*TAG) );
    memset(count_set, 0, (sizeof(LRUCounter[0])*(SET)) );

    indexLocation = 0, tagAddress = 0;
}

void Cache::transAddress(unsigned int address) {
    int block = (int)log2(BLOCKSIZE);
    int index = (int)log2(SET);
    int tmp = 0 ;
    indexLocation = address>>block;

    for( int i=0; i<index; i++)
        tmp = tmp<<1 | 1;

    indexLocation &= tmp;
    tagAddress = address>>(block+index);
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
}

void Cache::output() {
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
    for(int i=0; i<SET; i++) {
        printf("set%4d:", i);
        for(int j=0; j<ASSOC; j++) {
            printf("%8x ",c_tagArray[i + (j*SET)]);
            printf( (WRITE_POLICY == 0 && dirty_bit[i + (j*SET)] == 1) ? "D" : " " );
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



void Cache::getMissRate() {
    MISS_RATE = ( (double)(NUM_READ_MISS + NUM_WRITE_MISS )/(double)( NUM_READ + NUM_WRITE ) );
}

void Cache::getAccessTime() {
    MISS_PENALTY = (20 + 0.5*( ((double)BLOCKSIZE/16) ) );
    HIT_TIME = ( 0.25 + 2.5*( (double)SIZE/(512*1024) ) + 0.025*( (double)BLOCKSIZE/16 ) + 0.025*( (double)ASSOC ) );
    ACCESS_TIME = ( HIT_TIME + ( MISS_RATE*MISS_PENALTY ) );
}

void Cache::readFromAddress() {
    unsigned int tagLocation = 0;

    for( int i=0; i<ASSOC; i++)
        if( c_tagArray[indexLocation + (i*SET)] == tagAddress )	{
            if( REPLACEMENT_POLICY )  //LFU
                LRUCounter[( indexLocation + (i*SET) )] = (LRUCounter[( indexLocation + (i*SET) )]) + 1;
            else  //LRU
                HIT(indexLocation, ( indexLocation + (i*SET) ) );
            return;
        }

    NUM_READ_MISS++, TOT_MEM_TRAFFIC++;

    if( REPLACEMENT_POLICY ) {  //LFU
        LFU(indexLocation, &tagLocation);

        LRUCounter[tagLocation] = count_set[indexLocation] + 1;
    }
    else {  //LRU
        	//increase the memory traffic counter
        LRU(indexLocation, &tagLocation);
    }
    c_tagArray[tagLocation] = tagAddress;

    if(WRITE_POLICY == 0)	//Write Back policy
    {
        if( dirty_bit[tagLocation] == 1 )
        {
            TOT_MEM_TRAFFIC += 1;
            NUM_WRITEBACK += 1;
            dirty_bit[tagLocation] = 0;
        }
    }

    return;
}

void Cache::writeToAddress() {
    unsigned int tagLocation = 0;

    for( int i=0; i<ASSOC; i++) {
        if( c_tagArray[indexLocation + (i*SET)] == tagAddress ) {
            if( WRITE_POLICY == 0 )
                dirty_bit[indexLocation + (i*SET)] = 1;
            else if( WRITE_POLICY == 1 )
                TOT_MEM_TRAFFIC += 1;

                if(REPLACEMENT_POLICY == 0 )	//LRU Policy
                {
                    HIT(indexLocation, ( indexLocation + (i*SET) ) );
                }
                else if( REPLACEMENT_POLICY == 1 )	//LFU Policy
                {
                    LRUCounter[( indexLocation + (i*SET) )] = (LRUCounter[( indexLocation + (i*SET) )]) + 1;
                }
            return;
        }
    }


    //Cache Miss
    //for( int i=0; i< (int)ASSOC; )
    //{
        NUM_WRITE_MISS += 1; 	// increase write miss counter
        TOT_MEM_TRAFFIC += 1;		//increase the memory traffic counter

        if( WRITE_POLICY == 0 )
        {
            //printf("\nL1 MISS, WRITE BACK");
            if(REPLACEMENT_POLICY == 0 )	//LRU Policy
            {
                LRU(indexLocation, &tagLocation);
                LRUCounter[tagLocation] = 0;
            }
            else if( REPLACEMENT_POLICY == 1 )	//LFU Policy
            {
                LFU(indexLocation, &tagLocation);
                LRUCounter[tagLocation] = count_set[indexLocation] + 1;
            }

            if( (int)dirty_bit[tagLocation] == 1 )
            {
                TOT_MEM_TRAFFIC += 1;
                NUM_WRITEBACK += 1;
            }

            dirty_bit[tagLocation] = 1;
            c_tagArray[tagLocation] = tagAddress;

        }
        //return;
    //}

    return;
}


void Cache::HIT(unsigned int indexLocation, unsigned int tagLocation) {
    for( int i=0; i<ASSOC; i++)
        if( LRUCounter[indexLocation + (i*SET)] < LRUCounter[tagLocation] )
            LRUCounter[indexLocation + (i*SET)] = (LRUCounter[indexLocation + (i*SET)]) + 1;
    LRUCounter[tagLocation] = 0;
}



void Cache::LRU(unsigned int indexLocation, unsigned int* tagLocation)
{
    unsigned int i = 0;
    int max = -1;
    *tagLocation = 0;
    //printf("\nL1 UPDATE LRU");
    for( i=0; i<ASSOC; i++)
    {
        if( LRUCounter[indexLocation + (i*SET)] > max )
        {
            max = LRUCounter[indexLocation + (i*SET)];
            *tagLocation = ( indexLocation + (i*SET) );
        }
    }


    for( i=0; i<ASSOC; i++)
    {
        LRUCounter[indexLocation + (i*SET)] = (LRUCounter[indexLocation + (i*SET)]) + 1;
    }

    LRUCounter[*tagLocation] = 0;

}


void Cache::LFU(unsigned int indexLocation, unsigned int* tagLocation) {
    int min = 1<<24;
    //*tagLocation = 0;
    //printf("\nL1 UPDATE LFU");
    for( int i=0; i<ASSOC; i++)
        if( LRUCounter[indexLocation + (i*SET)] < min ) {
            min = LRUCounter[indexLocation + (i*SET)];
            *tagLocation = ( indexLocation + (i*SET) );
        }
    //*tagLocation gives the location of block which is selected to be evicted
    count_set[indexLocation] = LRUCounter[*tagLocation];
}

