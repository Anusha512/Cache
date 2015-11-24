#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "cache.h"



Cache l1Cache, l2Cache, victimCache;

int main(int argc, char **argv)
{


    unsigned int countTraceEntries = 0, noOfTagEntriesL1 = 0, noOfTagEntriesL2 = 0;
    unsigned int i=0, j=0, noOfTagEntriesVC = 0, k=0, tagKey = 0, dirtyKey = 0;
    int key = 0;
    double missRateL1 = 0, accessTimeL1 = 0, missPenaltyL1 = 0, cacheHitTimeL1 = 0;
    double missRateL2 = 0, accessTimeL2 = 0, missPenaltyL2 = 0, cacheHitTimeL2 = 0;

    if(argc<7)
    {
        printf("\nInsufficient Arguments Supplied. Returning..\n");
        return(0);
    }
    freopen(argv[7],"r",stdin);
    char *trace = argv[7];
    int id,len=(int)strlen(trace);
    for(id=len;id>=0;id--)
        if(trace[id]=='/')
            break;
    trace=trace+id+1;

    l1Cache.BLOCKSIZE = (unsigned int)atoi(argv[1]);
    l1Cache.SIZE = (unsigned int)atoi(argv[2]);
    l1Cache.ASSOC = (unsigned int)atoi(argv[3]);

    l1Cache.c_numOfSets = (l1Cache.SIZE/(l1Cache.BLOCKSIZE*l1Cache.ASSOC));



    //Initialize L1 Cache
    noOfTagEntriesL1 = l1Cache.c_numOfSets*l1Cache.ASSOC;
    l1Cache.TAGS = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
    l1Cache.DIRTY = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
    l1Cache.VALID = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
    l1Cache.LRUCounter = (int*)malloc( (noOfTagEntriesL1*sizeof(int)) );

    memset( l1Cache.TAGS, 0, (sizeof(l1Cache.TAGS[0])*noOfTagEntriesL1) );
    memset( l1Cache.DIRTY, 0, (sizeof(l1Cache.DIRTY[0])*noOfTagEntriesL1) );
    memset( l1Cache.VALID, 0, (sizeof(l1Cache.VALID[0])*noOfTagEntriesL1) );
    memset( l1Cache.LRUCounter, 0, (sizeof(l1Cache.LRUCounter[0])*noOfTagEntriesL1) );


    l1Cache.NUM_READ = 0;
    l1Cache.NUM_WRITE = 0;
    l1Cache.NUM_READ_MISS = 0;
    l1Cache.NUM_WRITE_MISS = 0;
    l1Cache.memoryAccessCounter = 0;
    l1Cache.NUM_WRITEBACK = 0;
    l1Cache.nextLevel = NULL;


    //Checking for L2 Cache Presence
    if( (unsigned int)atoi(argv[5]) == 0 )
    {
        l2Cache.SIZE = (unsigned int)atoi(argv[5]);
    }
    else
    {
        l2Cache.BLOCKSIZE = (unsigned int)atoi(argv[1]);
        l2Cache.SIZE = (unsigned int)atoi(argv[5]);
        l2Cache.ASSOC = (unsigned int)atoi(argv[6]);

        l1Cache.nextLevel = &l2Cache;
        l2Cache.nextLevel = NULL;

        l2Cache.c_numOfSets = (l2Cache.SIZE/(l2Cache.BLOCKSIZE*l2Cache.ASSOC));


        noOfTagEntriesL2 = l2Cache.c_numOfSets*l2Cache.ASSOC;
        l2Cache.TAGS = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
        l2Cache.DIRTY = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
        l2Cache.VALID = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
        l2Cache.LRUCounter = (int*)malloc( (noOfTagEntriesL2*sizeof(int)) );


        memset( l2Cache.TAGS, 0, (sizeof(l2Cache.TAGS[0])*noOfTagEntriesL2) );
        memset( l2Cache.DIRTY, 0, (sizeof(l2Cache.DIRTY[0])*noOfTagEntriesL2) );
        memset( l2Cache.VALID, 0, (sizeof(l2Cache.VALID[0])*noOfTagEntriesL2) );
        memset( l2Cache.LRUCounter, 0, (sizeof(l2Cache.LRUCounter[0])*noOfTagEntriesL2) );


        l2Cache.NUM_READ = 0;
        l2Cache.NUM_WRITE = 0;
        l2Cache.NUM_READ_MISS = 0;
        l2Cache.NUM_WRITE_MISS = 0;
        l2Cache.memoryAccessCounter = 0;
        l2Cache.NUM_WRITEBACK = 0;

    }



    //Checking for Victim Cache Presence
    if( (unsigned int)atoi(argv[4]) == 0 )
    {
        victimCache.SIZE = (unsigned int)atoi(argv[4]);
    }
    else
    {
        victimCache.BLOCKSIZE = l1Cache.BLOCKSIZE;
        victimCache.SIZE = (unsigned int)atoi(argv[4]);
        victimCache.nextLevel =NULL;

        victimCache.c_numOfSets = 1;
        victimCache.ASSOC = (victimCache.SIZE/victimCache.BLOCKSIZE);

        noOfTagEntriesVC = victimCache.c_numOfSets*victimCache.ASSOC;
        victimCache.TAGS = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
        victimCache.DIRTY = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
        victimCache.VALID = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
        victimCache.LRUCounter = (int*)malloc( (noOfTagEntriesVC*sizeof(int)) );

        memset( victimCache.TAGS, 0, (sizeof(victimCache.TAGS[0])*noOfTagEntriesVC) );
        memset( victimCache.DIRTY, 0, (sizeof(victimCache.DIRTY[0])*noOfTagEntriesVC) );
        memset( victimCache.VALID, 0, (sizeof(victimCache.VALID[0])*noOfTagEntriesVC) );
        memset( victimCache.LRUCounter, 0, (sizeof(victimCache.LRUCounter[0])*noOfTagEntriesVC) );


        victimCache.NUM_READ = 0;
        victimCache.NUM_WRITE = 0;
        victimCache.NUM_READ_MISS = 0;
        victimCache.NUM_WRITE_MISS = 0;
        victimCache.memoryAccessCounter = 0;
        victimCache.NUM_WRITEBACK = 0;
        victimCache.noOfSwaps = 0;


    }

    char rw;
    unsigned int address;
    while( scanf(" %c %x",&rw,&address)!=EOF )
    {

        if( rw == 'r' || rw == 'R')
        {
            //printf("\n\n%d. Address %x READ", (countTraceEntries+1), addressInInt);
            readFromAddress(l1Cache, address, victimCache, victimCache.SIZE);// indexLocation, tagAddress);
        }
        else if( rw == 'w' || rw == 'W')
        {
            //printf("\n\n%d. Address %x WRITE", (countTraceEntries+1), addressInInt);
            writeToAddress(l1Cache, address, victimCache, victimCache.SIZE);
        }

        countTraceEntries++;

    }


    printf("===== Simulator configuration =====\n");
    printf("%-30s%-d\n", "BLOCKSIZE:", l1Cache.BLOCKSIZE);
    printf("%-30s%-d\n", "L1_SIZE:", l1Cache.SIZE);
    printf("%-30s%-d\n", "L1_ASSOC:", l1Cache.ASSOC);
    printf("%-30s%-d\n", "Victim_Cache_SIZE:", victimCache.SIZE);
    printf("%-30s%-d\n", "L2_SIZE:", l2Cache.SIZE);
    printf("%-30s%-d\n", "L2_ASSOC:", l2Cache.ASSOC);
    printf("%-30s%-s\n", "trace_file:", trace);
    printf("===================================");
    printf("\n===== L1 contents =====\n");

    for( i=0; i<l1Cache.c_numOfSets; i++)
    {
        //sort L1 cache based on LRU counter
        for( j=1; j<l1Cache.ASSOC; j++)
        {
            key = l1Cache.LRUCounter[i + (j*l1Cache.c_numOfSets)];
            tagKey = l1Cache.TAGS[i + (j*l1Cache.c_numOfSets)];
            dirtyKey = l1Cache.DIRTY[i + (j*l1Cache.c_numOfSets)];
            k = j-1;

            while( ((int)k>=0) && ( key < ( l1Cache.LRUCounter[i + (k*l1Cache.c_numOfSets)] ) ) )
            {
                l1Cache.LRUCounter[i + ((k+1)*l1Cache.c_numOfSets)] = l1Cache.LRUCounter[i + (k*l1Cache.c_numOfSets)];
                l1Cache.TAGS[i + ((k+1)*l1Cache.c_numOfSets)] = l1Cache.TAGS[i + (k*l1Cache.c_numOfSets)];
                l1Cache.DIRTY[i + ((k+1)*l1Cache.c_numOfSets)] = l1Cache.DIRTY[i + (k*l1Cache.c_numOfSets)];;

                k = k-1;
            }

            l1Cache.LRUCounter[i + ((k+1)*l1Cache.c_numOfSets)] = key;
            l1Cache.TAGS[i + ((k+1)*l1Cache.c_numOfSets)] = tagKey;
            l1Cache.DIRTY[i + ((k+1)*l1Cache.c_numOfSets)] = dirtyKey;

        }

        printf("set %d: ", i);
        for( j=0; j<l1Cache.ASSOC; j++)
        {
            if( l1Cache.DIRTY[i + (j*l1Cache.c_numOfSets)] == 1)
                printf("%-7x%-3c",l1Cache.TAGS[i + (j*l1Cache.c_numOfSets)], 'D');
            else
                printf("%-10x",l1Cache.TAGS[i + (j*l1Cache.c_numOfSets)]);
        }
        printf("\n");
    }


    if( victimCache.SIZE != 0)
    {
        printf("===== Victim Cache contents =====\n");
        printf("set 0: ");


        //sort victim cache based on LRU counter
        for( i=1; i<victimCache.ASSOC; i++)
        {
            key = victimCache.LRUCounter[i];
            tagKey = victimCache.TAGS[i];
            dirtyKey = victimCache.DIRTY[i];
            j = i-1;

            while( ((int)j>=0) && (key<victimCache.LRUCounter[j]) )
            {
                victimCache.LRUCounter[j+1] = victimCache.LRUCounter[j];
                victimCache.TAGS[j+1] = victimCache.TAGS[j];
                victimCache.DIRTY[j+1] = victimCache.DIRTY[j];;

                j = j-1;
            }

            victimCache.LRUCounter[j+1] = key;
            victimCache.TAGS[j+1] = tagKey;
            victimCache.DIRTY[j+1] = dirtyKey;
        }




        for( i=0; i<victimCache.ASSOC; i++)
        {
            if( victimCache.DIRTY[i] == 1)
                printf("%-8x%-3c",victimCache.TAGS[i], 'D');
            else
                printf("%-10x",victimCache.TAGS[i]);
        }
        printf("\n");
    }



    if( l2Cache.SIZE != 0)
    {

        printf("===== L2 contents =====\n");

        for( i=0; i<l2Cache.c_numOfSets; i++)
        {

            //sort L2 cache based on LRU counter
            for( j=1; j<l2Cache.ASSOC; j++)
            {
                key = l2Cache.LRUCounter[i + (j*l2Cache.c_numOfSets)];
                tagKey = l2Cache.TAGS[i + (j*l2Cache.c_numOfSets)];
                dirtyKey = l2Cache.DIRTY[i + (j*l2Cache.c_numOfSets)];
                k = j-1;

                while( ((int)k>=0) && ( key < ( l2Cache.LRUCounter[i + (k*l2Cache.c_numOfSets)] ) ) )
                {
                    l2Cache.LRUCounter[i + ((k+1)*l2Cache.c_numOfSets)] = l2Cache.LRUCounter[i + (k*l2Cache.c_numOfSets)];
                    l2Cache.TAGS[i + ((k+1)*l2Cache.c_numOfSets)] = l2Cache.TAGS[i + (k*l2Cache.c_numOfSets)];
                    l2Cache.DIRTY[i + ((k+1)*l2Cache.c_numOfSets)] = l2Cache.DIRTY[i + (k*l2Cache.c_numOfSets)];;

                    k = k-1;
                }

                l2Cache.LRUCounter[i + ((k+1)*l2Cache.c_numOfSets)] = key;
                l2Cache.TAGS[i + ((k+1)*l2Cache.c_numOfSets)] = tagKey;
                l2Cache.DIRTY[i + ((k+1)*l2Cache.c_numOfSets)] = dirtyKey;

            }


            printf("set %d: ", i);
            for( j=0; j<l2Cache.ASSOC; j++)
            {
                if( l2Cache.DIRTY[i + (j*l2Cache.c_numOfSets)] == 1)
                    printf("%-7x%-3c",l2Cache.TAGS[i + (j*l2Cache.c_numOfSets)], 'D');
                else
                    printf("%-10x",l2Cache.TAGS[i + (j*l2Cache.c_numOfSets)]);
            }
            printf("\n");
        }
    }


    //L1 calculation
    missRateL1 = ( (double)( (int)l1Cache.NUM_READ_MISS + (int)l1Cache.NUM_WRITE_MISS )/(double)( (int)l1Cache.NUM_READ + (int)l1Cache.NUM_WRITE ) );
    missPenaltyL1 = (double)(20 + 0.5*( ((double)l1Cache.BLOCKSIZE/16) ) );

    l1Cache.SIZE = (unsigned int)atoi(argv[2]);
    l1Cache.ASSOC = (unsigned int)atoi(argv[3]);

    cacheHitTimeL1 = (double)( 0.25 + 2.5*( (double)l1Cache.SIZE/(512*1024) ) + 0.025*( (double)l1Cache.BLOCKSIZE/16 ) + 0.025*( (double)l1Cache.ASSOC ) );
    accessTimeL1 = (double)( (double)cacheHitTimeL1 + ( (double)missRateL1*(double)missPenaltyL1 ) );


    printf("====== Simulation results (raw) ======\n");
    printf("%-38s%-6d\n", "a. number of L1 reads:", l1Cache.NUM_READ);
    printf("%-38s%-6d\n", "b. number of L1 read misses:", l1Cache.NUM_READ_MISS);
    printf("%-38s%-6d\n", "c. number of L1 writes:", l1Cache.NUM_WRITE);
    printf("%-38s%-6d\n", "d. number of L1 write misses:", l1Cache.NUM_WRITE_MISS);
    printf("%-38s%-2.4f\n", "e. L1 miss rate:", missRateL1);


    //Victim cache results
    printf("%-38s%-6d\n", "f. number of swaps:", victimCache.noOfSwaps);
    printf("%-38s%-6d\n", "g. number of victim cache writeback:", victimCache.NUM_WRITEBACK);

    //L2 cache calculation
    missRateL2 = ( (double)( (int)l2Cache.NUM_READ_MISS )/(double)( (int)l2Cache.NUM_READ ) );
    missPenaltyL2 = (double)(20 + 0.5*( ((double)l2Cache.BLOCKSIZE/16) ) );

    l2Cache.SIZE = (unsigned int)atoi(argv[5]);
    l2Cache.ASSOC = (unsigned int)atoi(argv[6]);

    cacheHitTimeL2 = (double)( 2.5 + 2.5*( (double)l2Cache.SIZE/(512*1024) ) + 0.025*( (double)l2Cache.BLOCKSIZE/16 ) + 0.025*( (double)l2Cache.ASSOC ) );

    printf("%-38s%-6d\n", "h. number of L2 reads:", l2Cache.NUM_READ);
    printf("%-38s%-6d\n", "i. number of L2 read misses:", l2Cache.NUM_READ_MISS);
    printf("%-38s%-6d\n", "j. number of L2 writes:", l2Cache.NUM_WRITE);
    printf("%-38s%-6d\n", "k. number of L2 write misses:", l2Cache.NUM_WRITE_MISS);
    if( l2Cache.SIZE == 0 )
        printf("%-38s%-6c\n", "l. L2 miss rate:", '0');
    else
        printf("%-38s%-2.4f\n", "l. L2 miss rate:", missRateL2);
    printf("%-38s%-6d\n", "m. number of L2 writebacks:", l2Cache.NUM_WRITEBACK);

    if(l2Cache.SIZE == 0)
        printf("%-38s%-6d\n", "n. total memory traffic:", l1Cache.memoryAccessCounter);
    else
        printf("%-38s%-6d\n", "n. total memory traffic:", l2Cache.memoryAccessCounter);


    if(l2Cache.SIZE != 0)
    {
        accessTimeL2 = ( cacheHitTimeL1 + ( (double)missRateL1*( cacheHitTimeL2 + ( (double)missRateL2*(double)missPenaltyL2 ) ) ) );
    }

    printf("==== Simulation results (performance) ====\n");

    if(l2Cache.SIZE != 0)
    {
        printf("1. average access time:%15.4f ns\n", accessTimeL2);  //========================================AAt calculate
    }
    else
        printf("1. average access time:%15.4f ns\n", accessTimeL1);

    return(0);
}





void extractAddressParams(unsigned int addressInInt, Cache cache_ds, unsigned int* indexLocation, unsigned int* tagAddress)
{
    int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;

    noOfBlockBits = (int)log2(cache_ds.BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.c_numOfSets);

    *indexLocation = addressInInt>>noOfBlockBits;

    for( i=0; i<noOfIndexBits; i++)
    {
        tempIndexNo = ( 1 | tempIndexNo<<1 );
    }

    *indexLocation = ( *indexLocation & tempIndexNo );
    *tagAddress = addressInInt>>(noOfBlockBits + noOfIndexBits);
}





//Recursive solution

int readFromAddress(Cache &cache_ds, unsigned int addressInInt, Cache victimCache, unsigned int vc_size)
{
    int i=0, foundInvalidEntry = 0, noOfBlockBits = 0, noOfIndexBits = 0;
    unsigned int tagLocation = 0, indexLocation = 0, tagAddress = 0, tempAdd=0, temptagLocation = 0;

    noOfBlockBits = (int)log2(cache_ds.BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.c_numOfSets);


    cache_ds.NUM_READ +=1;
    extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[indexLocation + (i*cache_ds.c_numOfSets)] == tagAddress )	//Checking Tag Entries
        {

            if( cache_ds.VALID[indexLocation + (i*cache_ds.c_numOfSets)] != 0 )
            {
                LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds.c_numOfSets) ) );

                return(0);
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[indexLocation + (i*cache_ds.c_numOfSets)] == 0 )
        {
            cache_ds.VALID[indexLocation + (i*cache_ds.c_numOfSets)] = 1;
            tagLocation =  indexLocation + (i*cache_ds.c_numOfSets);
            foundInvalidEntry = 1;
            break;
        }
    }

    if( foundInvalidEntry == 1 )
    {
        cache_ds.NUM_READ_MISS += 1;

        if( cache_ds.nextLevel!=NULL )
        {
            readFromAddress(*cache_ds.nextLevel, addressInInt, victimCache, 0);
        }

        cache_ds.memoryAccessCounter += 1;		//increase the memory traffic counter
        cache_ds.TAGS[tagLocation] = tagAddress;
        cache_ds.DIRTY[tagLocation] = 0;
        LRUForMiss(cache_ds, indexLocation, &temptagLocation);
        cache_ds.LRUCounter[tagLocation] = 0;
        return(0);
    }



    if( vc_size != 0 )
    {

        LRUForMiss(cache_ds, indexLocation, &tagLocation);
        cache_ds.LRUCounter[tagLocation] = 0;

        readFromVictimCache(victimCache, addressInInt, cache_ds, tagLocation, 'r');

        return(0);
    }

    cache_ds.NUM_READ_MISS += 1;

    cache_ds.memoryAccessCounter += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, indexLocation, &tagLocation);
    cache_ds.LRUCounter[tagLocation] = 0;


    if( (int)cache_ds.DIRTY[tagLocation] == 1 )
    {

        cache_ds.memoryAccessCounter += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            tempAdd = cache_ds.TAGS[tagLocation];
            tempAdd = ( ( (tempAdd<<noOfIndexBits)|(tagLocation%cache_ds.c_numOfSets) )<<noOfBlockBits );
            writeToAddress(*cache_ds.nextLevel, tempAdd, victimCache, 0);
        }
        cache_ds.NUM_WRITEBACK += 1;
        cache_ds.DIRTY[tagLocation] = 0;
    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, addressInInt, victimCache, 0);
    }

    cache_ds.TAGS[tagLocation] = tagAddress;

    return(0);
}




int readFromVictimCache(Cache victimCache, unsigned int addressInInt, Cache &cache_ds, unsigned int tagLocationCache, char rw)
{
    int noOfBlockBits = 0, noOfIndexBitsCache = 0, i=0;
    unsigned int tagAddress = 0, indexLocOfCache = 0, vcTagLocation = 0;
    unsigned int tagAddressOfCache, tempTagAddress, constructedAddress = 0;

    noOfBlockBits = (int)log2(victimCache.BLOCKSIZE);
    tagAddress = addressInInt>>noOfBlockBits;

    noOfIndexBitsCache = (int)log2(cache_ds.c_numOfSets);

    extractAddressParams(addressInInt, cache_ds, &indexLocOfCache, &tagAddressOfCache);

    for( i=0; i< (int)victimCache.ASSOC; i++)
    {
        if( victimCache.TAGS[i] == tagAddress )	//Checking Tag Entries
        {
            if( victimCache.VALID[i] != 0 )
            {

                tempTagAddress = cache_ds.TAGS[tagLocationCache];
                cache_ds.TAGS[tagLocationCache] = tagAddressOfCache;
                victimCache.TAGS[i] = ( (tempTagAddress<<noOfIndexBitsCache)|(indexLocOfCache%cache_ds.c_numOfSets) );

                tempTagAddress =  cache_ds.DIRTY[tagLocationCache];
                cache_ds.DIRTY[tagLocationCache] = victimCache.DIRTY[i];
                victimCache.DIRTY[i] = tempTagAddress;
                LRUForHitVC(victimCache, i);
                victimCache.noOfSwaps += 1;
                if( rw == 'w' )
                    cache_ds.DIRTY[tagLocationCache] = 1;
                return(0);
            }
        }
    }

    //It's a Cache Miss
    if( rw == 'r' )
        cache_ds.NUM_READ_MISS += 1;
    else
        cache_ds.NUM_WRITE_MISS += 1;

    //printf("\nMISS VICTIM CACHE");

    cache_ds.memoryAccessCounter += 1;


    LRUForMissVC(victimCache, &vcTagLocation);
    if( (int)victimCache.DIRTY[vcTagLocation] == 1)
    {
        //printf("\nVICTIM CACHE WRITE BACK");

        if( cache_ds.nextLevel!=NULL )
        {
            constructedAddress = victimCache.TAGS[vcTagLocation]<<noOfBlockBits;
            writeToAddress(*cache_ds.nextLevel, constructedAddress, victimCache, 0);

        }
        victimCache.NUM_WRITEBACK += 1;
        cache_ds.memoryAccessCounter += 1;
    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, addressInInt, victimCache, 0);
    }

    tempTagAddress = cache_ds.TAGS[tagLocationCache];
    victimCache.TAGS[vcTagLocation] = ( (tempTagAddress<<noOfIndexBitsCache)|(indexLocOfCache%cache_ds.c_numOfSets) );

    tempTagAddress =  cache_ds.DIRTY[tagLocationCache];
    victimCache.DIRTY[vcTagLocation] = tempTagAddress;

    victimCache.VALID[vcTagLocation] = 1;
    cache_ds.TAGS[tagLocationCache] = tagAddressOfCache;

    if( rw == 'r' )
        cache_ds.DIRTY[tagLocationCache] = 0;
    else
        cache_ds.DIRTY[tagLocationCache] = 1;

    return(0);
}





//Recursive solution

int writeToAddress(Cache &cache_ds, unsigned int addressInInt, Cache victimCache, unsigned int vc_size)
{

    int i=0, foundInvalidEntry = 0;
    unsigned int tagLocation = 0, indexLocation = 0, tagAddress = 0, tempAdd = 0, temptagLocation = 0;

    int noOfBlockBits = 0, noOfIndexBits = 0;

    noOfBlockBits = log2(cache_ds.BLOCKSIZE);
    noOfIndexBits = log2(cache_ds.c_numOfSets);

    cache_ds.NUM_WRITE += 1;
    extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[indexLocation + (i*cache_ds.c_numOfSets)] == tagAddress )	//Checking Tag Entries
        {
            if( cache_ds.VALID[indexLocation + (i*cache_ds.c_numOfSets)] != 0 )
            {

                cache_ds.DIRTY[indexLocation + (i*cache_ds.c_numOfSets)] = 1;
                LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds.c_numOfSets) ) );

                return(0);
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[indexLocation + (i*cache_ds.c_numOfSets)] == 0 )
        {
            cache_ds.VALID[indexLocation + (i*cache_ds.c_numOfSets)] = 1;
            tagLocation =  indexLocation + (i*cache_ds.c_numOfSets);
            foundInvalidEntry = 1;
            break;
        }
    }


    if( foundInvalidEntry == 1 )
    {
        cache_ds.NUM_WRITE_MISS += 1;

        if( cache_ds.nextLevel!=NULL )
        {
            readFromAddress(*cache_ds.nextLevel, addressInInt, victimCache, 0);
        }
        cache_ds.memoryAccessCounter += 1;		//increase the memory traffic counter
        cache_ds.TAGS[tagLocation] = tagAddress;
        cache_ds.DIRTY[tagLocation] = 1;

        LRUForMiss(cache_ds, indexLocation, &temptagLocation);
        cache_ds.LRUCounter[tagLocation] = 0;
        return(0);
    }


    if( vc_size != 0 )
    {
        LRUForMiss(cache_ds, indexLocation, &tagLocation);
        cache_ds.LRUCounter[tagLocation] = 0;

        readFromVictimCache(victimCache, addressInInt, cache_ds, tagLocation, 'w');

        return(0);
    }

    cache_ds.NUM_WRITE_MISS += 1;

    cache_ds.memoryAccessCounter += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, indexLocation, &tagLocation);
    cache_ds.LRUCounter[tagLocation] = 0;

    if( (int)cache_ds.DIRTY[tagLocation] == 1 )
    {
        cache_ds.memoryAccessCounter += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            tempAdd = cache_ds.TAGS[tagLocation];
            tempAdd = ( ( (tempAdd<<noOfIndexBits)|(tagLocation%cache_ds.c_numOfSets) )<<noOfBlockBits );
            writeToAddress(*cache_ds.nextLevel, tempAdd, victimCache, 0);
        }
        cache_ds.NUM_WRITEBACK += 1;

    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, addressInInt, victimCache, 0);
    }

    cache_ds.DIRTY[tagLocation] = 1;
    cache_ds.TAGS[tagLocation] = tagAddress;


    return(0);
}




void LRUForHit(Cache &l1Cache, unsigned int indexLocation, unsigned int tagLocation)
{
    int i = 0;

    for( i=0; i< (int)l1Cache.ASSOC; i++)
    {
        if( (int)l1Cache.LRUCounter[indexLocation + (i*l1Cache.c_numOfSets)] < (int)l1Cache.LRUCounter[tagLocation] )
        {
            l1Cache.LRUCounter[indexLocation + (i*l1Cache.c_numOfSets)] = ((int)l1Cache.LRUCounter[indexLocation + (i*l1Cache.c_numOfSets)]) + 1;
        }
    }

    l1Cache.LRUCounter[tagLocation] = 0;
}



void LRUForMiss(Cache l1Cache, unsigned int indexLocation, unsigned int* tagLocation)
{
    unsigned int i = 0;
    int max = -1;
    *tagLocation = 0;
    //printf("\nL1 UPDATE LRU");
    for( i=0; i<l1Cache.ASSOC; i++)
    {
        if( (int)l1Cache.LRUCounter[indexLocation + (i*l1Cache.c_numOfSets)] > (int)max )
        {
            max = l1Cache.LRUCounter[indexLocation + (i*l1Cache.c_numOfSets)];
            *tagLocation = ( indexLocation + (i*l1Cache.c_numOfSets) );
        }
    }


    for( i=0; i<l1Cache.ASSOC; i++)
    {
        l1Cache.LRUCounter[indexLocation + (i*l1Cache.c_numOfSets)] = ((int)l1Cache.LRUCounter[indexLocation + (i*l1Cache.c_numOfSets)]) + 1;
    }
}




void LRUForHitVC(Cache &cache_ds, unsigned int indexLocation)
{
    int i = 0;

    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( (int)cache_ds.LRUCounter[i] < (int)cache_ds.LRUCounter[indexLocation] )
        {
            cache_ds.LRUCounter[i] = ((int)cache_ds.LRUCounter[i] + 1);
        }
    }

    cache_ds.LRUCounter[indexLocation] = 0;
}



void LRUForMissVC(Cache &cache_ds, unsigned int* tagLocation)
{
    unsigned int i = 0;
    int max = -1;
    *tagLocation = 0;

    for( i=0; i<cache_ds.ASSOC; i++)
    {
        if( (int)cache_ds.LRUCounter[i] > (int)max )
        {
            max = cache_ds.LRUCounter[i];
            *tagLocation = i;
        }
    }


    for( i=0; i<cache_ds.ASSOC; i++)
    {
        cache_ds.LRUCounter[i] = ((int)cache_ds.LRUCounter[i] + 1);
    }

    cache_ds.LRUCounter[*tagLocation] = 0;

}