//
// Created by Shintaku on 10/26/15.
//
#include <cstdio>
#include <cmath>
#include "Cache.h"

void Cache::init(unsigned int block, unsigned int size, unsigned int assoc, unsigned int replacement, unsigned int write, char *trace) {
    BLOCKSIZE = block;
    SIZE = size;
    ASSOC = assoc;
    REPLACEMENT_POLICY = replacement;
    WRITE_POLICY = write;
    TRACE_FILE = trace;
    SET = (size/(block*assoc));
    //nextLevel = NULL;
    NUM_READ = 0;
    NUM_READ_MISS = 0;
    NUM_WRITE = 0;
    NUM_WRITE_MISS = 0;
    TOT_MEM_TRAFFIC = 0;
    NUM_WRITEBACK = 0;

}

void Cache::input() {
    char rw[2];
    unsigned int address;
    unsigned int indexLocation = 0, tagAddress = 0;
    while(scanf("%s %x",rw,&address)!=EOF)
    {
        extractAddressParams(address, this, &indexLocation, &tagAddress);

        if( rw[0] == 'r' || rw[0] == 'R')
        {
            NUM_READ +=1;
            readFromAddress(this, indexLocation, tagAddress);
        }
        else if( rw[0] == 'w' || rw[0] == 'W')
        {
            NUM_WRITE +=1;
            writeToAddress(this, indexLocation, tagAddress);
        }
    }
}

void Cache::output(double miss,double access) {
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
    printf("  e. L1 miss rate:%22.4f\n", miss);
    printf("  f. number of writebacks from L1:%6d\n", NUM_WRITEBACK);
    printf("  g. total memory traffic:%14d\n", TOT_MEM_TRAFFIC);
    puts("");
    printf("  ==== Simulation results (performance) ====\n");
    printf("  1. average access time:%15.4f ns", access);
}

int readFromAddress(Cache* cache, unsigned int indexLocation, unsigned int tagAddress)
{
    int i=0;
    unsigned int tagLocation = 0;

    for( i=0; i< (int)cache->ASSOC; i++)
    {
        if( cache->c_tagArray[indexLocation + (i*cache->SET)] == tagAddress )	//Checking Tag Entries
        {

            if(cache->REPLACEMENT_POLICY == 0 )	//LRU Policy
            {
                LRUForHit(cache, indexLocation, ( indexLocation + (i*cache->SET) ) );
            }
            else if( cache->REPLACEMENT_POLICY == 1 )
            {
                cache->LRUCounter[( indexLocation + (i*cache->SET) )] = (cache->LRUCounter[( indexLocation + (i*cache->SET) )]) + 1;
            }

            //printf("\nL1 HIT");
            return(0);

        }
    }


    if(cache->REPLACEMENT_POLICY == 0 )	//LRU Policy
    {
        //It's a Cache Miss
        //printf("\nL1 MISS");
        cache->NUM_READ_MISS += 1;
        cache->TOT_MEM_TRAFFIC += 1;		//increase the memory traffic counter
        LRUForMiss(cache, indexLocation, &tagLocation);
        cache->c_tagArray[tagLocation] = tagAddress;
    }
    else if( cache->REPLACEMENT_POLICY == 1 )
    {
        //It's a Cache Miss, LFU Policy
        //printf("\nL1 MISS");
        cache->NUM_READ_MISS += 1;
        cache->TOT_MEM_TRAFFIC += 1;
        //cache->NUM_WRITEBACK += 1;
        LeastFrequentForMiss(cache, indexLocation, &tagLocation);
        cache->c_tagArray[tagLocation] = tagAddress;
        cache->LRUCounter[tagLocation] = cache->count_set[indexLocation] + 1;
    }

    if(cache->WRITE_POLICY == 0)	//Write Back policy
    {
        if( cache->dirty_bit[tagLocation] == 1 )
        {
            cache->TOT_MEM_TRAFFIC += 1;
            cache->NUM_WRITEBACK += 1;
            cache->dirty_bit[tagLocation] = 0;
        }
    }

    return(0);
}





int writeToAddress(Cache* cache, unsigned int indexLocation, unsigned int tagAddress)
{
    int i=0;
    unsigned int tagLocation = 0;

    for( i=0; i< (int)cache->ASSOC; i++)
    {

        //first check tag if it is matched or not
        if( cache->c_tagArray[indexLocation + (i*cache->SET)] == tagAddress )	//Checking Tag Entries
        {
            {
                //printf("\nL1 HIT");

                //0 for WBWA  --> Dirty bits
                //1 for WTNA  --> No use of dirty bits

                if( cache->WRITE_POLICY == 0 )
                {
                    cache->dirty_bit[indexLocation + (i*cache->SET)] = 1;
                }
                else if( cache->WRITE_POLICY == 1 )
                {
                    cache->TOT_MEM_TRAFFIC += 1;
                }

                if(cache->REPLACEMENT_POLICY == 0 )	//LRU Policy
                {
                    LRUForHit(cache, indexLocation, ( indexLocation + (i*cache->SET) ) );
                }
                else if( cache->REPLACEMENT_POLICY == 1 )	//LFU Policy
                {
                    cache->LRUCounter[( indexLocation + (i*cache->SET) )] = (cache->LRUCounter[( indexLocation + (i*cache->SET) )]) + 1;
                }

                return(0);
            }
        }
    }


    //Cache Miss
    for( i=0; i< (int)cache->ASSOC; )
    {
        cache->NUM_WRITE_MISS += 1; 	// increase write miss counter
        cache->TOT_MEM_TRAFFIC += 1;		//increase the memory traffic counter

        if( cache->WRITE_POLICY == 0 )
        {
            //printf("\nL1 MISS, WRITE BACK");
            if(cache->REPLACEMENT_POLICY == 0 )	//LRU Policy
            {
                LRUForMiss(cache, indexLocation, &tagLocation);
                cache->LRUCounter[tagLocation] = 0;
            }
            else if( cache->REPLACEMENT_POLICY == 1 )	//LFU Policy
            {
                LeastFrequentForMiss(cache, indexLocation, &tagLocation);
                cache->LRUCounter[tagLocation] = cache->count_set[indexLocation] + 1;
            }

            if( (int)cache->dirty_bit[tagLocation] == 1 )
            {
                cache->TOT_MEM_TRAFFIC += 1;
                cache->NUM_WRITEBACK += 1;
            }

            cache->dirty_bit[tagLocation] = 1;
            cache->c_tagArray[tagLocation] = tagAddress;

        }
        return(0);
    }

    return(0);
}


void LRUForHit(Cache* cache, unsigned int indexLocation, unsigned int tagLocation)
{
    int i = 0;

    for( i=0; i< (int)cache->ASSOC; i++)
    {
        if( cache->LRUCounter[indexLocation + (i*cache->SET)] < cache->LRUCounter[tagLocation] )
        {
            cache->LRUCounter[indexLocation + (i*cache->SET)] = (cache->LRUCounter[indexLocation + (i*cache->SET)]) + 1;
        }
    }

    cache->LRUCounter[tagLocation] = 0;
}



void LRUForMiss(Cache* cache, unsigned int indexLocation, unsigned int* tagLocation)
{
    unsigned int i = 0;
    int max = -1;
    *tagLocation = 0;
    //printf("\nL1 UPDATE LRU");
    for( i=0; i<cache->ASSOC; i++)
    {
        if( cache->LRUCounter[indexLocation + (i*cache->SET)] > max )
        {
            max = cache->LRUCounter[indexLocation + (i*cache->SET)];
            *tagLocation = ( indexLocation + (i*cache->SET) );
        }
    }


    for( i=0; i<cache->ASSOC; i++)
    {
        cache->LRUCounter[indexLocation + (i*cache->SET)] = (cache->LRUCounter[indexLocation + (i*cache->SET)]) + 1;
    }

    cache->LRUCounter[*tagLocation] = 0;

}





void LeastFrequentForMiss(Cache* cache, unsigned int indexLocation, unsigned int* tagLocation)
{
    unsigned int i = 0;
    int min = 16777215;
    *tagLocation = 0;
    //printf("\nL1 UPDATE LFU");


    for( i=0; i<cache->ASSOC; i++)
    {
        if( cache->LRUCounter[indexLocation + (i*cache->SET)] < min )
        {
            min = cache->LRUCounter[indexLocation + (i*cache->SET)];
            *tagLocation = ( indexLocation + (i*cache->SET) );
        }
    }

    //*tagLocation gives the location of block which is selected to be evicted
    cache->count_set[indexLocation] = cache->LRUCounter[*tagLocation];
}

void extractAddressParams(unsigned int addressInInt, Cache* cache, unsigned int* indexLocation, unsigned int* tagAddress)
{
    int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;

    noOfBlockBits = (int)log2(cache->BLOCKSIZE);
    noOfIndexBits = (int)log2(cache->SET);

    *indexLocation = addressInInt>>noOfBlockBits;

    for( i=0; i<noOfIndexBits; i++)
    {
        tempIndexNo = ( 1 | tempIndexNo<<1 );
    }

    *indexLocation = ( *indexLocation & tempIndexNo );
    *tagAddress = addressInInt>>(noOfBlockBits + noOfIndexBits);
}