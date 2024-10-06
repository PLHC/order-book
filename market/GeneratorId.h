#ifndef ORDERBOOK_GENERATORID_H
#define ORDERBOOK_GENERATORID_H

#include <mutex>


class GeneratorId{
    uint64_t lastID_;
    std::mutex mtx_;

public:
    explicit GeneratorId(uint64_t lastUsedValue);
    uint64_t nextID();
};
#endif //ORDERBOOK_GENERATORID_H
