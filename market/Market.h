#ifndef ORDERBOOK_MARKET_H
#define ORDERBOOK_MARKET_H

#include "OrderBook.h"

#include "proto/MarketAccess.grpc.pb.h"
#include "proto/MarketAccess.pb.h"

#include <thread>

class Market {
private:
    std::unordered_map<std::string, std::thread> productToOrderBookThreadMap_;
    GeneratorId * genId_;

public:
    std::unordered_map<std::string, OrderBook*> productToOrderBookMap_;

    explicit Market(GeneratorId * genID);
    ~Market();

    Market(Market&& other) = delete;
    Market& operator=(Market&&) = delete;

    void createNewOrderBook(const std::string& product_ID);
    void deleteOrderBook(const std::string& product_ID);

    inline OrderBook* getterOrderBookPointer(const std::string& productID) {return productToOrderBookMap_[productID];};
};

#endif //ORDERBOOK_MARKET_H
