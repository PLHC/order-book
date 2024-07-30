//
// Created by Paul  on 30/07/2024.
//

#ifndef ORDERBOOK_CUSTOMERREQUESTQUEUE_H
#define ORDERBOOK_CUSTOMERREQUESTQUEUE_H

#include <cstdint>
#include <utility>
#include <string>
#include "order.h"

enum customerRequestType {dummy, basic, deletion, insertion, update};

class DummyRequestNode{
    DummyRequestNode * nextNode_;
    DummyRequestNode * prevNode_;
    customerRequestType nodeType_;


public:
    DummyRequestNode();

    [[nodiscard]] inline DummyRequestNode* getterNextNode() {return nextNode_;};
    inline void updateNextNode(DummyRequestNode *node) {nextNode_ = node;};
    [[nodiscard]] inline customerRequestType getterNodeType() const {return nodeType_;};
    inline void setterNodeType(customerRequestType nodeType) {nodeType_ = nodeType;};
    [[nodiscard]] inline DummyRequestNode* getterPrevNode() {return prevNode_;};
    inline void updatePrevNode(DummyRequestNode *node) {prevNode_ = node;};
};

class BasicRequestNode : public DummyRequestNode{
    int32_t userID_;
    std::string product_ID_;
    bool processed_;
    int64_t boID_;

public:
    BasicRequestNode(int32_t userID,
                    std::string product_ID,
                    int64_t boID);

    [[nodiscard]] inline int32_t getterUserID() const {return userID_;};
    [[nodiscard]] inline std::string getterProductID() const{return product_ID_;};
    [[nodiscard]] inline int64_t getterBoID() const {return boID_;};
    [[nodiscard]] inline bool getterProcessed() const {return processed_;};
    inline void setProcessToTrue() {processed_ = true;};

};

class InsertRequestNode: public BasicRequestNode{
    double price_;
    double volume_;
    orderDirection buyOrSell_;
    orderType boType_;

public:
    InsertRequestNode(int32_t userId, std::string productId, int64_t boId, double price,
                      double volume, orderDirection buyOrSell, orderType boType);

    [[nodiscard]] inline double getterPrice() const {return price_;};
    [[nodiscard]] inline double getterVolume() const {return volume_;};
    [[nodiscard]] inline orderDirection getterOrderDirection() const {return buyOrSell_;};
    [[nodiscard]] inline orderType getterOrderType() const {return boType_;};
};

class DeleteRequestNode: public BasicRequestNode{
public:
    DeleteRequestNode(int32_t userID,
                      std::string product_ID,
                      int64_t boID);
};

class UpdateRequestNode: public InsertRequestNode{
    uint64_t updatedOrderID_;
public:
    UpdateRequestNode(int32_t userId,
                      const std::string& productId,
                      int64_t boId,
                      double price,
                      double volume,
                      orderDirection buyOrSell,
                      orderType boType,
                      uint64_t updatedOrderID);
    [[nodiscard]] inline uint64_t getterUpdatedOrderID() const {return updatedOrderID_;};
};

class CustomerRequestQueue {
    DummyRequestNode* dummyHead_;
    DummyRequestNode* tail_;

public:
    CustomerRequestQueue();
    ~CustomerRequestQueue();

    void insertNode(BasicRequestNode* node);
    static void deleteNode(BasicRequestNode* node);

};

#endif //ORDERBOOK_CUSTOMERREQUESTQUEUE_H
