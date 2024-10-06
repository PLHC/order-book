#include "Order.h"

OrderBase::OrderBase(orderDirection buyOrSell):
        userID_(),
        boID_(),
        price_(),
        volume_(),
        priceInCents_(),
        volumeInHundredths_(),
        productID_(),
        buyOrSell_(buyOrSell),
        boType_(),
        version_(1){}

OrderBase::OrderBase(std::string userID,
             uint64_t boID,
             double price,
             double volume,
             std::string productID,
             orderDirection buyOrSell,
             orderType boType)
             : userID_(std::move(userID)),
               boID_(boID),
               productID_(std::move(productID)),
               buyOrSell_(buyOrSell),
               boType_(boType),
               version_(1){
    if(volume<0){
        throw std::out_of_range("Order is being initialized with a negative volume value");
    }
    priceInCents_ = static_cast<int>(price * 100);
    price_ = priceInCents_/100.0;
    volumeInHundredths_ = static_cast<int>(volume * 100);
    volume_ = volumeInHundredths_/100.0;
}

OrderBase::OrderBase(std::string userID,
                     uint64_t boID,
                     double price,
                     double volume,
                     std::string productID,
                     orderDirection buyOrSell,
                     orderType boType,
                     uint32_t version)
        : userID_(std::move(userID)),
          boID_(boID),
          productID_(std::move(productID)),
          buyOrSell_(buyOrSell),
          boType_(boType),
          version_(version){
    if(volume<0){
        throw std::out_of_range("Order is being initialized with a negative volume value");
    }
    priceInCents_ = static_cast<int>(price * 100);
    price_ = priceInCents_/100.0;
    volumeInHundredths_ = static_cast<int>(volume * 100);
    volume_ = volumeInHundredths_/100.0;
}

Order::Order(orderDirection buyOrSell)
        : OrderBase(buyOrSell),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

Order::Order(std::string userID,
             uint64_t boID,
             double price,
             double volume,
             std::string productID,
             orderDirection buyOrSell,
             orderType boType)
        : OrderBase(userID, boID, price, volume, std::move(productID), buyOrSell, boType),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

Order::Order(std::string userID,
             uint64_t boID,
             double price,
             double volume,
             std::string productID,
             orderDirection buyOrSell,
             orderType boType,
             uint32_t version)
        : OrderBase(userID, boID, price, volume, std::move(productID), buyOrSell, boType, version),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

Order::Order(Order *other)
        : OrderBase(other->userID_,
                    other->boID_,
                    other->price_,
                    other->volume_,
                    other->productID_,
                    other->buyOrSell_,
                    other->boType_,
                    other->version_),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

