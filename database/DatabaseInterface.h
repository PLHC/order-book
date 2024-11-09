#ifndef ORDERBOOK_DATABASEINTERFACE_H
#define ORDERBOOK_DATABASEINTERFACE_H

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/types.hpp>
#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "../lock_free_queue/LockFreeQueue.h"
#include "../market/order/Order.h"

enum actions {UPDATE, EXECUTION, DELETION, INSERTION};

// Singleton implementation
class DatabaseInterface {
    static std::mutex dbMtx_;
    static DatabaseInterface* dbPtr_;
    const mongocxx::instance instance_{};
    const mongocxx::client client_{ mongocxx::uri{"mongodb://127.0.0.1:27017"} };
    const mongocxx::database db_{ client_["orderbookProjectDatabase"] };
    mongocxx::collection collection_{ db_["orders"] };

    LockFreeQueue<bsoncxx::builder::basic::document*> lfq_{};
    std::atomic<bool> stopFlag_{ false };
    std::thread processingThread_{ std::thread( &DatabaseInterface::process, this)};

    const int64_t lastRecordedIdWhenRestartingDatabase_{ extractLastID() };
    mongocxx::bulk_write bulk_{ collection_.create_bulk_write() };
    int32_t bulkSize_{0};

    DatabaseInterface() = default;
    void process();
    int64_t extractLastID();

public:
    DatabaseInterface(DatabaseInterface* other) = delete;
    DatabaseInterface* operator=(DatabaseInterface* other) = delete;

    ~DatabaseInterface();

    static DatabaseInterface* getterDatabase(){
        std::unique_lock<std::mutex> dbLock(dbMtx_);
        if(!dbPtr_){
            dbPtr_ = new DatabaseInterface();
        }
        return dbPtr_;
    }
    void pushNewDbInputOnQueue(actions action,
                               const Order &order,
                               const double executedVolumeInHundredth = 0,
                               const double executedPriceInCents = 0);
    int64_t getterLastRecordedIdWhenRestartingDatabase() const { return lastRecordedIdWhenRestartingDatabase_; }
};

#endif //ORDERBOOK_DATABASEINTERFACE_H
