//
// Created by Paul  on 04/07/2024.
//

#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H

#include <unordered_map>
#include "Order.h"
#include "OrderLinkedList.h"

enum orderExecution {fullExecution, partialExecution, noExecution};

class OrderBook {
private:
    OrderLinkedList bids_;
    OrderLinkedList offers_;
    std::unordered_map<uint64_t, Order*> IDtoPointerMap;

public:
    OrderBook();

    OrderBook(OrderBook&& other) = delete;
    OrderBook& operator=(const OrderBook&& other) = delete;

    [[nodiscard]] inline Order* getterPointerToOrderFromID(uint64_t& boID) {return IDtoPointerMap[boID];};

    orderExecution checkExecution(Order* orderToBeChecked);
    void performExecution(Order* executingOrder);
    void insertion(Order* newOrder);
    void update(Order* updatedOrder,
                Order* newOrder);
    void deletion(Order* deletedOrder);
    void displayOrderBook();
};


#endif //ORDERBOOK_ORDERBOOK_H
