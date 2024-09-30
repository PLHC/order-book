#ifndef ORDERBOOK_ORDERCLIENT_H
#define ORDERBOOK_ORDERCLIENT_H

#include "../../../market/Order.h"


class OrderClient : public OrderBase{
    std::string internalID_;

public:
    explicit OrderClient(uint32_t userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType,
                         std::string internalID);

    explicit OrderClient(uint32_t userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType);

    ~OrderClient() override = default;

    [[nodiscard]] inline std::string getterInternalID() const {return internalID_;};

    void inline updatePrice(double newPrice) {
        priceInCents_ = static_cast<int>(newPrice * 100);
        price_ = priceInCents_/100.0;
    };
    void inline updateBoID(uint64_t newBoID) {boID_ = newBoID;};
    void inline updateBuyOrSell(orderDirection newBuyOrSell) {buyOrSell_ = newBuyOrSell;};
    void inline updateBoType(orderType newBoType) {boType_ = newBoType;};
    void inline updateInternalID(std::string newInternalID) { internalID_ = std::move(newInternalID);};
};


#endif //ORDERBOOK_ORDERCLIENT_H
