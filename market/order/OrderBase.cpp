#include "OrderBase.h"


OrderBase::OrderBase(orderDirection buyOrSell,
                     std::string userID,
                     uint64_t boID,
                     double price,
                     double volume,
                     std::string productID,
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
