#ifndef ORDERBOOK_SERVICEASYNC_H
#define ORDERBOOK_SERVICEASYNC_H

#include <thread>
#include <unordered_map>
#include <vector>
#include <string>
#include <iostream>

#include "RequestHandler.h"


class RpcServiceAsync final : public marketAccess::Communication::AsyncService {
    std::atomic<bool> *stopFlag_;
    grpc::ServerCompletionQueue *main_cq_;
    std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> *orderBookMap_;

public:
    // constructor to initialize the server completion queue and mappings
    RpcServiceAsync(grpc::ServerCompletionQueue *main_cq, Market *market, std::atomic<bool> *stopFlag);

    // method to handle incoming RPCs
    void handleRpcs();
};

#endif //ORDERBOOK_SERVICEASYNC_H
