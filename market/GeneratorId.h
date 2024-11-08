#ifndef ORDERBOOK_GENERATORID_H
#define ORDERBOOK_GENERATORID_H

#include <mutex>

#include "../database/DatabaseInterface.h"

// Singleton implementation
class GeneratorId{
    static std::mutex generatorMtx_;
    static GeneratorId* instance_;
    DatabaseInterface* db_;
    int64_t lastID_;

    GeneratorId()
            : db_{ DatabaseInterface::getterDatabase() }
            , lastID_{ db_->getterLastRecordedIdWhenRestartingDatabase() }{}

public:
    GeneratorId (const GeneratorId&) = delete;
    GeneratorId& operator=(const GeneratorId&) = delete;

    ~GeneratorId() { delete db_; }
    static GeneratorId* getInstance(){
        std::unique_lock<std::mutex> genLock(generatorMtx_);
        if(!instance_){
            instance_ = new GeneratorId();
        }
        return instance_;
    }

    int64_t nextID();
};
#endif //ORDERBOOK_GENERATORID_H
