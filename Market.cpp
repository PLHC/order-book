//
// Created by Paul  on 04/07/2024.
//

#include "Market.h"
#include <iostream>


Market::Market():
        ProductToOrderBookMap(),
        ProductToCustomerRequestQueueMap(),
        ProductToOrderBookThreadMap(),
        // replace last ID by an ID in database; needs a lock on it
        lastID(0) {
    std::cout<<"in M constructor"<<std::endl;
}

Market::~Market(){
    std::cout<<"in market destructor"<<std::endl;
    for(const auto & [product, orderBookPointer] : ProductToOrderBookMap){
        orderBookPointer->setterStopFlagToTrue();
        ProductToCustomerRequestQueueMap[product]->queueConditionVariable_.notify_all();
    }
    for(auto & [product, orderBookThread]: ProductToOrderBookThreadMap){
        if(orderBookThread.joinable()) orderBookThread.join();
    }
    for(auto & [product, orderBookPointer] : ProductToOrderBookMap){
        delete orderBookPointer;
    }
    for(auto & [product, queuePointer] : ProductToCustomerRequestQueueMap){
        delete queuePointer;
    }
}


void Market::createNewOrderBook(const std::string& product_ID) {
    auto pointerToQueue = new CustomerRequestQueue();
    ProductToCustomerRequestQueueMap[product_ID] = pointerToQueue;
    auto pointerToOrderBook = new OrderBook(pointerToQueue);
    ProductToOrderBookMap[product_ID] = pointerToOrderBook;
    ProductToOrderBookThreadMap[product_ID] = std::thread(&OrderBook::listenToRequests, pointerToOrderBook);
}

void Market::deleteOrderBook(const std::string &product_ID) {
    // acquire lock for it first
    // check if orderbook existing or throw error
    delete ProductToOrderBookMap[product_ID];
    ProductToOrderBookMap.erase(product_ID);
    delete ProductToCustomerRequestQueueMap[product_ID];
    ProductToCustomerRequestQueueMap.erase(product_ID);
}

void Market::addDeleteOrderToQueue(int32_t userID,
                         const std::string &product_ID,
                         uint64_t boID) {
    //check if product and order exist first or throw error
    auto Q = ProductToCustomerRequestQueueMap[product_ID];
    std::lock_guard<std::mutex> lock(Q->queueMutex_);
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
    std::cout<<"market inserts"<<std::endl;
    auto Q = ProductToCustomerRequestQueueMap[product_ID];
    std::lock_guard<std::mutex> lock(Q->queueMutex_);
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
    std::lock_guard<std::mutex> lock(Q->queueMutex_);
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

