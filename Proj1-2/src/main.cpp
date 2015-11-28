#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "cache.h"

CACHE cache;
int main(int argc, char **argv) {
    if(argc<7) {
        printf("\nUsage: ./sim_cache [BLOCKSIZE] [L1_SIZE] [L1_ASSOC] [Victim_Cache_SIZE]"
                       " [L2_SIZE] [L2_ASSOC] [trace_file] > [output_file]\n");
        return 0;
    }

    if(!freopen(argv[7],"r",stdin))
        return 0;

    unsigned int block = (unsigned)atoi(argv[1]);
    unsigned int size1 = (unsigned)atoi(argv[2]);
    unsigned int assoc1 = (unsigned)atoi(argv[3]);
    unsigned int sizev = (unsigned)atoi(argv[4]);
    unsigned int size2 = (unsigned)atoi(argv[5]);
    unsigned int assoc2 = (unsigned)atoi(argv[6]);

    char *trace = argv[7];
    int i,len=(int)strlen(trace);
    for(i=len;i>=0;i--)
        if(trace[i]=='/')
            break;
    trace=trace+i+1;

    cache.init(block, size1, assoc1, sizev, size2, assoc2, trace);
    cache.input();
    cache.output();
    return 0;
}