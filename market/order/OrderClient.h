#ifndef ORDERBOOK_ORDERCLIENT_H
#define ORDERBOOK_ORDERCLIENT_H

#include "OrderBase.h"

// Destructor is still default and virtual from OrderBase
// As move constructor and assignment are deleted from OrderBase, they are by default deleted in OrderClient

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

    [[nodiscard]] const std::string& getterInternalID() const { return internalID_; }
    void updateInternalID(std::string newInternalID) { internalID_ = std::move(newInternalID); }
};

#endif //ORDERBOOK_ORDERCLIENT_H
