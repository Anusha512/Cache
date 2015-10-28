#include <iostream>
#include <cmath>
#include "cache.h"
using namespace std;

void extractAddressParams(unsigned int addressInInt, Cache* cache, unsigned int* indexLocation, unsigned int* tagAddress);

Cache cache;
int main(int argc, char **argv)
{
    if(argc!=7)
    {
        printf("\nUsage: ./sim_cache [BLOCKSIZE] [L1_SIZE] [L1_ASSOC] [L1_REPLACEMENT_POLICY]"
                       " [L1_WRITE_POLICY] [trace_file] > [output_file]\n");
        return 0;
    }

    unsigned int countTraceEntries = 0, noOfTagEntries = 0, addressInInt = 0;
    unsigned int indexLocation = 0, tagAddress = 0;
    char readAddress[100];
    char *address, *isItReadOrWrite;
    double missRate = 0, accessTime = 0, missPenalty = 0, cacheHitTime = 0;


    //check here for non-negative values

    unsigned int block = (unsigned)atoi(argv[1]);
    unsigned int size = (unsigned)atoi(argv[2]);
    unsigned int assoc = (unsigned)atoi(argv[3]);
    unsigned int replacement = (unsigned)atoi(argv[4]);
    unsigned int write = (unsigned)atoi(argv[5]);
    char *trace = argv[6];//, *trace;
    int i,len=(int)strlen(trace);
    for(i=len;i>=0;i--) if(trace[i]=='/') break;
    trace=trace+i+1;
    cache.input(block, size, assoc, replacement, write, trace);
    FILE *trace_file = fopen(argv[6], "r");


    noOfTagEntries = cache.SET*(unsigned)atoi(argv[3]);

    cache.c_tagArray = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
    cache.dirty_bit = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
    cache.valid_in_bit = (unsigned int*)malloc( (noOfTagEntries*sizeof(unsigned int)) );
    cache.LRUCounter = (int*)malloc( (noOfTagEntries*sizeof(int)) );
    cache.count_set = (int*)malloc( ((cache.SET)*sizeof(int)) );


    memset( cache.c_tagArray, 0, (sizeof(cache.c_tagArray[0])*noOfTagEntries) );
    memset( cache.dirty_bit, 0, (sizeof(cache.dirty_bit[0])*noOfTagEntries) );
    memset( cache.valid_in_bit, 0, (sizeof(cache.valid_in_bit[0])*noOfTagEntries) );
    memset( cache.LRUCounter, 0, (sizeof(cache.LRUCounter[0])*noOfTagEntries) );
    memset( cache.count_set, 0, (sizeof(cache.LRUCounter[0])*(cache.SET)) );

    while( fgets(readAddress, 100, trace_file)!=NULL )
    {
        //	printf("\nAddress is : %s", readAddress);
        isItReadOrWrite = strtok(readAddress, " ");
        address = strtok(NULL, "\n");
        addressInInt = (unsigned)strtoll(address, NULL, 16);

        extractAddressParams(addressInInt, &cache, &indexLocation, &tagAddress);

        if( isItReadOrWrite[0] == 'r' || isItReadOrWrite[0] == 'R')
        {
            cache.readCounter +=1;
            readFromAddress(&cache, indexLocation, tagAddress);
        }
        else if( isItReadOrWrite[0] == 'w' || isItReadOrWrite[0] == 'W')
        {
            cache.writeCounter +=1;
            writeToAddress(&cache, indexLocation, tagAddress);
        }
        else
        {
            printf("\nTrace file doesn't specify R/W at line %d", (countTraceEntries+1));
            continue;
        }

        //	printf("\nChanged set %d: ", indexLocation);
        countTraceEntries++;

    }

    missRate = ( (double)( (int)cache.readMissCounter + (int)cache.writeMissCounter )/(double)( (int)cache.readCounter + (int)cache.writeCounter ) );

    missPenalty = (20 + 0.5*( ((double)cache.BLOCKSIZE/16) ) );

    cache.SIZE = (unsigned)atoi(argv[2]);
    cache.ASSOC = (unsigned)atoi(argv[3]);

    cacheHitTime = ( 0.25 + 2.5*( (double)cache.SIZE/(512*1024) ) + 0.025*( (double)cache.BLOCKSIZE/16 ) + 0.025*( (double)cache.ASSOC ) );

    accessTime = ( cacheHitTime + ( missRate*missPenalty ) );

    cache.output(missRate,accessTime);
    return 0;
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
