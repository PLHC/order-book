#include "CustomerRequestQueue.h"
#include <iostream>

CustomerRequestQueue::CustomerRequestQueue():
    dummyTail_(std::make_shared<RequestNode>()),
    dummyHead_(std::make_shared<RequestNode>()),
    prevLock_(dummyHead_->prevMutex_)
    {
    dummyTail_->next_ = dummyHead_;
    dummyHead_->prev_ = dummyTail_;
}

CustomerRequestQueue::~CustomerRequestQueue() {
    // ignoring lock as no more node should be inserted at that point
    dummyHead_ = dummyHead_->prev_;
    while(dummyHead_ != dummyTail_){
        dummyHead_->status_ = CANCELLED;
        dummyHead_->statusConditionVariable_.notify_all();
        dummyHead_ = dummyHead_->prev_;
    }
}

RequestNode* CustomerRequestQueue::insertNode() {
    // check if it should react to stopFlag

    //Preparing new node before insertion
    auto newNodePtr = std::make_shared<RequestNode>();
    newNodePtr->prev_ = dummyTail_;

    //prevMutex of dummyTail is not used like for other nodes,
    //it is used as the only right to insert a node in the queue
    std::unique_lock<std::mutex> insertionLock(dummyTail_->prevMutex_);
    dummyTail_->prevConditionVariable_.wait(insertionLock, [](){return true;});

    // once allowed to process, first inserting the node by linking it
    newNodePtr->next_ = dummyTail_->next_;
    dummyTail_->next_ = newNodePtr;
    newNodePtr->next_->prev_ = newNodePtr;

    // updating dummyTail_ lock on last item in list to prevent processing to reach dummyTail
    std::unique_lock<std::mutex> newPrevLock(newNodePtr->prevMutex_);
    prevLock_ = std::move(newPrevLock);
    // and notifying processing to move forward
    newNodePtr->next_->prevConditionVariable_.notify_all();

    //releasing lock granting insertion rights
    insertionLock.unlock();
    dummyTail_->prevConditionVariable_.notify_all();

    return newNodePtr.get();
}

void CustomerRequestQueue::runNextRequest(){
    std::cout<<"running next request"<<std::endl;

    // get access to next node to process
    std::unique_lock<std::mutex> prevNodeLock(dummyHead_->prevMutex_);
    dummyHead_->prevConditionVariable_.wait(prevNodeLock,[](){return true;});
    auto processingNode = dummyHead_->prev_;

    // once access granted, update the status_ and notify the gRPC thread to process the request
    std::unique_lock<std::mutex> statusLock(processingNode->statusMutex_);
    processingNode->statusConditionVariable_.wait(statusLock, [](){return true;});
    processingNode->status_ = PROCESSING_ALLOWED;
    statusLock.unlock();
    processingNode->statusConditionVariable_.notify_all();

    // reacquire the lock once processed terminated on order book and
    // take care of deleting the next node and defining the current one as the new dummyHead
    statusLock.lock();
    processingNode->statusConditionVariable_.wait(statusLock, [&processingNode](){
        return processingNode->status_==PROCESSING_COMPLETED;
    });
    statusLock.unlock();
    prevNodeLock.unlock();// should i include a notify all?
    dummyHead_ = processingNode;
    dummyHead_->next_ = nullptr; // delete previous head
}