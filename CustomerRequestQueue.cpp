//
// Created by Paul  on 30/07/2024.
//

#include "CustomerRequestQueue.h"
#include <utility>


DummyRequestNode::DummyRequestNode():
    nextNode_(nullptr),
    prevNode_(nullptr),
    nodeType_(dummy){}

BasicRequestNode::BasicRequestNode(int32_t userID,
                                   std::string product_ID,
                                   uint64_t boID):
    userID_(userID),
    product_ID_(std::move(product_ID)),
    processed_(false),
    boID_(boID){
    setterNodeType(basic);
}

InsertRequestNode::InsertRequestNode(int32_t userId,
                                     std::string productId,
                                     uint64_t boId,
                                     double price,
                                     double volume,
                                     orderDirection buyOrSell,
                                     orderType boType) :
    BasicRequestNode(userId,
                     std::move(productId),
                     boId),
    price_(price),
    volume_(volume),
    buyOrSell_(buyOrSell),
    boType_(boType) {
    setterNodeType(insertion);
}


DeleteRequestNode::DeleteRequestNode(int32_t userID,
                                     std::string product_ID,
                                     uint64_t boID) :
     BasicRequestNode(userID,
                      std::move(product_ID),
                      boID) {
     setterNodeType(deletion);
}

UpdateRequestNode::UpdateRequestNode(int32_t userId,
                                     const std::string& productId,
                                     uint64_t boId,
                                     double price,
                                     double volume,
                                     orderDirection buyOrSell,
                                     orderType boType,
                                     uint64_t updatedOrderID):
    InsertRequestNode(userId,
                      productId,
                      boId,
                      price,
                      volume,
                      buyOrSell,
                      boType),
    updatedOrderID_(updatedOrderID){
    setterNodeType(update);
}

CustomerRequestQueue::CustomerRequestQueue():
    queueMutex_(),
    queueConditionVariable_(){
    dummyHead_ = new DummyRequestNode();
    tail_ = dummyHead_;
}

CustomerRequestQueue::~CustomerRequestQueue() {
    while(dummyHead_){
        tail_ = dummyHead_->getterNextNode();
        delete dummyHead_;
        dummyHead_ = tail_;
    }
}

void CustomerRequestQueue::insertNode(BasicRequestNode *node) {
    node->updateNextNode(dummyHead_->getterNextNode());
    dummyHead_->updateNextNode(node);
    node->updatePrevNode(dummyHead_);
    node->getterNextNode()->updatePrevNode(node);
    if(tail_==dummyHead_) tail_=node;
}

void CustomerRequestQueue::deleteNode(BasicRequestNode *node) {
    node->getterPrevNode()->updateNextNode(node->getterNextNode());
    node->getterNextNode()->updatePrevNode(node->getterPrevNode());
    delete node;
}
