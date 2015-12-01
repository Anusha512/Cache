#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

class Cache {
private:
    unsigned int BLOCK, SIZE, ASSOC, SET, TAG;
    unsigned int NUM_READ, NUM_READ_MISS, NUM_WRITE, NUM_WRITE_MISS, NUM_WRITEBACK, NUM_SWAP, NUM_ACCESS;
    double MISS_RATE, MISS_PENALTY, HIT_TIME, ACCESS_TIME;

public:
    unsigned int TAG_LOC, INDEX, TAG_ADD;
    unsigned int *LRUC, *TAGS, *DIRTY, *VALID;
    int block, index;

    Cache *nextLevel;

    void init(unsigned int block, unsigned int size, unsigned int assoc, unsigned int set, unsigned int tag);
    void transAddress(unsigned int address);
    void Hit(int index);
    void Miss();

    friend class CACHE;

};

class CACHE {
private:
    Cache L1,L2,Victim;
    unsigned int LRU_KEY, TAG_KEY, DIR_KEY;
    char *TRACE_FILE;

public:
    void init(unsigned int block, unsigned int size1, unsigned int assoc1, unsigned int sizev, unsigned int size2, unsigned int assoc2, char *trace);
    void input();
    void output();
    void readFromAddress(Cache &cache, unsigned int address, unsigned int vc);
    void writeToAddress(Cache &cache, unsigned int address, unsigned int vc);
    void readFromVictim(Cache &cache, unsigned int address, char rw);
};

#endif //CACHE_CACHE_H
