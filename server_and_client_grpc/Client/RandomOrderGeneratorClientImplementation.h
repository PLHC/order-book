#ifndef ORDERBOOK_RANDOMORDERGENERATORCLIENTIMPLEMENTATION_H
#define ORDERBOOK_RANDOMORDERGENERATORCLIENTIMPLEMENTATION_H

#include "ClientAsyncImplementation.h"
#include "ClientOrdersInOrderbook.h"
#include "../../market/Order.h"


class RandomOrderGeneratorClient : public Client, public OrdersInOrderbooks{
public:
    explicit RandomOrderGeneratorClient(const std::shared_ptr<grpc::Channel>& channel,
                                        uint32_t userID_,
                                        int nbOfOrdersAtConstruction,
                                        int initialSpread,
                                        int priceForecast,
                                        const std::string& tradedProduct);


private:
    int priceForecastInCents_;
    uint32_t userID_;

    void insertOrdersAtConstruction(int nbOfOrders, int initialSpread);
    void handleResponse(const marketAccess::InsertionConfirmation* responseParams) override;
    void handleResponse(const marketAccess::UpdateConfirmation* responseParams) override;
    void handleResponse(const marketAccess::DeletionConfirmation* responseParams) override;
};


#endif //ORDERBOOK_RANDOMORDERGENERATORCLIENTIMPLEMENTATION_H
