#ifndef ORDERBOOK_RANDOMIZERCLIENT_H
#define ORDERBOOK_RANDOMIZERCLIENT_H

#include "../ClientAsync.h"
#include "OrdersInClientOrderbook.h"


class RandomizerClient : private ClientAsync, private OrdersMonitoring{
    std::unordered_map<std::string, int> priceForecastsInCents_;
    uint32_t spread_;
    std::string userID_;
    uint32_t expectedNbOfOrdersOnEachSide_;

    void generateInsertionRequestAsync(std::shared_ptr<OrderClient> & order);
    void generateDeleteRequestAsync(std::shared_ptr<OrderClient> & order);
    void generateUpdateRequestAsync(std::shared_ptr<OrderClient> & order,
                                    const double newPrice,
                                    const double newVolume);

    void handleResponse(const marketAccess::InsertionConfirmation* responseParams) override; 
    void handleResponse(const marketAccess::UpdateConfirmation* responseParams) override; 
    void handleResponse(const marketAccess::DeletionConfirmation* responseParams) override; 

    void deleteRandomOrders(const std::string & product);
    void updateRandomOrders(const std::string & product);

    std::shared_ptr<OrderClient> generateRandomOrder(const orderDirection direction, std::string product); 
    std::shared_ptr<OrderClient> getterRandomOrder(const std::string &product);

public:
    RandomizerClient(const std::shared_ptr<grpc::Channel>& channel,
                     std::string userID,
                     const uint32_t expectedNbOfOrders,
                     const uint32_t spread,
                     const std::vector<int> &priceForecasts,
                     const std::vector<std::string>& tradedProducts,
                     const uint32_t nbOfThreadsInThreadPool);

    void randomlyInsertOrUpdateOrDelete();
};

#endif //ORDERBOOK_RANDOMIZERCLIENT_H
