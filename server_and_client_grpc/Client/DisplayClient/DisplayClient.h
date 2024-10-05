#ifndef ORDERBOOK_DISPLAYCLIENT_H
#define ORDERBOOK_DISPLAYCLIENT_H

#include "../ClientAsync.h"
#include <vector>
#include <string>

class DisplayClient : public ClientAsync{
    std::unordered_map<std::string, std::string> tradedProductsToOrderbookContentMap_; // product / orderbook content
    std::mutex mapMtx_;
    uint32_t nbOfLinesPerProduct_;
    uint32_t userID_;
    std::atomic<bool> stopFlag_;

    void handleResponse(const marketAccess::OrderBookContent* responseParams) override;
    void handleResponse(const marketAccess::InsertionConfirmation* responseParams) override {};
    void handleResponse(const marketAccess::UpdateConfirmation* responseParams) override {};
    void handleResponse(const marketAccess::DeletionConfirmation* responseParams) override {};

    void printAllOrderbooks();
    void printOrderBook(const std::string &orderbookContent);
    void process();

public:
    inline void setterStopFlagToTrue() {stopFlag_ = true;};
    DisplayClient(const std::shared_ptr<grpc::Channel> &channel,
                  uint32_t userID,
                  const std::vector<std::string> & tradedProducts,
                  uint32_t nbOfLinesPerProduct);
};

#endif //ORDERBOOK_DISPLAYCLIENT_H
