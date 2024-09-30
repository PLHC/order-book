#ifndef ORDERBOOK_RANDOMIZERCLIENT_H
#define ORDERBOOK_RANDOMIZERCLIENT_H

#include "../ClientAsync.h"
#include "OrdersInClientOrderbook.h"


class RandomizerClient : public ClientAsync, public OrdersMonitoring{
    int priceForecastInCents_;
    uint32_t spread_;
    uint32_t userID_;
    uint32_t expectedNbOfOrdersOnEachSide_;

public:
    RandomizerClient(const std::shared_ptr<grpc::Channel>& channel, ////
                              const uint32_t userID,
                              const uint32_t expectedNbOfOrders,
                              const uint32_t spread,
                              const int priceForecast,
                              const std::string& tradedProduct);


    void randomlyInsertOrUpdateOrDelete();

private:
    void insertOrdersAtConstruction(uint32_t nbOfOrders, uint32_t initialSpread); ////

    void generateInsertionRequestAsync(std::shared_ptr<OrderClient> & order); ////
    void generateUpdateRequestAsync(std::shared_ptr<OrderClient> & order, ////
                                    const double newPrice,
                                    const double newVolume);
    void generateDeleteRequestAsync(std::shared_ptr<OrderClient> & order); ////

    void handleResponse(const marketAccess::InsertionConfirmation* responseParams) override; ////
    void handleResponse(const marketAccess::UpdateConfirmation* responseParams) override;
    void handleResponse(const marketAccess::DeletionConfirmation* responseParams) override;

//    void deleteRandomOrders();
//    void updateRandomOrders();

    std::shared_ptr<OrderClient> generateRandomOrder(const orderDirection direction, std::string product);
};

#endif //ORDERBOOK_RANDOMIZERCLIENT_H
