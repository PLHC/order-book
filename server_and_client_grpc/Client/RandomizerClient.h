#ifndef ORDERBOOK_RANDOMIZERCLIENT_H
#define ORDERBOOK_RANDOMIZERCLIENT_H

#include "ClientAsync.h"
#include "OrdersInClientOrderbook.h"
#include "../../market/Order.h"


class RandomizerClient : public ClientAsync, public OrdersInOrderbooks{
    int priceForecastInCents_;
    uint32_t userID_;

    uint64_t requestID_;
    std::mutex requestIdMtx_;
    std::condition_variable conditionVariableRequestIdMtx_;

    std::unordered_map<std::string, std::unique_ptr<OrderClient>> requestIdToOrderClientMap_;
    std::mutex requestIdToOrderClientMapMtx_;
    std::condition_variable conditionVariableRequestIdToOrderClientMapMtx_;

public:
    explicit RandomizerClient(const std::shared_ptr<grpc::Channel>& channel,
                              uint32_t userID_,
                              uint32_t nbOfOrdersAtConstruction,
                              uint32_t initialSpread,
                              uint32_t priceForecast,
                              const std::string& tradedProduct);

    void insertOrder(std::unique_ptr<OrderClient> & orderPtr);

private:
    uint64_t nextRequestID();

    void insertOrdersAtConstruction(uint32_t nbOfOrders, uint32_t initialSpread);
    void handleResponse(const marketAccess::InsertionConfirmation* responseParams) override;
    void handleResponse(const marketAccess::UpdateConfirmation* responseParams) override;
    void handleResponse(const marketAccess::DeletionConfirmation* responseParams) override;
};


#endif //ORDERBOOK_RANDOMIZERCLIENT_H
