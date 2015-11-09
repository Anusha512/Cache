#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

class Cache {
private:
    unsigned int BLOCKSIZE, SIZE, ASSOC, REPLACEMENT_POLICY, WRITE_POLICY;
    char *TRACE_FILE;
    unsigned int NUM_READ, NUM_READ_MISS, NUM_WRITE, NUM_WRITE_MISS, NUM_WRITEBACK, TOT_MEM_TRAFFIC;
    double MISS_RATE, MISS_PENALTY, HIT_TIME, ACCESS_TIME;


public:
    //Cache* nextLevel;
    unsigned int SET,TAG;
    unsigned int indexLocation, tagAddress;
    unsigned int* c_tagArray;
    unsigned int* dirty_bit;
    unsigned int* valid_in_bit;
    int* LRUCounter;
    int* count_set;

    void init(unsigned int block,unsigned int size,unsigned int assoc,unsigned int replacement,unsigned int write,char *trace);
    void input();
    void output();

    void readFromAddress();
    void writeToAddress();

    void HIT(unsigned int indexLocation, unsigned int tagLocation);
    void LRU(unsigned int indexLocation, unsigned int* tagLocation);
    void LFU(unsigned int indexLocation, unsigned int* tagLocation);
    void transAddress(unsigned int address);

};

#endif //CACHE_CACHE_H
