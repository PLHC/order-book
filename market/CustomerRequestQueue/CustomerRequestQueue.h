#ifndef ORDERBOOK_CUSTOMERREQUESTQUEUE_H
#define ORDERBOOK_CUSTOMERREQUESTQUEUE_H

#include "RequestNode.h"

class CustomerRequestQueue {
public:
    std::shared_ptr<RequestNode> dummyHead_, dummyTail_;
    std::unique_lock<std::mutex> prevLock_;

    CustomerRequestQueue();

    CustomerRequestQueue(CustomerRequestQueue&& other) = delete;
    CustomerRequestQueue& operator=(const CustomerRequestQueue&& other) = delete;

    RequestNode* insertNode();
    void runNextRequest();
};

#endif //ORDERBOOK_CUSTOMERREQUESTQUEUE_H
