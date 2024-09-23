#ifndef ORDERBOOK_REQUESTNODE_H
#define ORDERBOOK_REQUESTNODE_H

#include <mutex>

enum processState {CREATED, PROCESSING_ALLOWED, PROCESSING_COMPLETED, CANCELLED};

class RequestNode {
public:
    std::shared_ptr<RequestNode> prev_, next_;
    std::mutex prevMutex_, statusMutex_;
    std::condition_variable prevConditionVariable_, statusConditionVariable_;
    processState status_;

    RequestNode();
};


#endif //ORDERBOOK_REQUESTNODE_H