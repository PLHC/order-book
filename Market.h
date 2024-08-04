//
// Created by Paul  on 04/07/2024.
//

#ifndef ORDERBOOK_MARKET_H
#define ORDERBOOK_MARKET_H

#include "Order.h"
#include "OrderLinkedList.h"
#include "OrderBook.h"
#include "CustomerRequestQueue.h"
#include <thread>

class Market {
private:
    std::unordered_map<std::string, OrderBook*> ProductToOrderBookMap;
    std::unordered_map<std::string, CustomerRequestQueue*> ProductToCustomerRequestQueueMap;
    std::unordered_map<std::string, std::thread> ProductToOrderBookThreadMap;
    uint64_t lastID;

public:
    Market();
    ~Market();

    Market(Market&& other) = delete;
    Market& operator=(Market&&) = delete;

    inline uint64_t nextID(){return ++lastID;};
    void createNewOrderBook(const std::string& product_ID);
    void deleteOrderBook(const std::string& product_ID);
    void addDeleteOrderToQueue(int32_t userID,
                     const std::string& product,
                     uint64_t boID);
    void addInsertOrderToQueue(int32_t userID,
                     double price,
                     double volume,
                     const std::string& product_ID,
                     orderDirection buyOrSell,
                     orderType boType);
    void addUpdateOrderToQueue(int32_t userID,
                     int32_t price,
                     uint32_t volume,
                     const std::string& product_ID,
                     orderDirection buyOrSell,
                     orderType boType,
                     uint64_t updatedOrderID);
    inline OrderBook* getterOrderBookPointer(const std::string& productID) {return ProductToOrderBookMap[productID];};
    inline CustomerRequestQueue* getterCustomerRequestQueue(const std::string& productID) {
        return ProductToCustomerRequestQueueMap[productID];
    };
};


#endif //ORDERBOOK_MARKET_H
