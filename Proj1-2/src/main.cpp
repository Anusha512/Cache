#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include "cache.h"

CACHE cache;

int main(int argc, char **argv)
{


    unsigned int noOfTagEntriesL1 = 0, noOfTagEntriesL2 = 0;
    unsigned int i=0, j=0, noOfTagEntriesVC = 0, k=0, tagKey = 0, dirtyKey = 0;
    int key = 0;


    if(argc<7) {
        printf("\nUsage: ./sim_cache [BLOCKSIZE] [L1_SIZE] [L1_ASSOC] [Victim_Cache_SIZE]"
                       " [L2_SIZE] [L2_ASSOC] [trace_file] > [output_file]\n");
        return 0;
    }

    freopen(argv[7],"r",stdin);
    char *trace = argv[7];
    int id,len=(int)strlen(trace);
    for(id=len;id>=0;id--)
        if(trace[id]=='/')
            break;
    trace=trace+id+1;
    cache.TRACE_FILE=trace;

    cache.L1.init((unsigned int)atoi(argv[1]),(unsigned int)atoi(argv[2]),(unsigned int)atoi(argv[3]));
    /*cache.TRACE_FILE=trace;
    cache.L1.BLOCKSIZE = (unsigned int)atoi(argv[1]);
    cache.L1.SIZE = (unsigned int)atoi(argv[2]);
    cache.L1.ASSOC = (unsigned int)atoi(argv[3]);

    cache.L1.SET = (cache.L1.SIZE/(cache.L1.BLOCKSIZE*cache.L1.ASSOC));



    //Initialize L1 Cache
    noOfTagEntriesL1 = cache.L1.SET*cache.L1.ASSOC;
    cache.L1.TAGS = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
    cache.L1.DIRTY = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
    cache.L1.VALID = (unsigned int*)malloc( (noOfTagEntriesL1*sizeof(unsigned int)) );
    cache.L1.LRUCounter = (int*)malloc( (noOfTagEntriesL1*sizeof(int)) );

    memset( cache.L1.TAGS, 0, (sizeof(cache.L1.TAGS[0])*noOfTagEntriesL1) );
    memset( cache.L1.DIRTY, 0, (sizeof(cache.L1.DIRTY[0])*noOfTagEntriesL1) );
    memset( cache.L1.VALID, 0, (sizeof(cache.L1.VALID[0])*noOfTagEntriesL1) );
    memset( cache.L1.LRUCounter, 0, (sizeof(cache.L1.LRUCounter[0])*noOfTagEntriesL1) );


    cache.L1.NUM_READ = 0;
    cache.L1.NUM_WRITE = 0;
    cache.L1.NUM_READ_MISS = 0;
    cache.L1.NUM_WRITE_MISS = 0;
    cache.L1.NUM_ACCESS = 0;
    cache.L1.NUM_WRITEBACK = 0;
    cache.L1.nextLevel = NULL;*/

    //Checking for L2 Cache Presence
    if( (unsigned int)atoi(argv[5]) == 0 )
    {
        cache.L2.SIZE = (unsigned int)atoi(argv[5]);
    }
    else
    {
        cache.L2.BLOCKSIZE = (unsigned int)atoi(argv[1]);
        cache.L2.SIZE = (unsigned int)atoi(argv[5]);
        cache.L2.ASSOC = (unsigned int)atoi(argv[6]);

        cache.L1.nextLevel = &cache.L2;
        cache.L2.nextLevel = NULL;

        cache.L2.SET = (cache.L2.SIZE/(cache.L2.BLOCKSIZE*cache.L2.ASSOC));


        noOfTagEntriesL2 = cache.L2.SET*cache.L2.ASSOC;
        cache.L2.TAGS = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
        cache.L2.DIRTY = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
        cache.L2.VALID = (unsigned int*)malloc( (noOfTagEntriesL2*sizeof(unsigned int)) );
        cache.L2.LRUCounter = (int*)malloc( (noOfTagEntriesL2*sizeof(int)) );


        memset( cache.L2.TAGS, 0, (sizeof(cache.L2.TAGS[0])*noOfTagEntriesL2) );
        memset( cache.L2.DIRTY, 0, (sizeof(cache.L2.DIRTY[0])*noOfTagEntriesL2) );
        memset( cache.L2.VALID, 0, (sizeof(cache.L2.VALID[0])*noOfTagEntriesL2) );
        memset( cache.L2.LRUCounter, 0, (sizeof(cache.L2.LRUCounter[0])*noOfTagEntriesL2) );


        cache.L2.NUM_READ = 0;
        cache.L2.NUM_WRITE = 0;
        cache.L2.NUM_READ_MISS = 0;
        cache.L2.NUM_WRITE_MISS = 0;
        cache.L2.NUM_ACCESS = 0;
        cache.L2.NUM_WRITEBACK = 0;

    }



    //Checking for Victim Cache Presence
    if( (unsigned int)atoi(argv[4]) == 0 )
    {
        cache.Victim.SIZE = (unsigned int)atoi(argv[4]);
    }
    else
    {
        cache.Victim.BLOCKSIZE = cache.L1.BLOCKSIZE;
        cache.Victim.SIZE = (unsigned int)atoi(argv[4]);
        cache.Victim.nextLevel =NULL;

        cache.Victim.SET = 1;
        cache.Victim.ASSOC = (cache.Victim.SIZE/cache.Victim.BLOCKSIZE);

        noOfTagEntriesVC = cache.Victim.SET*cache.Victim.ASSOC;
        cache.Victim.TAGS = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
        cache.Victim.DIRTY = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
        cache.Victim.VALID = (unsigned int*)malloc( (noOfTagEntriesVC*sizeof(unsigned int)) );
        cache.Victim.LRUCounter = (int*)malloc( (noOfTagEntriesVC*sizeof(int)) );

        memset( cache.Victim.TAGS, 0, (sizeof(cache.Victim.TAGS[0])*noOfTagEntriesVC) );
        memset( cache.Victim.DIRTY, 0, (sizeof(cache.Victim.DIRTY[0])*noOfTagEntriesVC) );
        memset( cache.Victim.VALID, 0, (sizeof(cache.Victim.VALID[0])*noOfTagEntriesVC) );
        memset( cache.Victim.LRUCounter, 0, (sizeof(cache.Victim.LRUCounter[0])*noOfTagEntriesVC) );


        cache.Victim.NUM_READ = 0;
        cache.Victim.NUM_WRITE = 0;
        cache.Victim.NUM_READ_MISS = 0;
        cache.Victim.NUM_WRITE_MISS = 0;
        cache.Victim.NUM_ACCESS = 0;
        cache.Victim.NUM_WRITEBACK = 0;
        cache.Victim.NUM_SWAP = 0;


    }

    cache.input();
    cache.output();




    return(0);
}



