#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H


#include "order/Order.h"
#include "OrderLinkedList.h"
#include "CustomerRequestQueue/CustomerRequestQueue.h"
#include "GeneratorId.h"

#include <atomic>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <thread>
#include <vector>


enum orderExecution { FULL_EXECUTION, PARTIAL_EXECUTION, NO_EXECUTION };

class OrderBook {
private:
    std::string productId_;
    OrderLinkedList bids_;
    OrderLinkedList offers_;
    std::unordered_map<uint64_t, Order*> idToPointerMap_;
    bool stopFlagOB_;
    std::thread processingThread_;

    orderExecution checkExecution(Order* orderToBeChecked);
    void performExecution(Order* & executingOrder);

public:
    GeneratorId* genId_;
    CustomerRequestQueue requestQueue_;

    explicit OrderBook(std::string productID);
    ~OrderBook();

    OrderBook(OrderBook& other) = delete;
    OrderBook(OrderBook&& other) = delete;
    OrderBook& operator=(const OrderBook&& other) = delete;

    bool insertion(Order* &newOrder);
    bool update(Order* updatedOrder, Order* &newOrder);
    void deletion(Order* deletedOrder);
    std::string displayOrderBook(uint32_t nbOfOrdersToDisplay);

    [[nodiscard]] Order* getterPointerToOrderFromID(uint64_t boID);
    [[nodiscard]] std::string getterProductID() { return productId_; };
    void setterStopFlagToTrue() { stopFlagOB_ = true;};

    void processRequests();
};


#endif //ORDERBOOK_ORDERBOOK_H
