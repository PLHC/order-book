#include "DatabaseInterface.h"

#include <chrono>


std::mutex DatabaseInterface::dbMtx_ {};
DatabaseInterface* DatabaseInterface::dbPtr_ { nullptr };

DatabaseInterface::DatabaseInterface()
        : instance_{}
        , client_{mongocxx::uri{"mongodb://127.0.0.1:27017"}}
        , db_{ client_["orderbookProjectDatabase"] }
        , collection_ { db_["orders"] }
        , lfq_{}
        , stopFlag_{ false }
        , processingThread_( std::thread( &DatabaseInterface::process, this))
        , lastRecordedIdWhenRestartingDatabase_{ extractLastID() }
        , bulk_ { collection_.create_bulk_write() }
        , bulkSize_ {0} {}

DatabaseInterface::~DatabaseInterface(){
    std::cout<<"in DBInterface destructor\n";
    stopFlag_.store(true);
    if(processingThread_.joinable()){
        processingThread_.join();
    }
    if(bulkSize_){
        bulk_.execute();
    }
    std::cout<<"end DBInterface destructor\n";
}

void DatabaseInterface::process(){
    while(!stopFlag_.load() || lfq_.getterSize()){
        auto docPtr = lfq_.pop();
        if(docPtr.has_value()) {
            bulk_.append(std::move(mongocxx::model::insert_one { ( docPtr.value())->view() }));
            bulkSize_++;
            delete docPtr.value();

        }
        if(bulkSize_>500) { // insertion in DB for 500 orders between 3500 and 7500 microseconds
//            std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
            bulk_.execute();
//            std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
//            std::cout << "emptying vector in DB = "
//                        << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count()
//                        << "[µs]\n";
            bulk_ = collection_.create_bulk_write();
            bulkSize_ = 0;
        }
    }
}

int64_t DatabaseInterface::extractLastID() {
    using bsoncxx::builder::basic::make_document;
    using bsoncxx::builder::basic::kvp;

    mongocxx::options::find opts{};
    opts.sort(make_document(kvp("order_ID", -1)));
    opts.limit(1);

    auto max_doc = collection_.find_one({}, opts);
    auto lastID = (*max_doc)["order_ID"].get_int64();
    std::cout<<"max current ID: "<<lastID<<std::endl;
    return lastID;
}

void DatabaseInterface::pushNewDbInputOnQueue(actions action,
                                              const Order &order,
                                              const double executedVolumeInHundredth,
                                              const double executedPriceInCents) {
    using bsoncxx::builder::basic::kvp;
    auto doc = new bsoncxx::builder::basic::document();

    switch(action){
        case EXECUTION:
            doc->append(kvp("event", "execution"));
            break;
        case UPDATE:
            doc->append(kvp("event", "update"));
            break;
        case INSERTION:
            doc->append(kvp("event", "insertion"));
            break;
        case DELETION:
            doc->append(kvp("event", "deletion"));
            break;
    }

    doc->append(kvp("time", bsoncxx::types::b_date(std::chrono::system_clock::now())),
                kvp("order_ID", order.getterBoID()),
                kvp("user_ID", order.getterUserID()),
                kvp("product", order.getterProductID()),
                kvp("direction", order.getterOrderDirection()==BUY? "buy": "sell"),
                kvp("volume", order.getterVolume()),
                kvp("price", order.getterPrice()),
                kvp("version", order.getterVersion()),
                kvp("order_type", order.getterOrderType()==GOOD_TIL_CANCELLED? "GTC": "FOK")
                );

    if(executedVolumeInHundredth > 0){
        doc->append(kvp("executed_volume", executedVolumeInHundredth/100.0),
                    kvp("executed_price", executedPriceInCents/100.0));
    }

    lfq_.push(doc);
}
