#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "cache.h"

void Cache::init(unsigned int size, unsigned int assoc, unsigned int set, unsigned int tag) {
    SIZE = size;
    ASSOC = assoc;
    SET = set;
    TAG = tag;
    
    TAGS = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    DIRTY = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    VALID = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    LRUCounter = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );

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
    MISS_RATE =0;
    MISS_PENALTY = 0;
    HIT_TIME = 0;
    ACCESS_TIME = 0;
    nextLevel = NULL;
}

void CACHE::init(unsigned int block, unsigned int size1, unsigned int assoc1, unsigned int sizev, unsigned int size2, unsigned int assoc2, char *trace) {
    TRACE_FILE = trace;
    BLOCKSIZE = block;
    L1.init(size1, assoc1, size1/(block*assoc1), size1/block);
    if( size2 > 0 ){
        L1.nextLevel = &L2;
        L2.init(size2, assoc2, size2/(block*assoc2), size2/block);
    }
    if( sizev > 0 )
        Victim.init(sizev, sizev/block, 1, sizev/block);
}

void CACHE::input() {
    char rw;
    unsigned int address;
    while( scanf(" %c %x",&rw,&address)!=EOF ) {
        //transAddress()
        if( rw == 'r' || rw == 'R')
            readFromAddress(L1, address, Victim.SIZE);
        else if( rw == 'w' || rw == 'W')
            writeToAddress(L1, address, Victim.SIZE);
    }
}

void CACHE::output() {
    int i,j,k;
    printf("===== Simulator configuration =====\n");
    printf("%-30s%-d\n", "BLOCKSIZE:", BLOCKSIZE);
    printf("%-30s%-d\n", "L1_SIZE:", L1.SIZE);
    printf("%-30s%-d\n", "L1_ASSOC:", L1.ASSOC);
    printf("%-30s%-d\n", "Victim_Cache_SIZE:", Victim.SIZE);
    printf("%-30s%-d\n", "L2_SIZE:", L2.SIZE);
    printf("%-30s%-d\n", "L2_ASSOC:", L2.ASSOC);
    printf("%-30s%-s\n", "trace_file:", TRACE_FILE);
    printf("===================================\n");
    printf("===== L1 contents =====\n");

    for( i=0; i<L1.SET; i++)
    {
        //sort L1 cache based on LRU counter
        for( j=1; j<L1.ASSOC; j++)
        {
            key = L1.LRUCounter[i + (j*L1.SET)];
            tagKey = L1.TAGS[i + (j*L1.SET)];
            dirtyKey = L1.DIRTY[i + (j*L1.SET)];
            k = j-1;

            while( (k>=0) && ( key < ( L1.LRUCounter[i + (k*L1.SET)] ) ) )
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
                printf("%x D  ",L1.TAGS[i + (j*L1.SET)]);
            else
                printf("%x    ",L1.TAGS[i + (j*L1.SET)]);
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

            while( ((j>=0) && (key<Victim.LRUCounter[j]) ))
            {
                Victim.LRUCounter[j+1] = Victim.LRUCounter[j];
                Victim.TAGS[j+1] = Victim.TAGS[j];
                Victim.DIRTY[j+1] = Victim.DIRTY[j];

                j = j-1;
            }

            Victim.LRUCounter[j+1] = key;
            Victim.TAGS[j+1] = tagKey;
            Victim.DIRTY[j+1] = dirtyKey;
        }




        for( i=0; i<Victim.ASSOC; i++) {
            if( Victim.DIRTY[i] )
                printf("%x D  ",Victim.TAGS[i]);
            else
                printf("%x    ",Victim.TAGS[i]);
        }
        puts("");
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

                while( (k>=0) && ( key < ( L2.LRUCounter[i + (k*L2.SET)] ) ) )
                {
                    L2.LRUCounter[i + ((k+1)*L2.SET)] = L2.LRUCounter[i + (k*L2.SET)];
                    L2.TAGS[i + ((k+1)*L2.SET)] = L2.TAGS[i + (k*L2.SET)];
                    L2.DIRTY[i + ((k+1)*L2.SET)] = L2.DIRTY[i + (k*L2.SET)];

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
                    printf("%x D  ",L2.TAGS[i + (j*L2.SET)]);
                else
                    printf("%x    ",L2.TAGS[i + (j*L2.SET)]);
            }
            printf("\n");
        }
    }


    //L1 calculation
    L1.MISS_RATE = double(L1.NUM_READ_MISS + L1.NUM_WRITE_MISS)/double(L1.NUM_READ + L1.NUM_WRITE );
    L1.MISS_PENALTY = 20 + 0.5*BLOCKSIZE/16;
    L1.HIT_TIME = 0.25 + 2.5*L1.SIZE/(512*1024) + 0.025*BLOCKSIZE/16 + 0.025*L1.ASSOC;
    L1.ACCESS_TIME = L1.HIT_TIME + L1.MISS_RATE*L1.MISS_PENALTY;


    printf("====== Simulation results (raw) ======\n");
    printf("%-38s%d\n", "a. number of L1 reads:", L1.NUM_READ);
    printf("%-38s%d\n", "b. number of L1 read misses:", L1.NUM_READ_MISS);
    printf("c. number of L1 writes:\t\t     %d\n", L1.NUM_WRITE);
    printf("%-38s%d\n", "d. number of L1 write misses:", L1.NUM_WRITE_MISS);
    printf("%-38s%-2.4f\n", "e. L1 miss rate:", L1.MISS_RATE);
    printf("%-38s%d\n", "f. number of swaps:", Victim.NUM_SWAP);
    printf("%-38s%d\n", "g. number of victim cache writeback:", Victim.NUM_WRITEBACK);

    //L2 cache calculation
    L2.MISS_RATE = ( (double)( (int)L2.NUM_READ_MISS )/(double)( (int)L2.NUM_READ ) );
    L2.MISS_PENALTY = (20 + 0.5*( ((double)BLOCKSIZE/16) ) );



    L2.HIT_TIME = ( 2.5 + 2.5*( (double)L2.SIZE/(512*1024) ) + 0.025*( (double)BLOCKSIZE/16 ) + 0.025*( (double)L2.ASSOC ) );

    printf("%-38s%d\n", "h. number of L2 reads:", L2.NUM_READ);
    printf("%-38s%d\n", "i. number of L2 read misses:", L2.NUM_READ_MISS);
    printf("%-38s%d\n", "j. number of L2 writes:", L2.NUM_WRITE);
    printf("%-38s%d\n", "k. number of L2 write misses:", L2.NUM_WRITE_MISS);
    if( L2.SIZE == 0 )
        printf("%-38s%c\n", "l. L2 miss rate:", '0');
    else
        printf("%-38s%-2.4f\n", "l. L2 miss rate:", L2.MISS_RATE);

    printf("%-38s%d\n", "m. number of L2 writebacks:", L2.NUM_WRITEBACK);

    if(L2.SIZE == 0)
        printf("%-38s%d\n", "n. total memory traffic:", L1.NUM_ACCESS);
    else
        printf("%-38s%d\n", "n. total memory traffic:", L2.NUM_ACCESS);


    if(L2.SIZE != 0)
    {
        L2.ACCESS_TIME = ( L1.HIT_TIME + ( L1.MISS_RATE*( L2.HIT_TIME + ( L2.MISS_RATE*L2.MISS_PENALTY ) ) ) );
    }

    printf("==== Simulation results (performance) ====\n");
    printf("1. average access time:%15.4f ns\n", L2.SIZE ? L2.ACCESS_TIME : L1.ACCESS_TIME);
}

void CACHE::extractAddressParams(unsigned int address, Cache cache_ds, unsigned int* INDEX, unsigned int* TAG_ADD)
{
    int noOfBlockBits = 0, noOfIndexBits = 0, tempIndexNo = 0, i=0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);

    *INDEX = address>>noOfBlockBits;

    for( i=0; i<noOfIndexBits; i++)
    {
        tempIndexNo = ( 1 | tempIndexNo<<1 );
    }

    *INDEX = ( *INDEX & tempIndexNo );
    *TAG_ADD = address>>(noOfBlockBits + noOfIndexBits);
}

/*void CACHE::transAddress(Cache &cache_ds, unsigned int address) {
    int block = (int)log2(BLOCKSIZE);
    int index = (int)log2(cache_ds.SET);
    int tmp = 0;
    for( int i=0; i<index; i++)
        tmp = tmp<<1 | 1;
    INDEX = address>>block & tmp;
    TAG_LOC = address>>(block+index);
}*/



//Recursive solution

void CACHE::readFromAddress(Cache &cache_ds, unsigned int address, unsigned int vc_size)
{
    int i=0, foundInvalidEntry = 0, noOfBlockBits = 0, noOfIndexBits = 0;
    unsigned int TAG_LOC = 0, INDEX = 0, TAG_ADD = 0, TMP_ADD=0, TMP_LOC = 0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);


    cache_ds.NUM_READ +=1;
    extractAddressParams(address, cache_ds, &INDEX, &TAG_ADD);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[INDEX + (i*cache_ds.SET)] == TAG_ADD )	//Checking Tag Entries
        {

            if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] != 0 )
            {
                LRUForHit(cache_ds, INDEX, ( INDEX + (i*cache_ds.SET) ) );

                return;
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] == 0 )
        {
            cache_ds.VALID[INDEX + (i*cache_ds.SET)] = 1;
            TAG_LOC =  INDEX + (i*cache_ds.SET);
            foundInvalidEntry = 1;
            break;
        }
    }

    if( foundInvalidEntry == 1 )
    {
        cache_ds.NUM_READ_MISS += 1;

        if( cache_ds.nextLevel!=NULL )
        {
            readFromAddress(*cache_ds.nextLevel, address, 0);
        }

        cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
        cache_ds.TAGS[TAG_LOC] = TAG_ADD;
        cache_ds.DIRTY[TAG_LOC] = 0;
        LRUForMiss(cache_ds, INDEX, &TMP_LOC);
        cache_ds.LRUCounter[TAG_LOC] = 0;
        return;
    }



    if( vc_size != 0 )
    {

        LRUForMiss(cache_ds, INDEX, &TAG_LOC);
        cache_ds.LRUCounter[TAG_LOC] = 0;

        readFromVictim(cache_ds, address, TAG_LOC, 'r');

        return;
    }

    cache_ds.NUM_READ_MISS += 1;

    cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, INDEX, &TAG_LOC);
    cache_ds.LRUCounter[TAG_LOC] = 0;


    if( (int)cache_ds.DIRTY[TAG_LOC] == 1 )
    {

        cache_ds.NUM_ACCESS += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            TMP_ADD = cache_ds.TAGS[TAG_LOC];
            TMP_ADD = ( ( (TMP_ADD<<noOfIndexBits)|(TAG_LOC%cache_ds.SET) )<<noOfBlockBits );
            writeToAddress(*cache_ds.nextLevel, TMP_ADD, 0);
        }
        cache_ds.NUM_WRITEBACK += 1;
        cache_ds.DIRTY[TAG_LOC] = 0;
    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, address, 0);
    }

    cache_ds.TAGS[TAG_LOC] = TAG_ADD;

    return;
}




void CACHE::readFromVictim(Cache &cache_ds, unsigned int address, unsigned int tagLocationCache, char rw)
{
    int noOfBlockBits = 0, noOfIndexBitsCache = 0, i=0;
    unsigned int TAG_ADD = 0, INDEX = 0, TAG_LOC = 0;
    unsigned int tagAddressOfCache, TMP_TAG, constructedAddress = 0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    TAG_ADD = address>>noOfBlockBits;

    noOfIndexBitsCache = (int)log2(cache_ds.SET);

    extractAddressParams(address, cache_ds, &INDEX, &tagAddressOfCache);

    for( i=0; i< (int)Victim.ASSOC; i++)
    {
        if( Victim.TAGS[i] == TAG_ADD )	//Checking Tag Entries
        {
            if( Victim.VALID[i] != 0 )
            {

                TMP_TAG = cache_ds.TAGS[tagLocationCache];
                cache_ds.TAGS[tagLocationCache] = tagAddressOfCache;
                Victim.TAGS[i] = ( (TMP_TAG<<noOfIndexBitsCache)|(INDEX%cache_ds.SET) );

                TMP_TAG =  cache_ds.DIRTY[tagLocationCache];
                cache_ds.DIRTY[tagLocationCache] = Victim.DIRTY[i];
                Victim.DIRTY[i] = TMP_TAG;
                LRUForHitVC(i);
                Victim.NUM_SWAP ++;
                if( rw == 'w' )
                    cache_ds.DIRTY[tagLocationCache] = 1;
                return;
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


    LRUForMissVC(&TAG_LOC);

    if( (int)Victim.DIRTY[TAG_LOC] == 1)
    {
        //printf("\nVICTIM CACHE WRITE BACK");

        if( cache_ds.nextLevel!=NULL )
        {
            constructedAddress = Victim.TAGS[TAG_LOC]<<noOfBlockBits;
            writeToAddress(*cache_ds.nextLevel, constructedAddress, 0);

        }
        Victim.NUM_WRITEBACK ++;
        cache_ds.NUM_ACCESS += 1;
    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, address, 0);
    }

    TMP_TAG = cache_ds.TAGS[tagLocationCache];
    Victim.TAGS[TAG_LOC] = ( (TMP_TAG<<noOfIndexBitsCache)|(INDEX%cache_ds.SET) );

    TMP_TAG =  cache_ds.DIRTY[tagLocationCache];
    Victim.DIRTY[TAG_LOC] = TMP_TAG;

    Victim.VALID[TAG_LOC] = 1;
    cache_ds.TAGS[tagLocationCache] = tagAddressOfCache;

    if( rw == 'r' )
        cache_ds.DIRTY[tagLocationCache] = 0;
    else
        cache_ds.DIRTY[tagLocationCache] = 1;
}





//Recursive solution

void CACHE::writeToAddress(Cache &cache_ds, unsigned int address, unsigned int vc_size)
{

    int i=0, foundInvalidEntry = 0;
    unsigned int TAG_LOC = 0, INDEX = 0, TAG_ADD = 0, TMP_ADD = 0, TMP_LOC = 0;

    int noOfBlockBits = 0, noOfIndexBits = 0;

    noOfBlockBits = (int)log2(BLOCKSIZE);
    noOfIndexBits = (int)log2(cache_ds.SET);

    cache_ds.NUM_WRITE += 1;
    extractAddressParams(address, cache_ds, &INDEX, &TAG_ADD);


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.TAGS[INDEX + (i*cache_ds.SET)] == TAG_ADD )	//Checking Tag Entries
        {
            if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] != 0 )
            {

                cache_ds.DIRTY[INDEX + (i*cache_ds.SET)] = 1;
                LRUForHit(cache_ds, INDEX, ( INDEX + (i*cache_ds.SET) ) );

                return;
            }
        }
    }


    for( i=0; i< (int)cache_ds.ASSOC; i++)
    {
        if( cache_ds.VALID[INDEX + (i*cache_ds.SET)] == 0 )
        {
            cache_ds.VALID[INDEX + (i*cache_ds.SET)] = 1;
            TAG_LOC =  INDEX + (i*cache_ds.SET);
            foundInvalidEntry = 1;
            break;
        }
    }


    if( foundInvalidEntry == 1 )
    {
        cache_ds.NUM_WRITE_MISS += 1;

        if( cache_ds.nextLevel!=NULL )
        {
            readFromAddress(*cache_ds.nextLevel, address, 0);
        }
        cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
        cache_ds.TAGS[TAG_LOC] = TAG_ADD;
        cache_ds.DIRTY[TAG_LOC] = 1;

        LRUForMiss(cache_ds, INDEX, &TMP_LOC);
        cache_ds.LRUCounter[TAG_LOC] = 0;
        return;
    }


    if( vc_size != 0 )
    {
        LRUForMiss(cache_ds, INDEX, &TAG_LOC);
        cache_ds.LRUCounter[TAG_LOC] = 0;

        readFromVictim(cache_ds, address, TAG_LOC, 'w');

        return;
    }

    cache_ds.NUM_WRITE_MISS += 1;

    cache_ds.NUM_ACCESS += 1;		//increase the memory traffic counter
    LRUForMiss(cache_ds, INDEX, &TAG_LOC);
    cache_ds.LRUCounter[TAG_LOC] = 0;

    if( (int)cache_ds.DIRTY[TAG_LOC] == 1 )
    {
        cache_ds.NUM_ACCESS += 1;
        if( cache_ds.nextLevel!=NULL )
        {
            TMP_ADD = cache_ds.TAGS[TAG_LOC];
            TMP_ADD = ( ( (TMP_ADD<<noOfIndexBits)|(TAG_LOC%cache_ds.SET) )<<noOfBlockBits );
            writeToAddress(*cache_ds.nextLevel, TMP_ADD, 0);
        }
        cache_ds.NUM_WRITEBACK += 1;

    }


    if( cache_ds.nextLevel!=NULL )
    {
        readFromAddress(*cache_ds.nextLevel, address, 0);
    }

    cache_ds.DIRTY[TAG_LOC] = 1;
    cache_ds.TAGS[TAG_LOC] = TAG_ADD;


    return;
}




void CACHE::LRUForHit(Cache &l1Cache, unsigned int INDEX, unsigned int TAG_LOC)
{
    int i = 0;

    for( i=0; i< (int)l1Cache.ASSOC; i++)
    {
        if( l1Cache.LRUCounter[INDEX + (i*l1Cache.SET)] < l1Cache.LRUCounter[TAG_LOC] )
        {
            l1Cache.LRUCounter[INDEX + (i*l1Cache.SET)] = (l1Cache.LRUCounter[INDEX + (i*l1Cache.SET)]) + 1;
        }
    }

    l1Cache.LRUCounter[TAG_LOC] = 0;
}



void CACHE::LRUForMiss(Cache &l1Cache, unsigned int INDEX, unsigned int* TAG_LOC)
{
    unsigned int i = 0;
    int max = 0;
    *TAG_LOC = 0;
    //printf("\nL1 UPDATE LRU");
    for( i=0; i<l1Cache.ASSOC; i++)
    {
        if( l1Cache.LRUCounter[INDEX + (i*l1Cache.SET)] > max )
        {
            max = l1Cache.LRUCounter[INDEX + (i*l1Cache.SET)];
            *TAG_LOC = ( INDEX + (i*l1Cache.SET) );
        }
    }


    for( i=0; i<l1Cache.ASSOC; i++)
    {
        l1Cache.LRUCounter[INDEX + (i*l1Cache.SET)] = (l1Cache.LRUCounter[INDEX + (i*l1Cache.SET)]) + 1;
    }
}




void CACHE::LRUForHitVC(unsigned int INDEX)
{
    int i = 0;

    for( i=0; i< (int)Victim.ASSOC; i++)
    {
        if( Victim.LRUCounter[i] < Victim.LRUCounter[INDEX] )
        {
            Victim.LRUCounter[i] = (Victim.LRUCounter[i] + 1);
        }
    }

    Victim.LRUCounter[INDEX] = 0;
}



void CACHE::LRUForMissVC(unsigned int* TAG_LOC)
{
    unsigned int i = 0;
    int max = 0;
    *TAG_LOC = 0;

    for( i=0; i<Victim.ASSOC; i++)
    {
        if( Victim.LRUCounter[i] > max )
        {
            max = Victim.LRUCounter[i];
            *TAG_LOC = i;
        }
    }


    for( i=0; i<Victim.ASSOC; i++)
    {
        Victim.LRUCounter[i] = (Victim.LRUCounter[i] + 1);
    }

    Victim.LRUCounter[*TAG_LOC] = 0;

}