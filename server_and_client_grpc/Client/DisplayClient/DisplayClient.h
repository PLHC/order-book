#ifndef ORDERBOOK_DISPLAYCLIENT_H
#define ORDERBOOK_DISPLAYCLIENT_H

#include "../ClientAsync.h"
#include <vector>
#include <string>

class DisplayClient : public ClientAsync{
    std::unordered_map<std::string, std::string*> tradedProductsToOrderbookContentMap_; // product / orderbook content
    std::mutex mapMtx_;
    const uint32_t nbOfLinesPerProduct_;
    const std::string userID_;
    std::atomic<bool> stopFlag_;

    void printAllOrderbooks();
    void process();
    void handleResponse(marketAccess::OrderBookContent *responseParams) override;

    // unused handleResponse override by empty functions
    void handleResponse(const marketAccess::InsertionConfirmation* responseParams) override {}
    void handleResponse(const marketAccess::UpdateConfirmation* responseParams) override {}
    void handleResponse(const marketAccess::DeletionConfirmation* responseParams) override {}

public:
    void setterStopFlagToTrue() {stopFlag_.store(true);}
    DisplayClient(const std::shared_ptr<grpc::Channel> &channel,
                  std::string userID,
                  const std::vector<std::string> & tradedProducts,
                  uint32_t nbOfLinesPerProduct,
                  uint32_t nbOfThreadsInThreadPool);

    ~DisplayClient();
};

#endif //ORDERBOOK_DISPLAYCLIENT_H
