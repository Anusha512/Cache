#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "cache.h"

void Cache::init(unsigned int block, unsigned int size, unsigned int assoc) {
    BLOCKSIZE = block;
    SIZE = size;
    ASSOC = assoc;
    SET = (SIZE/(BLOCKSIZE*ASSOC));
    TAG = SET*ASSOC;
    
    TAGS = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    DIRTY = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    VALID = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    LRUCounter = (int*)malloc( (TAG*sizeof(int)) );

    memset( TAGS, 0, (sizeof(TAGS[0])*TAG) );
    memset( DIRTY, 0, (sizeof(DIRTY[0])*TAG) );
    memset( VALID, 0, (sizeof(VALID[0])*TAG) );
    memset( LRUCounter, 0, (sizeof(LRUCounter[0])*TAG) );


    NUM_READ = 0;
    NUM_WRITE = 0;
    NUM_READ_MISS = 0;
    NUM_WRITE_MISS = 0;
    NUM_ACCESS = 0;
    NUM_WRITEBACK = 0;
    nextLevel = NULL;
}

void CACHE::init() {

}

void CACHE::input() {
    char rw;
    unsigned int address;
    while( scanf(" %c %x",&rw,&address)!=EOF )
    {
        if( rw == 'r' || rw == 'R')
            readFromAddress(L1, address, Victim, Victim.SIZE);
        else if( rw == 'w' || rw == 'W')
            writeToAddress(L1, address, Victim, Victim.SIZE);
    }
}

void CACHE::output() {
    int i,j,k;
    double missRateL1 = 0, accessTimeL1 = 0, missPenaltyL1 = 0, cacheHitTimeL1 = 0;
    double missRateL2 = 0, accessTimeL2 = 0, missPenaltyL2 = 0, cacheHitTimeL2 = 0;
    printf("===== Simulator configuration =====\n");
    printf("%-30s%-d\n", "BLOCKSIZE:", L1.BLOCKSIZE);
    printf("%-30s%-d\n", "L1_SIZE:", L1.SIZE);
    printf("%-30s%-d\n", "L1_ASSOC:", L1.ASSOC);
    printf("%-30s%-d\n", "Victim_Cache_SIZE:", Victim.SIZE);
    printf("%-30s%-d\n", "L2_SIZE:", L2.SIZE);
    printf("%-30s%-d\n", "L2_ASSOC:", L2.ASSOC);
    printf("%-30s%-s\n", "trace_file:", TRACE_FILE);
    printf("===================================");
    printf("\n===== L1 contents =====\n");

    for( i=0; i<L1.SET; i++)
    {
        //sort L1 cache based on LRU counter
        for( j=1; j<L1.ASSOC; j++)
        {
            key = L1.LRUCounter[i + (j*L1.SET)];
            tagKey = L1.TAGS[i + (j*L1.SET)];
            dirtyKey = L1.DIRTY[i + (j*L1.SET)];
            k = j-1;

            while( ((int)k>=0) && ( key < ( L1.LRUCounter[i + (k*L1.SET)] ) ) )
            {
                L1.LRUCounter[i + ((k+1)*L1.SET)] = L1.LRUCounter[i + (k*L1.SET)];
                L1.TAGS[i + ((k+1)*L1.SET)] = L1.TAGS[i + (k*L1.SET)];
                L1.DIRTY[i + ((k+1)*L1.SET)] = L1.DIRTY[i + (k*L1.SET)];;

                k = k-1;
            }

            L1.LRUCounter[i + ((k+1)*L1.SET)] = key;
            L1.TAGS[i + ((k+1)*L1.SET)] = tagKey;
            L1.DIRTY[i + ((k+1)*L1.SET)] = dirtyKey;

        }

        printf("set %d: ", i);
        for( j=0; j<L1.ASSOC; j++)
        {
            if( L1.DIRTY[i + (j*L1.SET)] == 1)
                printf("%-7x%-3c",L1.TAGS[i + (j*L1.SET)], 'D');
            else
                printf("%-10x",L1.TAGS[i + (j*L1.SET)]);
        }
        printf("\n");
    }


    if( Victim.SIZE != 0)
    {
        printf("===== Victim Cache contents =====\n");
        printf("set 0: ");


        //sort victim cache based on LRU counter
        for( i=1; i<Victim.ASSOC; i++)
        {
            key = Victim.LRUCounter[i];
            tagKey = Victim.TAGS[i];
            dirtyKey = Victim.DIRTY[i];
            j = i-1;

            while( ((int)j>=0) && (key<Victim.LRUCounter[j]) )
            {
                Victim.LRUCounter[j+1] = Victim.LRUCounter[j];
                Victim.TAGS[j+1] = Victim.TAGS[j];
                Victim.DIRTY[j+1] = Victim.DIRTY[j];;

                j = j-1;
            }

            Victim.LRUCounter[j+1] = key;
            Victim.TAGS[j+1] = tagKey;
            Victim.DIRTY[j+1] = dirtyKey;
        }




        for( i=0; i<Victim.ASSOC; i++)
        {
            if( Victim.DIRTY[i] == 1)
                printf("%-8x%-3c",Victim.TAGS[i], 'D');
            else
                printf("%-10x",Victim.TAGS[i]);
        }
        printf("\n");
    }



    if( L2.SIZE != 0)
    {

        printf("===== L2 contents =====\n");

        for( i=0; i<L2.SET; i++)
        {

            //sort L2 cache based on LRU counter
            for( j=1; j<L2.ASSOC; j++)
            {
                key = L2.LRUCounter[i + (j*L2.SET)];
                tagKey = L2.TAGS[i + (j*L2.SET)];
                dirtyKey = L2.DIRTY[i + (j*L2.SET)];
                k = j-1;

                while( ((int)k>=0) && ( key < ( L2.LRUCounter[i + (k*L2.SET)] ) ) )
                {
                    L2.LRUCounter[i + ((k+1)*L2.SET)] = L2.LRUCounter[i + (k*L2.SET)];
                    L2.TAGS[i + ((k+1)*L2.SET)] = L2.TAGS[i + (k*L2.SET)];
                    L2.DIRTY[i + ((k+1)*L2.SET)] = L2.DIRTY[i + (k*L2.SET)];;

                    k = k-1;
                }

                L2.LRUCounter[i + ((k+1)*L2.SET)] = key;
                L2.TAGS[i + ((k+1)*L2.SET)] = tagKey;
                L2.DIRTY[i + ((k+1)*L2.SET)] = dirtyKey;

            }


            printf("set %d: ", i);
            for( j=0; j<L2.ASSOC; j++)
            {
                if( L2.DIRTY[i + (j*L2.SET)] == 1)
                    printf("%-7x%-3c",L2.TAGS[i + (j*L2.SET)], 'D');
                else
                    printf("%-10x",L2.TAGS[i + (j*L2.SET)]);
            }
            printf("\n");
        }
    }


    //L1 calculation
    missRateL1 = ( (double)( (int)L1.NUM_READ_MISS + (int)L1.NUM_WRITE_MISS )/(double)( (int)L1.NUM_READ + (int)L1.NUM_WRITE ) );
    missPenaltyL1 = (double)(20 + 0.5*( ((double)L1.BLOCKSIZE/16) ) );



    cacheHitTimeL1 = (double)( 0.25 + 2.5*( (double)L1.SIZE/(512*1024) ) + 0.025*( (double)L1.BLOCKSIZE/16 ) + 0.025*( (double)L1.ASSOC ) );
    accessTimeL1 = (double)( (double)cacheHitTimeL1 + ( (double)missRateL1*(double)missPenaltyL1 ) );


    printf("====== Simulation results (raw) ======\n");
    printf("%-38s%-6d\n", "a. number of L1 reads:", L1.NUM_READ);
    printf("%-38s%-6d\n", "b. number of L1 read misses:", L1.NUM_READ_MISS);
    printf("%-38s%-6d\n", "c. number of L1 writes:", L1.NUM_WRITE);
    printf("%-38s%-6d\n", "d. number of L1 write misses:", L1.NUM_WRITE_MISS);
    printf("%-38s%-2.4f\n", "e. L1 miss rate:", missRateL1);


    //Victim cache results
    printf("%-38s%-6d\n", "f. number of swaps:", Victim.NUM_SWAP);
    printf("%-38s%-6d\n", "g. number of victim cache writeback:", Victim.NUM_WRITEBACK);

    //L2 cache calculation
    missRateL2 = ( (double)( (int)L2.NUM_READ_MISS )/(double)( (int)L2.NUM_READ ) );
    missPenaltyL2 = (double)(20 + 0.5*( ((double)L2.BLOCKSIZE/16) ) );



    cacheHitTimeL2 = (double)( 2.5 + 2.5*( (double)L2.SIZE/(512*1024) ) + 0.025*( (double)L2.BLOCKSIZE/16 ) + 0.025*( (double)L2.ASSOC ) );

    printf("%-38s%-6d\n", "h. number of L2 reads:", L2.NUM_READ);
    printf("%-38s%-6d\n", "i. number of L2 read misses:", L2.NUM_READ_MISS);
    printf("%-38s%-6d\n", "j. number of L2 writes:", L2.NUM_WRITE);
    printf("%-38s%-6d\n", "k. number of L2 write misses:", L2.NUM_WRITE_MISS);
    if( L2.SIZE == 0 )
        printf("%-38s%-6c\n", "l. L2 miss rate:", '0');
    else
        printf("%-38s%-2.4f\n", "l. L2 miss rate:", missRateL2);
    printf("%-38s%-6d\n", "m. number of L2 writebacks:", L2.NUM_WRITEBACK);

    if(L2.SIZE == 0)
        printf("%-38s%-6d\n", "n. total memory traffic:", L1.NUM_ACCESS);
    else
        printf("%-38s%-6d\n", "n. total memory traffic:", L2.NUM_ACCESS);


    if(L2.SIZE != 0)
    {
        accessTimeL2 = ( cacheHitTimeL1 + ( (double)missRateL1*( cacheHitTimeL2 + ( (double)missRateL2*(double)missPenaltyL2 ) ) ) );
    }

    printf("==== Simulation results (performance) ====\n");

    if(L2.SIZE != 0)
    {
        printf("1. average access time:%15.4f ns\n", accessTimeL2);  //========================================AAt calculate
    }
    else
        printf("1. average access time:%15.4f ns\n", accessTimeL1);
}

void extractAddressParams(unsigned int addressInInt, Cache cache_ds, unsigned int* indexLocation, unsigned int* tagAddress)
{
    int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;

    noOfBlockBits = (int)log2(cache_ds.BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);

    *indexLocation = addressInInt>>noOfBlockBits;

    for( i=0; i<noOfIndexBits; i++)
    {
        tempIndexNo = ( 1 | tempIndexNo<<1 );
    }

    *indexLocation = ( *indexLocation & tempIndexNo );
    *tagAddress = addressInInt>>(noOfBlockBits + noOfIndexBits);
}





//Recursive solution

int readFromAddress(Cache &cache_ds, unsigned int addressInInt, Cache &victimCache, unsigned int vc_size)
{
    int i=0, foundInvalidEntry = 0, noOfBlockBits = 0, noOfIndexBits = 0;
    unsigned int tagLocation = 0, indexLocation = 0, tagAddress = 0, tempAdd=0, temptagLocation = 0;

    noOfBlockBits = (int)log2(cache_ds.BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);


    cache_ds.NUM_READ +=1;
    extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[indexLocation + (i*cache_ds.SET)] == tagAddress )	//Checking Tag Entries
        {

            if( cache_ds.VALID[indexLocation + (i*cache_ds.SET)] != 0 )
            {
                LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds.SET) ) );

                return(0);
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[indexLocation + (i*cache_ds.SET)] == 0 )
        {
            cache_ds.VALID[indexLocation + (i*cache_ds.SET)] = 1;
            tagLocation =  indexLocation + (i*cache_ds.SET);
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

        cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
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

    cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, indexLocation, &tagLocation);
    cache_ds.LRUCounter[tagLocation] = 0;


    if( (int)cache_ds.DIRTY[tagLocation] == 1 )
    {

        cache_ds.NUM_ACCESS += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            tempAdd = cache_ds.TAGS[tagLocation];
            tempAdd = ( ( (tempAdd<<noOfIndexBits)|(tagLocation%cache_ds.SET) )<<noOfBlockBits );
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




int readFromVictimCache(Cache &victimCache, unsigned int addressInInt, Cache &cache_ds, unsigned int tagLocationCache, char rw)
{
    int noOfBlockBits = 0, noOfIndexBitsCache = 0, i=0;
    unsigned int tagAddress = 0, indexLocOfCache = 0, vcTagLocation = 0;
    unsigned int tagAddressOfCache, tempTagAddress, constructedAddress = 0;

    noOfBlockBits = (int)log2(victimCache.BLOCKSIZE);
    tagAddress = addressInInt>>noOfBlockBits;

    noOfIndexBitsCache = (int)log2(cache_ds.SET);

    extractAddressParams(addressInInt, cache_ds, &indexLocOfCache, &tagAddressOfCache);

    for( i=0; i< (int)victimCache.ASSOC; i++)
    {
        if( victimCache.TAGS[i] == tagAddress )	//Checking Tag Entries
        {
            if( victimCache.VALID[i] != 0 )
            {

                tempTagAddress = cache_ds.TAGS[tagLocationCache];
                cache_ds.TAGS[tagLocationCache] = tagAddressOfCache;
                victimCache.TAGS[i] = ( (tempTagAddress<<noOfIndexBitsCache)|(indexLocOfCache%cache_ds.SET) );

                tempTagAddress =  cache_ds.DIRTY[tagLocationCache];
                cache_ds.DIRTY[tagLocationCache] = victimCache.DIRTY[i];
                victimCache.DIRTY[i] = tempTagAddress;
                LRUForHitVC(victimCache, i);
                victimCache.NUM_SWAP ++;
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

    cache_ds.NUM_ACCESS += 1;


    LRUForMissVC(victimCache, &vcTagLocation);
    if( (int)victimCache.DIRTY[vcTagLocation] == 1)
    {
        //printf("\nVICTIM CACHE WRITE BACK");

        if( cache_ds.nextLevel!=NULL )
        {
            constructedAddress = victimCache.TAGS[vcTagLocation]<<noOfBlockBits;
            writeToAddress(*cache_ds.nextLevel, constructedAddress, victimCache, 0);

        }
        victimCache.NUM_WRITEBACK ++;
        cache_ds.NUM_ACCESS += 1;
    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, addressInInt, victimCache, 0);
    }

    tempTagAddress = cache_ds.TAGS[tagLocationCache];
    victimCache.TAGS[vcTagLocation] = ( (tempTagAddress<<noOfIndexBitsCache)|(indexLocOfCache%cache_ds.SET) );

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

int writeToAddress(Cache &cache_ds, unsigned int addressInInt, Cache &victimCache, unsigned int vc_size)
{

    int i=0, foundInvalidEntry = 0;
    unsigned int tagLocation = 0, indexLocation = 0, tagAddress = 0, tempAdd = 0, temptagLocation = 0;

    int noOfBlockBits = 0, noOfIndexBits = 0;

    noOfBlockBits = (int)log2(cache_ds.BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);

    cache_ds.NUM_WRITE += 1;
    extractAddressParams(addressInInt, cache_ds, &indexLocation, &tagAddress);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[indexLocation + (i*cache_ds.SET)] == tagAddress )	//Checking Tag Entries
        {
            if( cache_ds.VALID[indexLocation + (i*cache_ds.SET)] != 0 )
            {

                cache_ds.DIRTY[indexLocation + (i*cache_ds.SET)] = 1;
                LRUForHit(cache_ds, indexLocation, ( indexLocation + (i*cache_ds.SET) ) );

                return(0);
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[indexLocation + (i*cache_ds.SET)] == 0 )
        {
            cache_ds.VALID[indexLocation + (i*cache_ds.SET)] = 1;
            tagLocation =  indexLocation + (i*cache_ds.SET);
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
        cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
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

    cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, indexLocation, &tagLocation);
    cache_ds.LRUCounter[tagLocation] = 0;

    if( (int)cache_ds.DIRTY[tagLocation] == 1 )
    {
        cache_ds.NUM_ACCESS += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            tempAdd = cache_ds.TAGS[tagLocation];
            tempAdd = ( ( (tempAdd<<noOfIndexBits)|(tagLocation%cache_ds.SET) )<<noOfBlockBits );
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
        if( l1Cache.LRUCounter[indexLocation + (i*l1Cache.SET)] < l1Cache.LRUCounter[tagLocation] )
        {
            l1Cache.LRUCounter[indexLocation + (i*l1Cache.SET)] = (l1Cache.LRUCounter[indexLocation + (i*l1Cache.SET)]) + 1;
        }
    }

    l1Cache.LRUCounter[tagLocation] = 0;
}



void LRUForMiss(Cache &l1Cache, unsigned int indexLocation, unsigned int* tagLocation)
{
    unsigned int i = 0;
    int max = -1;
    *tagLocation = 0;
    //printf("\nL1 UPDATE LRU");
    for( i=0; i<l1Cache.ASSOC; i++)
    {
        if( l1Cache.LRUCounter[indexLocation + (i*l1Cache.SET)] > max )
        {
            max = l1Cache.LRUCounter[indexLocation + (i*l1Cache.SET)];
            *tagLocation = ( indexLocation + (i*l1Cache.SET) );
        }
    }


    for( i=0; i<l1Cache.ASSOC; i++)
    {
        l1Cache.LRUCounter[indexLocation + (i*l1Cache.SET)] = (l1Cache.LRUCounter[indexLocation + (i*l1Cache.SET)]) + 1;
    }
}




void LRUForHitVC(Cache &cache_ds, unsigned int indexLocation)
{
    int i = 0;

    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.LRUCounter[i] < cache_ds.LRUCounter[indexLocation] )
        {
            cache_ds.LRUCounter[i] = (cache_ds.LRUCounter[i] + 1);
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
        if( cache_ds.LRUCounter[i] > max )
        {
            max = cache_ds.LRUCounter[i];
            *tagLocation = i;
        }
    }


    for( i=0; i<cache_ds.ASSOC; i++)
    {
        cache_ds.LRUCounter[i] = (cache_ds.LRUCounter[i] + 1);
    }

    cache_ds.LRUCounter[*tagLocation] = 0;

}