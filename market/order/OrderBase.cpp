#include "OrderBase.h"


OrderBase::OrderBase(orderDirection buyOrSell,
                     std::string userID,
                     uint64_t boID,
                     double price,
                     double volume,
                     std::string productID,
                     orderType boType,
                     uint32_t version)
        : userID_(std::move(userID))
        , boID_(boID)
        , productID_(std::move(productID))
        , buyOrSell_(buyOrSell)
        , boType_(boType)
        , version_(version){
    if(volume<0){
        throw std::out_of_range("Order is being initialized with a negative volume value");
    }
    priceInCents_ = static_cast<int32_t>(price * 100);
    price_ = priceInCents_/100.0;
    volumeInHundredths_ = static_cast<uint32_t>(volume * 100);
    volume_ = volumeInHundredths_/100.0;
}

uint32_t OrderBase::incrementAndReturnVersion() {
    if(version_==std::numeric_limits<uint32_t>::max()){
        throw std::overflow_error("Overflow in order version");
    }
    return ++version_;
}

void OrderBase::updateVolume(const double newVolume) {
    if(newVolume<0){
        throw std::out_of_range("Volume is being updated with a negative value");
    }
    volumeInHundredths_ = static_cast<uint32_t>(newVolume * 100);
    volume_ = volumeInHundredths_/100.0;
}

void OrderBase::updatePrice(const double newPrice) {
    priceInCents_ = static_cast<int>(newPrice * 100);
    price_ = priceInCents_/100.0;
}