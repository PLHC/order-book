#include "CustomerRequestQueue.h"
#include <iostream>

CustomerRequestQueue::CustomerRequestQueue()
    : dummyTail_(std::make_shared<RequestNode>())
    , dummyHead_(std::make_shared<RequestNode>())
    , prevLock_(dummyHead_->prevMutex_)
{
    dummyTail_->next_ = dummyHead_;
    dummyHead_->prev_ = dummyTail_;
}

RequestNode* CustomerRequestQueue::insertNode() {
    // preparing new node before insertion
    auto newNodePtr = std::make_shared<RequestNode>();
    newNodePtr->prev_ = dummyTail_;

    // prevMutex of dummyTail is not used like for other nodes,
    // it is used as the only right to insert a node in the queue
    std::unique_lock<std::mutex> insertionLock(dummyTail_->prevMutex_);

    // once allowed to process, first inserting the node by linking it
    newNodePtr->next_ = dummyTail_->next_;
    dummyTail_->next_ = newNodePtr;
    newNodePtr->next_->prev_ = newNodePtr;

    // updating dummyTail_ lock on last item in list to prevent processing to reach dummyTail
    std::unique_lock<std::mutex> newPrevLock(newNodePtr->prevMutex_);
    prevLock_ = std::move(newPrevLock);

    // releasing lock, granting insertion rights
    insertionLock.unlock();

    return newNodePtr.get();
}

void CustomerRequestQueue::runNextRequest(){

    // get access to next node to process
    std::unique_lock<std::mutex> prevNodeLock(dummyHead_->prevMutex_);
    auto processingNode = dummyHead_->prev_;

    if(processingNode->status_ == CANCELLED){ // stop processing requests and acquiring new locks
        prevNodeLock.unlock();
        return;
    }

    // once access granted, update the status_ and notify the gRPC thread to process the request
    std::unique_lock<std::mutex> statusLock(processingNode->statusMutex_);
    if(processingNode->status_ != CANCELLED){
        processingNode->status_ = PROCESSING_ALLOWED;
    }
    statusLock.unlock();
    processingNode->statusConditionVariable_.notify_all();

    if(processingNode->status_ == CANCELLED){ // stop processing requests and acquiring new locks
        prevNodeLock.unlock();
        return;
    }

    // reacquire the lock once processed terminated on order book,
    // take care of deleting the next node
    // and defining the current one as the new dummyHead
    statusLock.lock();
    processingNode->statusConditionVariable_.wait(statusLock, [&processingNode](){
        return processingNode->status_ == PROCESSING_COMPLETED || processingNode->status_ == CANCELLED;
    });
    statusLock.unlock();
    prevNodeLock.unlock();
    dummyHead_ = processingNode;
    dummyHead_->next_ = nullptr; // delete previous head
}