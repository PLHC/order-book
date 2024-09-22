#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H


#include "Order.h"
#include "OrderLinkedList.h"
#include "CustomerRequestQueue/CustomerRequestQueue.h"
#include "GeneratorID.h"

#include <atomic>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

enum orderExecution {fullExecution, partialExecution, noExecution};

class OrderBook {
private:
    std::string productID_;
    OrderLinkedList bids_;
    OrderLinkedList offers_;
    std::unordered_map<uint64_t, Order*> IDtoPointerMap;
    std::atomic<bool> stopFlag;


    orderExecution checkExecution(Order* orderToBeChecked);
    void performExecution(Order* executingOrder);

public:
    GeneratorID *genID_;
    CustomerRequestQueue requestQueue_;

    OrderBook(std::string productID, GeneratorID * genID);

    OrderBook(OrderBook&& other) = delete;
    OrderBook& operator=(const OrderBook&& other) = delete;

    void insertion(Order* newOrder);
    void update(Order* updatedOrder,
                Order* newOrder);
    void deletion(Order* deletedOrder);
    std::string displayOrderBook();
    [[nodiscard]] inline Order* getterPointerToOrderFromID(uint64_t boID) {return IDtoPointerMap[boID];};
    [[nodiscard]] inline std::string getterProductID() {return productID_;};
    void processRequests();
    inline void setterStopFlagToTrue() {stopFlag.store(true);};
};


#endif //ORDERBOOK_ORDERBOOK_H
