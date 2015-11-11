#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

class Cache {
private:
    unsigned int BLOCKSIZE, SIZE, ASSOC, REPLACEMENT_POLICY, WRITE_POLICY;
    char *TRACE_FILE;
    unsigned int NUM_READ, NUM_READ_MISS, NUM_WRITE, NUM_WRITE_MISS, NUM_WRITEBACK, TOT_MEM_TRAFFIC;
    double MISS_RATE, MISS_PENALTY, HIT_TIME, ACCESS_TIME;

public:
    unsigned int SET,TAG,INDEX,TAG_LOC, TAG_ADD;
    unsigned int *TAGS, *DIRTY, *VALID;
    int *NUM_TAG, *NUM_SET;

    void init(unsigned int block,unsigned int size,unsigned int assoc,unsigned int replacement,unsigned int write,char *trace);
    void input();
    void output();

    void readFromAddress();
    void writeToAddress();
    void transAddress(unsigned int address);

    void HIT(int index);
    void LRU();
    void LFU();
};

#endif //CACHE_CACHE_H
