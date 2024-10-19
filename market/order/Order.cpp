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
        : OrderBase(other->buyOrSell_,
                    other->userID_,
                    other->boID_,
                    other->price_,
                    other->volume_,
                    other->productID_,
                    other->boType_,
                    other->version_),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

bool Order::checkIfOnlyVolumeUpdatedAndDown(Order* newOrder) const{
    return userID_==newOrder->getterUserID() &&
           price_==newOrder->getterPrice() &&
           volume_>=newOrder->getterVolume() &&
           buyOrSell_==newOrder->getterOrderDirection() &&
           boType_==newOrder->getterOrderType();
}
