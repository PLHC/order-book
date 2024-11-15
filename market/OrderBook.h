#ifndef ORDERBOOK_ORDERBOOK_H
#define ORDERBOOK_ORDERBOOK_H


#include "order/Order.h"
#include "OrderLinkedList.h"
#include "CustomerRequestQueue/CustomerRequestQueue.h"
#include "GeneratorId.h"
#include "../database/DatabaseInterface.h"

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
    OrderLinkedList bids_{BUY};
    OrderLinkedList offers_{SELL};
    std::unordered_map<uint64_t, Order*> idToPointerMap_{};
    bool stopFlagOB_ {false};
    std::thread processingThread_{ std::thread( &OrderBook::processRequests, this) };
    DatabaseInterface* db_{ DatabaseInterface::getterDatabase() };

    orderExecution checkExecution(Order* orderToBeChecked);
    void performExecution(Order* & executingOrder);

public:
    GeneratorId* genId_{ GeneratorId::getInstance() };
    CustomerRequestQueue requestQueue_{};

    explicit OrderBook(std::string_view productID): productId_{productID} {}
    ~OrderBook();

    OrderBook(const OrderBook& other) = delete;
    OrderBook& operator=(OrderBook& other) = delete;

    // references to pointer so the original pointer can be updated, instead of passing it by value
    bool insertion(Order* &newOrder, communicate communicated);
    bool update(Order* updatedOrder, Order* &newOrder);
    void deletion(Order* deletedOrder, communicate communicated);
    std::string displayOrderBook(uint32_t nbOfOrdersToDisplay);

    [[nodiscard]] Order* getterPointerToOrderFromID( uint64_t boID );
    [[nodiscard]] const std::string& getterProductID() { return productId_; }
    void setterStopFlagToTrue() { stopFlagOB_ = true; }

    void processRequests();
};

#endif //ORDERBOOK_ORDERBOOK_H
