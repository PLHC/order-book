//
// Created by Paul  on 04/07/2024.
//

#ifndef ORDERBOOK_MARKET_H
#define ORDERBOOK_MARKET_H

#include "Order.h"
#include "OrderLinkedList.h"
#include "OrderBook.h"

class Market {
private:
    std::unordered_map<std::string, OrderBook*> ProductToOrderBookMap;
    uint32_t lastID;

public:
    Market();
    ~Market();

    Market(Market&& other) = delete;
    Market& operator=(Market&&) = delete;

    inline uint32_t nextID(){return ++lastID;};
    inline void createNewOrderBook(const std::string& product_ID) {
        ProductToOrderBookMap[product_ID] = new OrderBook();
    }
    void deleteOrderBook(const std::string& product_ID);
    void deleteOrder(const std::string& product,
                     uint32_t boID);
    void insertOrder(int32_t userID,
                     int32_t price,
                     uint32_t volume,
                     const std::string& product_ID,
                     orderDirection buyOrSell,
                     orderType boType);
    void updateOrder(int32_t userID,
                     int32_t price,
                     uint32_t volume,
                     const std::string& product_ID,
                     orderDirection buyOrSell,
                     orderType boType,
                     uint32_t updatedOrderID);
    inline OrderBook* getterOrderBookPointer(const std::string& productID) {return ProductToOrderBookMap[productID];};
};


#endif //ORDERBOOK_MARKET_H
