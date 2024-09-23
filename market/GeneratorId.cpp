#include "GeneratorId.h"

GeneratorId::GeneratorId(uint64_t lastUsedValue) : lastID_(lastUsedValue){}

uint64_t GeneratorId::nextID() {
    uint64_t val;

    std::unique_lock<std::mutex> generatorLock(mtx_);
    cv_.wait(generatorLock, [](){return true;});

    lastID_++;
    val = lastID_;

    generatorLock.unlock();
    cv_.notify_all();

    return val;
};
