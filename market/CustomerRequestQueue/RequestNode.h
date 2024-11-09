#ifndef ORDERBOOK_REQUESTNODE_H
#define ORDERBOOK_REQUESTNODE_H

#include <mutex>

enum processState {CREATED, PROCESSING_ALLOWED, PROCESSING_COMPLETED, CANCELLED};

class RequestNode {
public:
    std::shared_ptr<RequestNode> prev_{nullptr};
    std::shared_ptr<RequestNode> next_{nullptr};
    std::mutex prevMutex_{};
    std::mutex statusMutex_{};
    std::condition_variable statusConditionVariable_{};
    processState status_{CREATED};
};


#endif //ORDERBOOK_REQUESTNODE_H
