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
        boType_(){}

OrderBase::OrderBase(uint32_t userID,
             uint64_t boID,
             double price,
             double volume,
             std::string productID,
             orderDirection buyOrSell,
             orderType boType)
             : userID_(userID),
               boID_(boID),
               productID_(std::move(productID)),
               buyOrSell_(buyOrSell),
               boType_(boType){
    priceInCents_ = static_cast<int>(price * 100);
    price_ = priceInCents_/100.0;
    volumeInHundredths_ = static_cast<int>(volume * 100);
    volume_ = volumeInHundredths_/100.0;
}

Order::Order(orderDirection buyOrSell)
        : OrderBase(buyOrSell),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

Order::Order(uint32_t userID,
             uint64_t boID,
             double price,
             double volume,
             std::string productID,
             orderDirection buyOrSell,
             orderType boType)
        : OrderBase(userID, boID, price, volume, std::move(productID), buyOrSell, boType),
          prevBO_(nullptr),
          nextBO_(nullptr) {}

OrderClient::OrderClient(uint32_t userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType,
                         std::string internalID,
                         std::string requestID)
        : OrderBase(userID, boID, price, volume, std::move(productID), buyOrSell, boType),
          internalID_(std::move(internalID)),
          requestID_(std::move(requestID)){}

OrderClient::OrderClient(uint32_t userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType)
        : OrderBase(userID, boID, price, volume, std::move(productID), buyOrSell, boType),
          internalID_(),
          requestID_(){}