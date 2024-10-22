#include "Order.h"


Order::Order(orderDirection buyOrSell,
             std::string userID,
             uint64_t boID,
             double price,
             double volume,
             std::string productID,
             orderType boType,
             uint32_t version)
        : OrderBase(buyOrSell, std::move(userID), boID, price, volume, std::move(productID), boType, version)
        , prevBO_(nullptr)
        , nextBO_(nullptr) {}

Order::Order(Order* other)
        : OrderBase(other->getterOrderDirection(),
                    other->getterUserID(),
                    other->getterBoID(),
                    other->getterPrice(),
                    other->getterVolume(),
                    other->getterProductID(),
                    other->getterOrderType(),
                    other->getterVersion()),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

bool Order::checkIfOnlyVolumeUpdatedAndDownwards(Order* newOrder) const{
    return getterUserID() == newOrder->getterUserID() &&
           getterPrice() == newOrder->getterPrice() &&
           getterVolume() >= newOrder->getterVolume() &&
           getterOrderDirection() == newOrder->getterOrderDirection() &&
           getterOrderType() == newOrder->getterOrderType();
}
