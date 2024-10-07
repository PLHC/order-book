#ifndef ORDERBOOK_DISPLAYCLIENT_H
#define ORDERBOOK_DISPLAYCLIENT_H

#include "../ClientAsync.h"
#include <vector>
#include <string>

class DisplayClient : public ClientAsync{
    std::unordered_map<std::string, std::string> tradedProductsToOrderbookContentMap_; // product / orderbook content
    std::mutex mapMtx_;
    uint32_t nbOfLinesPerProduct_;
    std::string userID_;
    std::atomic<bool> stopFlag_;

    void handleResponse(const marketAccess::OrderBookContent* responseParams) override;
    void handleResponse(const marketAccess::InsertionConfirmation* responseParams) override {};
    void handleResponse(const marketAccess::UpdateConfirmation* responseParams) override {};
    void handleResponse(const marketAccess::DeletionConfirmation* responseParams) override {};

    void printAllOrderbooks();
    void process();

public:
    inline void setterStopFlagToTrue() {stopFlag_.store(true);};
    DisplayClient(const std::shared_ptr<grpc::Channel> &channel,
                  std::string userID,
                  const std::vector<std::string> & tradedProducts,
                  const uint32_t nbOfLinesPerProduct,
                  const uint32_t nbOfThreadsInThreadPool);
};

#endif //ORDERBOOK_DISPLAYCLIENT_H
