#ifndef ORDERBOOK_ORDER_H
#define ORDERBOOK_ORDER_H

#include "OrderBase.h"

// Destructor is still default and virtual from OrderBase
// As move constructor and assignment are deleted from OrderBase, they are by default deleted in Order

class Order : public OrderBase{
    Order *prevBO_;
    Order *nextBO_;

public:
    explicit Order(orderDirection buyOrSell,
                  std::string userID = "undefined",
                  int64_t boID = 0,
                  double price = 0,
                  double volume = 0,
                  std::string productID = "undefined",
                  orderType boType = GOOD_TIL_CANCELLED,
                  int32_t version = 1);

    explicit Order(Order* other);

    [[nodiscard]] Order* getterPrevBO() const {return prevBO_;}
    [[nodiscard]] Order* getterNextBO() const {return nextBO_;}
    void updatePrevBO(Order* newBO) { prevBO_=newBO; }
    void updateNextBO(Order* newBO) { nextBO_=newBO; }

    [[nodiscard]] bool checkIfOnlyVolumeUpdatedAndDownwards(Order* newOrder) const;
};

#endif //ORDERBOOK_ORDER_H