#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H


#include "Order.h"
#include "OrderLinkedList.h"
#include "CustomerRequestQueue/CustomerRequestQueue.h"
#include "GeneratorId.h"

#include <atomic>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>

enum orderExecution { FULL_EXECUTION, PARTIAL_EXECUTION, NO_EXECUTION };

class OrderBook {
private:
    std::string productId_;
    OrderLinkedList bids_;
    OrderLinkedList offers_;
    std::unordered_map<uint64_t, Order*> idToPointerMap_;
    std::atomic<bool> stopFlag_;
    uint64_t logicalClock_;


    orderExecution checkExecution(Order* orderToBeChecked);
    void performExecution(Order* executingOrder);

public:
    GeneratorId *genId_;
    CustomerRequestQueue requestQueue_;

    OrderBook(std::string productID, GeneratorId * genID);

    OrderBook(OrderBook&& other) = delete;
    OrderBook& operator=(const OrderBook&& other) = delete;

    void insertion(Order* newOrder);
    void update(Order* updatedOrder,
                Order* newOrder);
    void deletion(Order* deletedOrder);
    std::string displayOrderBook();
    [[nodiscard]] inline Order* getterPointerToOrderFromID(uint64_t boID) {return idToPointerMap_[boID];};
    [[nodiscard]] inline std::string getterProductID() {return productId_;};
    void processRequests();
    inline void setterStopFlagToTrue() {stopFlag_.store(true);};
};


#endif //ORDERBOOK_ORDERBOOK_H
