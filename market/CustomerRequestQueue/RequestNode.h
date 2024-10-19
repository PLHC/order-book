#ifndef ORDERBOOK_REQUESTNODE_H
#define ORDERBOOK_REQUESTNODE_H

#include <mutex>

enum processState {CREATED, PROCESSING_ALLOWED, PROCESSING_COMPLETED, CANCELLED};

class RequestNode {
public:
    std::shared_ptr<RequestNode> prev_, next_;
    std::mutex prevMutex_, statusMutex_;
    std::condition_variable statusConditionVariable_;
    processState status_;

    RequestNode()
        : prev_(nullptr)
        , next_(nullptr)
        , prevMutex_()
        , statusMutex_()
        , statusConditionVariable_()
        , status_(CREATED){}
};


#endif //ORDERBOOK_REQUESTNODE_H
