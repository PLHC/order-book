#ifndef ORDERBOOK_GENERATORID_H
#define ORDERBOOK_GENERATORID_H

#include <mutex>


class GeneratorID{
    uint64_t lastID_;
    std::mutex mtx_;
    std::condition_variable cv_;

public:
    explicit GeneratorID(uint64_t lastUsedValue);
    uint64_t nextID();
};
#endif //ORDERBOOK_GENERATORID_H
