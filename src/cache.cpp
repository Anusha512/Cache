#include <cstdio>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include "Cache.h"

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

}

void Cache::input() {
    char rw[2];
    unsigned int address;
    unsigned int indexLocation = 0, tagAddress = 0;
    while(scanf("%s %x",rw,&address)!=EOF)
    {
        extractAddressParams(address, &indexLocation, &tagAddress);

        if( rw[0] == 'r' || rw[0] == 'R')
        {
            NUM_READ++;
            readFromAddress(indexLocation, tagAddress);
        }
        else if( rw[0] == 'w' || rw[0] == 'W')
        {
            NUM_WRITE++;
            writeToAddress(indexLocation, tagAddress);
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
    printf("===== L1 contents =====\n");
    for(int i=0; i<SET; i++)
    {
        printf("set%4d:", i);
        for(int j=0; j<ASSOC; j++)
        {
            printf("%8x ",c_tagArray[i + (j*SET)]);
            if( WRITE_POLICY == 0 && dirty_bit[i + (j*SET)] == 1)
                printf("D");
            else printf(" ");
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
int Cache::readFromAddress(unsigned int indexLocation, unsigned int tagAddress)
{
    int i=0;
    unsigned int tagLocation = 0;

    for( i=0; i< (int)ASSOC; i++)
    {
        if( c_tagArray[indexLocation + (i*SET)] == tagAddress )	//Checking Tag Entries
        {

            if(REPLACEMENT_POLICY == 0 )	//LRU Policy
            {
                LRUForHit(indexLocation, ( indexLocation + (i*SET) ) );
            }
            else if( REPLACEMENT_POLICY == 1 )
            {
                LRUCounter[( indexLocation + (i*SET) )] = (LRUCounter[( indexLocation + (i*SET) )]) + 1;
            }

            //printf("\nL1 HIT");
            return(0);

        }
    }


    if(REPLACEMENT_POLICY == 0 )	//LRU Policy
    {
        //It's a Cache Miss
        //printf("\nL1 MISS");
        NUM_READ_MISS += 1;
        TOT_MEM_TRAFFIC += 1;		//increase the memory traffic counter
        LRUForMiss(indexLocation, &tagLocation);
        c_tagArray[tagLocation] = tagAddress;
    }
    else if( REPLACEMENT_POLICY == 1 )
    {
        //It's a Cache Miss, LFU Policy
        //printf("\nL1 MISS");
        NUM_READ_MISS += 1;
        TOT_MEM_TRAFFIC += 1;
        //NUM_WRITEBACK += 1;
        LeastFrequentForMiss(indexLocation, &tagLocation);
        c_tagArray[tagLocation] = tagAddress;
        LRUCounter[tagLocation] = count_set[indexLocation] + 1;
    }

    if(WRITE_POLICY == 0)	//Write Back policy
    {
        if( dirty_bit[tagLocation] == 1 )
        {
            TOT_MEM_TRAFFIC += 1;
            NUM_WRITEBACK += 1;
            dirty_bit[tagLocation] = 0;
        }
    }

    return(0);
}

int Cache::writeToAddress(unsigned int indexLocation, unsigned int tagAddress)
{
    int i=0;
    unsigned int tagLocation = 0;

    for( i=0; i< (int)ASSOC; i++)
    {

        //first check TAG if it is matched or not
        if( c_tagArray[indexLocation + (i*SET)] == tagAddress )	//Checking Tag Entries
        {
            {
                //printf("\nL1 HIT");

                //0 for WBWA  --> Dirty bits
                //1 for WTNA  --> No use of dirty bits

                if( WRITE_POLICY == 0 )
                {
                    dirty_bit[indexLocation + (i*SET)] = 1;
                }
                else if( WRITE_POLICY == 1 )
                {
                    TOT_MEM_TRAFFIC += 1;
                }

                if(REPLACEMENT_POLICY == 0 )	//LRU Policy
                {
                    LRUForHit(indexLocation, ( indexLocation + (i*SET) ) );
                }
                else if( REPLACEMENT_POLICY == 1 )	//LFU Policy
                {
                    LRUCounter[( indexLocation + (i*SET) )] = (LRUCounter[( indexLocation + (i*SET) )]) + 1;
                }

                return(0);
            }
        }
    }


    //Cache Miss
    for( i=0; i< (int)ASSOC; )
    {
        NUM_WRITE_MISS += 1; 	// increase write miss counter
        TOT_MEM_TRAFFIC += 1;		//increase the memory traffic counter

        if( WRITE_POLICY == 0 )
        {
            //printf("\nL1 MISS, WRITE BACK");
            if(REPLACEMENT_POLICY == 0 )	//LRU Policy
            {
                LRUForMiss(indexLocation, &tagLocation);
                LRUCounter[tagLocation] = 0;
            }
            else if( REPLACEMENT_POLICY == 1 )	//LFU Policy
            {
                LeastFrequentForMiss(indexLocation, &tagLocation);
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
        return(0);
    }

    return(0);
}


void Cache::LRUForHit(unsigned int indexLocation, unsigned int tagLocation)
{
    int i = 0;

    for( i=0; i< (int)ASSOC; i++)
    {
        if( LRUCounter[indexLocation + (i*SET)] < LRUCounter[tagLocation] )
        {
            LRUCounter[indexLocation + (i*SET)] = (LRUCounter[indexLocation + (i*SET)]) + 1;
        }
    }

    LRUCounter[tagLocation] = 0;
}



void Cache::LRUForMiss(unsigned int indexLocation, unsigned int* tagLocation)
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


void Cache::LeastFrequentForMiss(unsigned int indexLocation, unsigned int* tagLocation)
{
    unsigned int i = 0;
    int min = 16777215;
    *tagLocation = 0;
    //printf("\nL1 UPDATE LFU");


    for( i=0; i<ASSOC; i++)
    {
        if( LRUCounter[indexLocation + (i*SET)] < min )
        {
            min = LRUCounter[indexLocation + (i*SET)];
            *tagLocation = ( indexLocation + (i*SET) );
        }
    }

    //*tagLocation gives the location of block which is selected to be evicted
    count_set[indexLocation] = LRUCounter[*tagLocation];
}

void Cache::extractAddressParams(unsigned int addressInInt, unsigned int* indexLocation, unsigned int* tagAddress)
{
    int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    noOfIndexBits = (int)log2(SET);

    *indexLocation = addressInInt>>noOfBlockBits;

    for( i=0; i<noOfIndexBits; i++)
    {
        tempIndexNo = ( 1 | tempIndexNo<<1 );
    }

    *indexLocation = ( *indexLocation & tempIndexNo );
    *tagAddress = addressInInt>>(noOfBlockBits + noOfIndexBits);
}