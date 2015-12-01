#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "cache.h"

void Cache::init(unsigned int block, unsigned int size, unsigned int assoc, unsigned int set, unsigned int tag) {
    BLOCK = block;
    SIZE = size;
    ASSOC = assoc;
    SET = set;
    TAG = tag;

    LRUC = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    TAGS = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    DIRTY = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );
    VALID = (unsigned int*)malloc( (TAG*sizeof(unsigned int)) );

    memset( LRUC, 0, (sizeof(LRUC[0])*TAG) );
    memset( TAGS, 0, (sizeof(TAGS[0])*TAG) );
    memset( DIRTY, 0, (sizeof(DIRTY[0])*TAG) );
    memset( VALID, 0, (sizeof(VALID[0])*TAG) );

    NUM_READ = 0;
    NUM_WRITE = 0;
    NUM_READ_MISS = 0;
    NUM_WRITE_MISS = 0;
    NUM_ACCESS = 0;
    NUM_WRITEBACK = 0;
    MISS_RATE = 0;
    MISS_PENALTY = 0;
    HIT_TIME = 0;
    ACCESS_TIME = 0;
    nextLevel = NULL;
}

void Cache::transAddress(unsigned int address) {
    block = (int)log2(BLOCK);
    index = (int)log2(SET);
    int tmp = 0;
    for( int i=0; i<index; i++)
        tmp = tmp<<1 | 1;
    INDEX = address>>block & tmp;
    TAG_ADD = address>>(block+index);
}

void Cache::Hit(int index) {
    for(unsigned int i=0; i<ASSOC; i++)
        if( LRUC[INDEX + (i*SET)] < LRUC[index] )
            LRUC[INDEX + (i*SET)] = (LRUC[INDEX + (i*SET)]) + 1;
    LRUC[index] = 0;
}

void Cache::Miss() {
    unsigned int max = 0, i;
    TAG_LOC = 0;
    for(i=0; i<ASSOC; i++)
        if( LRUC[INDEX + (i*SET)] > max ) {
            max = LRUC[INDEX + (i*SET)];
            TAG_LOC = ( INDEX + (i*SET) );
        }
    for(i=0; i<ASSOC; i++)
        LRUC[INDEX + (i*SET)] = (LRUC[INDEX + (i*SET)]) + 1;
}

void CACHE::init(unsigned int block, unsigned int size1, unsigned int assoc1, unsigned int sizev, unsigned int size2, unsigned int assoc2, char *trace) {
    TRACE_FILE = trace;
    L1.init( block, size1, assoc1, size1/(block*assoc1), size1/block );
    if( size2 > 0 ) {
        L1.nextLevel = &L2;
        L2.init( block, size2, assoc2, size2/(block*assoc2), size2/block );
    }
    if( sizev > 0 )
        Victim.init( block, sizev, sizev/block, 1, sizev/block );
}

void CACHE::input() {
    char rw;
    unsigned int address;
    while( scanf(" %c %x",&rw,&address)!=EOF ) {
        if( rw == 'r' || rw == 'R')
            readFromAddress(L1, address, Victim.SIZE);
        else if( rw == 'w' || rw == 'W')
            writeToAddress(L1, address, Victim.SIZE);
    }
}

void CACHE::output() {
    int i,j,k;
    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE:                    %d\n", L1.BLOCK);
    printf("L1_SIZE:                      %d\n", L1.SIZE);
    printf("L1_ASSOC:                     %d\n", L1.ASSOC);
    printf("Victim_Cache_SIZE:            %d\n", Victim.SIZE);
    printf("L2_SIZE:                      %d\n", L2.SIZE);
    printf("L2_ASSOC:                     %d\n", L2.ASSOC);
    printf("trace_file:                   %s\n", TRACE_FILE);
    printf("===================================\n");
    
    printf("===== L1 contents =====\n");  //Part L1
    for(i=0; i<L1.SET; i++) {
        //sort L1
        for(j=1; j<L1.ASSOC; j++) {
            LRU_KEY = L1.LRUC[i+(j*L1.SET)];
            TAG_KEY = L1.TAGS[i+(j*L1.SET)];
            DIR_KEY = L1.DIRTY[i+(j*L1.SET)];
            for(k = j-1; k>=0 && LRU_KEY<L1.LRUC[i+(k*L1.SET)]; k--) {
                L1.LRUC[i+((k+1)*L1.SET)] = L1.LRUC[i+(k*L1.SET)];
                L1.TAGS[i+((k+1)*L1.SET)] = L1.TAGS[i+(k*L1.SET)];
                L1.DIRTY[i+((k+1)*L1.SET)] = L1.DIRTY[i+(k*L1.SET)];
            }
            L1.LRUC[i+((k+1)*L1.SET)] = LRU_KEY;
            L1.TAGS[i+((k+1)*L1.SET)] = TAG_KEY;
            L1.DIRTY[i+((k+1)*L1.SET)] = DIR_KEY;
        }
        printf("set %d: ", i);
        for(j=0; j<L1.ASSOC; j++)
            printf("%x %c  ",L1.TAGS[i+(j*L1.SET)], L1.DIRTY[i+(j*L1.SET)]?'D':' ');
        puts("");
    }

    if( Victim.SIZE ) {  //Part Victim
        printf("===== Victim Cache contents =====\n");
        printf("set 0: ");
        //sort Victim
        for( i=1; i<Victim.ASSOC; i++) {
            LRU_KEY = Victim.LRUC[i];
            TAG_KEY = Victim.TAGS[i];
            DIR_KEY = Victim.DIRTY[i];
            for( j = i-1; j>=0 && LRU_KEY<Victim.LRUC[j];j--) {
                Victim.LRUC[j+1] = Victim.LRUC[j];
                Victim.TAGS[j+1] = Victim.TAGS[j];
                Victim.DIRTY[j+1] = Victim.DIRTY[j];
            }
            Victim.LRUC[j+1] = LRU_KEY;
            Victim.TAGS[j+1] = TAG_KEY;
            Victim.DIRTY[j+1] = DIR_KEY;
        }
        for(i=0; i<Victim.ASSOC; i++)
            printf("%x %c  ",Victim.TAGS[i], Victim.DIRTY[i]?'D':' ');
        puts("");
    }

    if( L2.SIZE ) {  //Part L2
        printf("===== L2 contents =====\n");
        for( i=0; i<L2.SET; i++) {
            //sort L2
            for( j=1; j<L2.ASSOC; j++) {
                LRU_KEY = L2.LRUC[i+(j*L2.SET)];
                TAG_KEY = L2.TAGS[i+(j*L2.SET)];
                DIR_KEY = L2.DIRTY[i+(j*L2.SET)];
                for( k = j-1; k>=0 && LRU_KEY<L2.LRUC[i+(k*L2.SET)]; k-- ) {
                    L2.LRUC[i+((k+1)*L2.SET)] = L2.LRUC[i+(k*L2.SET)];
                    L2.TAGS[i+((k+1)*L2.SET)] = L2.TAGS[i+(k*L2.SET)];
                    L2.DIRTY[i+((k+1)*L2.SET)] = L2.DIRTY[i+(k*L2.SET)];
                }
                L2.LRUC[i+((k+1)*L2.SET)] = LRU_KEY;
                L2.TAGS[i+((k+1)*L2.SET)] = TAG_KEY;
                L2.DIRTY[i+((k+1)*L2.SET)] = DIR_KEY;

            }
            printf("set %d: ", i);
            for(j=0; j<L2.ASSOC; j++)
                printf("%x %c  ",L2.TAGS[i+(j*L2.SET)], L2.DIRTY[i+(j*L2.SET)]?'D':' ');
            puts("");
        }
    }

    printf("====== Simulation results (raw) ======\n");
    
    L1.MISS_RATE = double(L1.NUM_READ_MISS + L1.NUM_WRITE_MISS)/double(L1.NUM_READ + L1.NUM_WRITE );
    L1.MISS_PENALTY = 20 + 0.5*L1.BLOCK/16;
    L1.HIT_TIME = 0.25 + 2.5*L1.SIZE/(512*1024) + 0.025*L1.BLOCK/16 + 0.025*L1.ASSOC;
    L1.ACCESS_TIME = L1.HIT_TIME + L1.MISS_RATE*L1.MISS_PENALTY;
    
    printf("a. number of L1 reads:                %d\n", L1.NUM_READ);
    printf("b. number of L1 read misses:          %d\n", L1.NUM_READ_MISS);
    printf("c. number of L1 writes:\t\t     %d\n",       L1.NUM_WRITE);
    printf("d. number of L1 write misses:         %d\n", L1.NUM_WRITE_MISS);
    printf("e. L1 miss rate:                      %.4f\n", L1.MISS_RATE);
    printf("f. number of swaps:                   %d\n", Victim.NUM_SWAP);
    printf("g. number of victim cache writeback:  %d\n", Victim.NUM_WRITEBACK);
    
    L2.MISS_RATE = (double)L2.NUM_READ_MISS/(double)L2.NUM_READ;
    L2.MISS_PENALTY = 20 + 0.5*L2.BLOCK/16;
    L2.HIT_TIME = 2.5 + 2.5*L2.SIZE/(512*1024) + 0.025*L2.BLOCK/16 + 0.025*L2.ASSOC;
    L2.ACCESS_TIME = L1.HIT_TIME + L1.MISS_RATE*( L2.HIT_TIME + L2.MISS_RATE*L2.MISS_PENALTY );

    printf("h. number of L2 reads:                %d\n", L2.NUM_READ);
    printf("i. number of L2 read misses:          %d\n", L2.NUM_READ_MISS);
    printf("j. number of L2 writes:               %d\n", L2.NUM_WRITE);
    printf("k. number of L2 write misses:         %d\n", L2.NUM_WRITE_MISS);
    if( L2.SIZE )
        printf("l. L2 miss rate:                      %.4f\n", L2.MISS_RATE);
    else
        printf("l. L2 miss rate:                      0\n");
    printf("m. number of L2 writebacks:           %d\n", L2.NUM_WRITEBACK);
    printf("n. total memory traffic:              %d\n", L2.SIZE? L2.NUM_ACCESS:L1.NUM_ACCESS);
    
    printf("==== Simulation results (performance) ====\n");
    printf("1. average access time:         %.4f ns\n", L2.SIZE ? L2.ACCESS_TIME : L1.ACCESS_TIME);
}

void CACHE::readFromAddress(Cache &cache, unsigned int address, unsigned int vc) {
    unsigned int i, flag = 0;
    cache.NUM_READ ++;
    cache.transAddress(address);

    for(i=0; i<cache.ASSOC; i++)
        if( cache.TAGS[cache.INDEX + (i*cache.SET)] == cache.TAG_ADD && cache.VALID[cache.INDEX + (i*cache.SET)] ) {
            cache.Hit( cache.INDEX + (i*cache.SET) );
            return;
        }

    for(i=0; i<cache.ASSOC; i++)
        if( cache.VALID[cache.INDEX + (i*cache.SET)] == 0 ) {
            cache.VALID[cache.INDEX + (i*cache.SET)] = 1;
            cache.TAG_LOC =  cache.INDEX + (i*cache.SET);
            flag = 1;
            break;
        }

    if( flag ) {  //Found Invalid
        cache.NUM_READ_MISS ++;
        if( cache.nextLevel!=NULL )
            readFromAddress(L2, address, 0);
        cache.NUM_ACCESS ++;
        cache.TAGS[cache.TAG_LOC] = cache.TAG_ADD;
        cache.DIRTY[cache.TAG_LOC] = 0;
        cache.LRUC[cache.TAG_LOC] = 0;
        cache.Miss();
        return;
    }

    if( vc ) {  //Victim Exist
        cache.Miss();
        cache.LRUC[cache.TAG_LOC] = 0;
        readFromVictim(cache, address, 'r');
        return;
    }

    cache.NUM_READ_MISS ++;
    cache.NUM_ACCESS ++;
    cache.Miss();
    cache.LRUC[cache.TAG_LOC] = 0;

    if( cache.DIRTY[cache.TAG_LOC] ) {
        cache.NUM_ACCESS ++;
        if( cache.nextLevel!=NULL ) {
            unsigned int tmp = cache.TAGS[cache.TAG_LOC];
            tmp = ( ( (tmp<<cache.index)|(cache.TAG_LOC%cache.SET) )<<cache.block );
            writeToAddress(L2, tmp, 0);
        }
        cache.NUM_WRITEBACK ++;
        cache.DIRTY[cache.TAG_LOC] = 0;
    }

    if( cache.nextLevel!=NULL )
        readFromAddress(L2, address, 0);

    cache.TAGS[cache.TAG_LOC] = cache.TAG_ADD;
}

void CACHE::readFromVictim(Cache &cache, unsigned int address, char rw) {
    unsigned int i, tmp;
    cache.transAddress(address);

    for(i=0; i<Victim.ASSOC; i++)
        if( Victim.TAGS[i] == address>>cache.block && Victim.VALID[i] ) {
            tmp = cache.TAGS[cache.TAG_LOC];
            cache.TAGS[cache.TAG_LOC] = cache.TAG_ADD;
            Victim.TAGS[i] = ( (tmp<<cache.index)|(cache.INDEX%cache.SET) );

            tmp = cache.DIRTY[cache.TAG_LOC];
            cache.DIRTY[cache.TAG_LOC] = Victim.DIRTY[i];
            Victim.DIRTY[i] = tmp;
            Victim.Hit(i);
            Victim.NUM_SWAP ++;
            if( rw == 'w' )
                cache.DIRTY[cache.TAG_LOC] = 1;
            return;
        }

    //Cache Miss
    if( rw == 'r' )
        cache.NUM_READ_MISS ++;
    else
        cache.NUM_WRITE_MISS ++;
    cache.NUM_ACCESS ++;
    Victim.Miss();
    Victim.LRUC[Victim.TAG_LOC] = 0;

    if( Victim.DIRTY[Victim.TAG_LOC] ) {
        if( cache.nextLevel!=NULL ) {
            tmp = Victim.TAGS[Victim.TAG_LOC]<<cache.block;
            writeToAddress(L2, tmp, 0);
        }
        Victim.NUM_WRITEBACK ++;
        cache.NUM_ACCESS ++;
    }

    if( cache.nextLevel!=NULL )
        readFromAddress(L2, address, 0);

    tmp = cache.TAGS[cache.TAG_LOC];
    Victim.TAGS[Victim.TAG_LOC] = ( (tmp<<cache.index)|(cache.INDEX%cache.SET) );
    tmp = cache.DIRTY[cache.TAG_LOC];
    Victim.DIRTY[Victim.TAG_LOC] = tmp;
    Victim.VALID[Victim.TAG_LOC] = 1;
    cache.TAGS[cache.TAG_LOC] = cache.TAG_ADD;
    cache.DIRTY[cache.TAG_LOC] = rw-'r'?1:0;
}

void CACHE::writeToAddress(Cache &cache, unsigned int address, unsigned int vc) {
    unsigned int i, flag = 0;
    cache.NUM_WRITE ++;
    cache.transAddress(address);

    for(i=0; i<cache.ASSOC; i++)
        if( cache.TAGS[cache.INDEX + (i*cache.SET)] == cache.TAG_ADD && cache.VALID[cache.INDEX + (i*cache.SET)] ) {
            cache.DIRTY[cache.INDEX + (i*cache.SET)] = 1;
            cache.Hit( cache.INDEX + (i*cache.SET) );
            return;
        }

    for(i=0; i<cache.ASSOC; i++)
        if( cache.VALID[cache.INDEX + (i*cache.SET)] == 0 ) {
            cache.VALID[cache.INDEX + (i*cache.SET)] = 1;
            cache.TAG_LOC =  cache.INDEX + (i*cache.SET);
            flag = 1;
            break;
        }

    if( flag ) {  //Found Invalid
        cache.NUM_WRITE_MISS ++;
        if( cache.nextLevel!=NULL )
            readFromAddress(L2, address, 0);
        cache.NUM_ACCESS ++;
        cache.TAGS[cache.TAG_LOC] = cache.TAG_ADD;
        cache.DIRTY[cache.TAG_LOC] = 1;
        cache.LRUC[cache.TAG_LOC] = 0;
        cache.Miss();
        return;
    }

    if( vc ) {  //Victim Exist
        cache.Miss();
        cache.LRUC[cache.TAG_LOC] = 0;
        readFromVictim(cache, address, 'w');
        return;
    }

    cache.NUM_WRITE_MISS ++;
    cache.NUM_ACCESS ++;
    cache.Miss();
    cache.LRUC[cache.TAG_LOC] = 0;

    if( cache.DIRTY[cache.TAG_LOC] ) {
        cache.NUM_ACCESS ++;
        if( cache.nextLevel!=NULL ) {
            unsigned int tmp = cache.TAGS[cache.TAG_LOC];
            tmp = ( ( (tmp<<cache.index)|(cache.TAG_LOC%cache.SET) )<<cache.block );
            writeToAddress(L2, tmp, 0);
        }
        cache.NUM_WRITEBACK ++;
    }

    if( cache.nextLevel!=NULL )
        readFromAddress(L2, address, 0);

    cache.DIRTY[cache.TAG_LOC] = 1;
    cache.TAGS[cache.TAG_LOC] = cache.TAG_ADD;
}