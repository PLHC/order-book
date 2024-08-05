//
// Created by Paul  on 30/07/2024.
//

#ifndef ORDERBOOK_CUSTOMERREQUESTQUEUE_H
#define ORDERBOOK_CUSTOMERREQUESTQUEUE_H

#include <cstdint>
#include <utility>
#include <string>
#include "Order.h"
#include <mutex>
#include <queue>

enum customerRequestType {deletionCR, insertionCR, updateCR, displayOrderBookCR};

class Request{
private:
    customerRequestType nodeType_;
    int32_t userID_{};
    std::string product_ID_;
    uint64_t boID_{};
    double price_{};
    double volume_{};
    orderDirection buyOrSell_;
    orderType boType_;
    uint64_t updatedOrderID_{};

public:
    // display_orderbook constructor
    explicit Request(customerRequestType nodeType);

    // delete_node constructor
    Request(customerRequestType nodeType,
            int32_t userID,
            std::string product_ID,
            uint64_t boID);

    // insertion_node constructor
    Request(customerRequestType nodeType,
            int32_t userID,
            std::string product_ID,
            uint64_t boID,
            double price,
            double volume,
            orderDirection buyOrSell,
            orderType boType);

    // update_node constructor
    Request(customerRequestType nodeType,
            int32_t userID,
            std::string product_ID,
            uint64_t boID,
            double price,
            double volume,
            orderDirection buyOrSell,
            orderType boType,
            uint64_t updatedOrderID);

    [[nodiscard]] inline customerRequestType getterNodeType() const{return nodeType_;};
    [[nodiscard]] inline int32_t getterUserID() const{return userID_;};
    [[nodiscard]] inline std::string getterProductID() const{return product_ID_;};
    [[nodiscard]] inline uint64_t getterBoID() const {return boID_;};
    [[nodiscard]] inline double getterPrice() const{return price_;};
    [[nodiscard]] inline double getterVolume() const{return volume_;};
    [[nodiscard]] inline orderDirection getterOrderDirection() const{return buyOrSell_;};
    [[nodiscard]] inline orderType getterOrderType() const{return boType_;};
    [[nodiscard]] inline uint64_t getterUpdatedOrderID() const{return updatedOrderID_;};
};


class CustomerRequestQueue {
public:
    std::mutex queueMutex_;
    std::condition_variable queueConditionVariable_;
    std::queue<Request> CRQueue_;
    CustomerRequestQueue();

    CustomerRequestQueue(CustomerRequestQueue&& other) = delete;
    CustomerRequestQueue& operator=(const CustomerRequestQueue&& other) = delete;
};

#endif //ORDERBOOK_CUSTOMERREQUESTQUEUE_H
