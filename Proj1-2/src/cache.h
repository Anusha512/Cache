#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

class Cache {
private:
    unsigned int SIZE, ASSOC, SET, TAG;
    unsigned int NUM_READ, NUM_READ_MISS, NUM_WRITE, NUM_WRITE_MISS, NUM_WRITEBACK, NUM_SWAP, NUM_ACCESS;
    double MISS_RATE, MISS_PENALTY, HIT_TIME, ACCESS_TIME;

public:
    //unsigned int SET,TAG,INDEX,TAG_LOC, TAG_ADD;
    unsigned int TAG_LOC, INDEX;
    unsigned int *LRUC, *TAGS, *DIRTY, *VALID;


    Cache *nextLevel;

    void init(unsigned int size, unsigned int assoc, unsigned int set, unsigned int tag);
    void Hit(int index);
    void Miss();

    friend class CACHE;

};

class CACHE {
private:
    Cache L1,L2,Victim;
    unsigned int BLOCKSIZE, LRU_KEY, TAG_KEY, DIR_KEY;
    char *TRACE_FILE;

public:
    void init(unsigned int block, unsigned int size1, unsigned int assoc1, unsigned int sizev, unsigned int size2, unsigned int assoc2, char *trace);
    void input();
    void output();
    void transAddress(Cache &cache_ds, unsigned int address);
    void extractAddressParams(unsigned int address, Cache l1Cache, unsigned int* indexLocation, unsigned int* tagAddress);
    void readFromAddress(Cache &cache_ds, unsigned int address, unsigned int vc_size);
    void writeToAddress(Cache &cache_ds, unsigned int address, unsigned int vc_size);
    void readFromVictim(Cache &cache_ds, unsigned int address, unsigned int tagLocationCache, char rw);




};






#endif //CACHE_CACHE_H
