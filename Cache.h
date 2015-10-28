//
// Created by Shintaku on 10/26/15.
//

#ifndef CACHE_CACHE_H
#define CACHE_CACHE_H


class Cache {
//private:
public:
    unsigned int BLOCKSIZE, SIZE, ASSOC, REPLACEMENT_POLICY, WRITE_POLICY;
    char *TRACE_FILE;

public:
    Cache* nextLevel;
    unsigned int SET;

    unsigned int* c_tagArray;
    unsigned int* dirty_bit;
    unsigned int* valid_in_bit;
    int* LRUCounter;
    int* count_set;

    unsigned int readCounter;
    unsigned int readMissCounter;
    unsigned int writeCounter;
    unsigned int writeMissCounter;
    unsigned int memoryAccessCounter;
    unsigned int noOfWritebacks;

    //bool readFromAddress(unsigned int add);
    //bool writeToAddress(unsigned int add);

    void input(unsigned int block,unsigned int size,unsigned int assoc,unsigned int replacement,unsigned int write,char *trace);
    void output(double miss,double access);

};

int readFromAddress(Cache* cache, unsigned int indexLocation, unsigned int tagAddress);
int writeToAddress(Cache* cache, unsigned int indexLocation, unsigned int tagAddress);
void LRUForHit(Cache* cache, unsigned int indexLocation, unsigned int tagLocation);
void LRUForMiss(Cache* cache, unsigned int indexLocation, unsigned int* tagLocation);
void LeastFrequentForMiss(Cache* cache, unsigned int indexLocation, unsigned int* tagLocation);

#endif //CACHE_CACHE_H
