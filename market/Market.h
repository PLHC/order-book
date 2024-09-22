#ifndef ORDERBOOK_MARKET_H
#define ORDERBOOK_MARKET_H

#include "OrderBook.h"

#include "proto/MarketAccess.grpc.pb.h"
#include "proto/MarketAccess.pb.h"

#include <thread>

class Market {
private:
    std::unordered_map<std::string, std::thread> ProductToOrderBookThreadMap;
    GeneratorID * genID_;

public:
    std::unordered_map<std::string, OrderBook*> ProductToOrderBookMap;

    explicit Market(GeneratorID * genID);
    ~Market();

    Market(Market&& other) = delete;
    Market& operator=(Market&&) = delete;

    void createNewOrderBook(const std::string& product_ID);
    void deleteOrderBook(const std::string& product_ID);

    inline OrderBook* getterOrderBookPointer(const std::string& productID) {return ProductToOrderBookMap[productID];};
};

#endif //ORDERBOOK_MARKET_H
