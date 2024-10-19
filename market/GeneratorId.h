#ifndef ORDERBOOK_GENERATORID_H
#define ORDERBOOK_GENERATORID_H

#include <mutex>

// Singleton implementation
class GeneratorId{
    uint64_t lastID_;
    static std::mutex mtx_;
    static GeneratorId* instance_;

    explicit GeneratorId(uint64_t lastUsedValue): lastID_(lastUsedValue){}

public:
    GeneratorId (const GeneratorId&) = delete;
    GeneratorId& operator=(const GeneratorId&) = delete;

    static GeneratorId* getInstance(){
        std::unique_lock<std::mutex> genLock(mtx_);
        if(!instance_){
            instance_ = new GeneratorId(0);
        }
        return instance_;
    }

    uint64_t nextID();
};
#endif //ORDERBOOK_GENERATORID_H
