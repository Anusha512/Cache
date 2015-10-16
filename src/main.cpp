#include <iostream>
#include "cache.h"
using namespace std;

Cache cache;
int main(int argc, char **argv) {

    if(argc<6) {
        printf("\nUsage: ./sim_cache [BLOCKSIZE] [L1_SIZE] [L1_ASSOC]"
        " [L1_REPLACEMENT_POLICY] [L1_WRITE_POLICY] [trace_file] > [output_file]\n");
        return 0;
    }

    freopen(argv[6],"r",stdin);

    unsigned int block = (unsigned)atoi(argv[1]);
    unsigned int size = (unsigned)atoi(argv[2]);
    unsigned int assoc = (unsigned)atoi(argv[3]);
    unsigned int replacement = (unsigned)atoi(argv[4]);
    unsigned int write = (unsigned)atoi(argv[5]);

    char *trace = argv[6];
    int i,len=(int)strlen(trace);
    for(i=len;i>=0;i--)
        if(trace[i]=='/')
            break;
    trace=trace+i+1;

    cache.init(block, size, assoc, replacement, write, trace);
    cache.input();
    cache.getMissRate();
    cache.getAccessTime();
    cache.output();
    return 0;
}


