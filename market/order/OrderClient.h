#ifndef ORDERBOOK_ORDERCLIENT_H
#define ORDERBOOK_ORDERCLIENT_H

#include "OrderBase.h"


class OrderClient : public OrderBase{
    std::string internalID_;

public:
    OrderClient(orderDirection buyOrSell,
                std::string userID,
                uint64_t boID,
                double price,
                double volume,
                std::string productID,
                orderType boType,
                std::string internalID = "0");

    [[nodiscard]] std::string getterInternalID() const { return internalID_; };

    void updatePrice(double newPrice) {
        priceInCents_ = static_cast<int>(newPrice * 100);
        price_ = priceInCents_/100.0;
    };
    void updateBoID(uint64_t newBoID) { boID_ = newBoID; };
    void updateBuyOrSell(orderDirection newBuyOrSell) { buyOrSell_ = newBuyOrSell; };
    void updateBoType(orderType newBoType) { boType_ = newBoType; };
    void updateInternalID(std::string newInternalID) { internalID_ = std::move(newInternalID); };
};


#endif //ORDERBOOK_ORDERCLIENT_H
