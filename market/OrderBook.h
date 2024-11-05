#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H


#include "order/Order.h"
#include "OrderLinkedList.h"
#include "CustomerRequestQueue/CustomerRequestQueue.h"
#include "GeneratorId.h"
#include "../lock_free_queue/LockFreeQueue.h"
#include "../server_and_client_zero_mq/Message/Message.h"

#include <atomic>
#include <unordered_map>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <thread>
#include <vector>



enum orderExecution { FULL_EXECUTION, PARTIAL_EXECUTION, NO_EXECUTION };
enum communicate {COMMUNICATED, NON_COMMUNICATED};

class OrderBook {
private:
    const std::string productId_;
    OrderLinkedList bids_;
    OrderLinkedList offers_;
    std::unordered_map<uint64_t, Order*> idToPointerMap_;
    bool stopFlagOB_;
    std::thread processingThread_;
    LockFreeQueue<Message*>* messageQueue_;

    orderExecution checkExecution(Order* orderToBeChecked);
    void performExecution(Order* & executingOrder);

public:
    GeneratorId* genId_;
    CustomerRequestQueue requestQueue_;

    OrderBook(std::string_view productID, LockFreeQueue<Message*>* messageQueuePtr);
    ~OrderBook();

    OrderBook(const OrderBook& other) = delete;
    OrderBook& operator=(OrderBook& other) = delete;


    bool insertion(Order* &newOrder, communicate communicated); // reference to pointer so the original pointer can be updated, instead of passing it by value
    bool update(Order* updatedOrder, Order* &newOrder);
    void deletion(Order* deletedOrder, communicate communicated);
    std::string displayOrderBook(uint32_t nbOfOrdersToDisplay);

    [[nodiscard]] Order* getterPointerToOrderFromID(uint64_t boID);
    [[nodiscard]] const std::string& getterProductID() { return productId_; }
    void setterStopFlagToTrue() { stopFlagOB_ = true; }

    void processRequests();
};

#endif //ORDERBOOK_ORDERBOOK_H
