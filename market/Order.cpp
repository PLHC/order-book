#include "Order.h"

Order::Order(orderDirection buyOrSell):
        userID_(),
        boID_(),
        price_(),
        volume_(),
        priceInCents_(),
        volumeInHundredths_(),
        productID_(),
        buyOrSell_(buyOrSell),
        boType_(),
        prevBO_(nullptr),
        nextBO_(nullptr) {}

Order::Order(uint32_t userID,
             uint64_t boID,
             double price,
             double volume,
             std::string productID,
             orderDirection buyOrSell,
             orderType boType):
        userID_(userID),
        boID_(boID),
        productID_(std::move(productID)),
        buyOrSell_(buyOrSell),
        boType_(boType),
        prevBO_(nullptr),
        nextBO_(nullptr) {
    priceInCents_ = static_cast<int>(price * 100);
    price_ = priceInCents_/100.0;
    volumeInHundredths_ = static_cast<int>(volume * 100);
    volume_ = volumeInHundredths_/100.0;
}