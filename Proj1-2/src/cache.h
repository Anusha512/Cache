#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H

class Cache {
public:
    unsigned int SIZE, ASSOC, SET, TAG;
    unsigned int NUM_READ, NUM_READ_MISS, NUM_WRITE, NUM_WRITE_MISS, NUM_WRITEBACK, NUM_SWAP, NUM_ACCESS;
    double MISS_RATE, MISS_PENALTY, HIT_TIME, ACCESS_TIME;

public:
    //unsigned int SET,TAG,INDEX,TAG_LOC, TAG_ADD;
    unsigned int *TAGS, *DIRTY, *VALID;
    int *NUM_TAG, *NUM_SET;

    unsigned int c_numOfSets;


    int* LRUCounter;
    //unsigned int memoryAccessCounter;


    Cache *nextLevel;

    void init(unsigned int size, unsigned int assoc, unsigned int set, unsigned int tag);

    void transAddress(unsigned int address);

    void HIT(int index);
    void LRU();
    void LFU();



};

class CACHE {
public:
    Cache L1,L2,Victim;
    unsigned int BLOCKSIZE;
    char *TRACE_FILE;
    unsigned int key, tagKey, dirtyKey;
    void init(unsigned int block, unsigned int size1, unsigned int assoc1, unsigned int sizev, unsigned int size2, unsigned int assoc2, char *trace);
    void input();
    void output();

    void extractAddressParams(unsigned int addressInInt, Cache l1Cache, unsigned int* indexLocation, unsigned int* tagAddress);
    int readFromAddress(Cache &cache_ds, unsigned int addressInInt, unsigned int vc_size);
    int writeToAddress(Cache &cache_ds, unsigned int addressInInt, unsigned int vc_size);
    int readFromVictimCache(unsigned int addressInInt, Cache &cache_ds, unsigned int tagLocationCache, char rw);

};




void LRUForHit(Cache &l1Cache, unsigned int indexLocation, unsigned int tagLocation);
void LRUForMiss(Cache &l1Cache, unsigned int indexLocation, unsigned int* tagLocation);

void LRUForHitVC(Cache &cache_ds, unsigned int indexLocation);
void LRUForMissVC(Cache &cache_ds, unsigned int* tagLocation);

#endif //CACHE_CACHE_H
