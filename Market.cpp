//
// Created by Paul  on 04/07/2024.
//

#include "Market.h"
#include "CustomerRequestQueue.h"

Market::Market():
        ProductToOrderBookMap(),
        ProductToCustomerRequestQueueMap(),
        lastID(0) {} // replace last ID by an ID in database; needs a lock on it

Market::~Market(){
    for(auto & [product, OrderBookPointer] : ProductToOrderBookMap){
        delete OrderBookPointer;
    }
    for(auto & [product, QueuePointer] : ProductToCustomerRequestQueueMap){
        delete QueuePointer;
    }
}

void Market::addDeleteOrderToQueue(int32_t userID,
                         const std::string &product_ID,
                         uint64_t boID) {
    //check if product and order exist first or throw error
    auto Q = ProductToCustomerRequestQueueMap[product_ID];
    std::unique_lock<std::mutex> lock(Q->queueMutex_);
    Q->requestQueue_.emplace(deletionCR,
                                  userID,
                                  product_ID,
                                  boID);
    Q->queueConditionVariable_.notify_one();
}

void Market::addInsertOrderToQueue(int32_t userID,
                         double price,
                         double volume,
                         const std::string& product_ID,
                         orderDirection buyOrSell,
                         orderType boType) {
    //check if product exists first or throw error
    auto Q = ProductToCustomerRequestQueueMap[product_ID];
    std::unique_lock<std::mutex> lock(Q->queueMutex_);
    Q->requestQueue_.emplace(insertionCR,
                             userID,
                             product_ID,
                             nextID(),
                             price,
                             volume,
                             buyOrSell,
                             boType);
    Q->queueConditionVariable_.notify_one();
}

void Market::addUpdateOrderToQueue(int32_t userID,
                         int32_t price,
                         uint32_t volume,
                         const std::string &product_ID,
                         orderDirection buyOrSell,
                         orderType boType,
                         uint64_t updatedOrderID) {
    //check if product and order exist first or throw error
    auto Q = ProductToCustomerRequestQueueMap[product_ID];
    std::unique_lock<std::mutex> lock(Q->queueMutex_);
    Q->requestQueue_.emplace(updateCR,
                             userID,
                             product_ID,
                             nextID(),
                             price,
                             volume,
                             buyOrSell,
                             boType,
                             updatedOrderID);
    Q->queueConditionVariable_.notify_one();
}

void Market::deleteOrderBook(const std::string &product_ID) {
    // acquire lock for it first
    // check if orderbook existing or throw error
    delete ProductToOrderBookMap[product_ID];
    ProductToOrderBookMap.erase(product_ID);
    delete ProductToCustomerRequestQueueMap[product_ID];
    ProductToCustomerRequestQueueMap.erase(product_ID);
}