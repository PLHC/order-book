#ifndef ORDERBOOK_MARKET_H
#define ORDERBOOK_MARKET_H

#include "Order.h"
#include "OrderLinkedList.h"
#include "OrderBook.h"
#include <thread>
#include "proto/MarketAccess.grpc.pb.h"
#include "proto/MarketAccess.pb.h"

class Market {
private:
    std::unordered_map<std::string, std::thread> ProductToOrderBookThreadMap;
    uint64_t lastID;

public:
    std::unordered_map<std::string, OrderBook*> ProductToOrderBookMap;

    Market();
    ~Market();

    Market(Market&& other) = delete;
    Market& operator=(Market&&) = delete;

    inline uint64_t nextID(){return ++lastID;};
    void createNewOrderBook(const std::string& product_ID);
    void deleteOrderBook(const std::string& product_ID);

    inline OrderBook* getterOrderBookPointer(const std::string& productID) {return ProductToOrderBookMap[productID];};
};

#endif //ORDERBOOK_MARKET_H
