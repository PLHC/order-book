#ifndef ORDERBOOK_ORDERBASE_H
#define ORDERBOOK_ORDERBASE_H

#include <iostream>
#include <cstdint>
#include <utility>
#include <string>

// the enum definitions need to match the order in the enums of the proto file
enum orderType { FILL_OR_KILL, GOOD_TIL_CANCELLED };
enum orderDirection { BUY, SELL };

class OrderBase{
protected:
    std::string userID_;
    uint32_t boID_;
    double price_;
    double volume_;
    int32_t priceInCents_;
    uint32_t volumeInHundredths_;
    std::string productID_;
    orderDirection buyOrSell_;
    orderType boType_;
    uint32_t version_;

public:
    explicit OrderBase(orderDirection buyOrSell,
                       std::string userID = "undefined",
                       uint64_t boID = 0,
                       double price = 0,
                       double volume = 0,
                       std::string productID = "undefined",
                       orderType boType = GOOD_TIL_CANCELLED,
                       uint32_t version = 1);

    virtual ~OrderBase() = default ;

    OrderBase(OrderBase&& other) = delete;
    OrderBase& operator=(const OrderBase&& other) = delete;

    [[nodiscard]] std::string getterUserID() const {return userID_;};
    [[nodiscard]] uint64_t getterBoID() const {return boID_;};
    [[nodiscard]] double getterPrice() const {return price_;};
    [[nodiscard]] double getterVolume() const {return volume_;};
    [[nodiscard]] int32_t getterPriceInCents() const {return priceInCents_;};
    [[nodiscard]] uint32_t getterVolumeInHundredth() const {return volumeInHundredths_;};
    [[nodiscard]] orderDirection getterOrderDirection() const {return buyOrSell_;};
    [[nodiscard]] orderType getterOrderType() const {return boType_;};
    [[nodiscard]] std::string getterProductID() const {return productID_;};
    [[nodiscard]] uint32_t getterVersion() const {return version_;};

    [[nodiscard]] bool checkIfItHasAnOlderVersionThan(OrderBase* other) const {return other->getterVersion() >= version_;};
    void updateVersion(const uint32_t newVersion) {version_ = newVersion;};
    uint32_t incrementAndReturnVersion() {
        if(version_==std::numeric_limits<uint32_t>::max()){
            throw std::overflow_error("Overflow in order version");
        }
        return ++version_;
    };

    void updateVolume(double newVolume) {
        if(newVolume<0){
            throw std::out_of_range("Volume is being updated with a negative value");
        }
        volumeInHundredths_ = static_cast<int>(newVolume * 100);
        volume_ = volumeInHundredths_/100.0;
    };
};

#endif //ORDERBOOK_ORDERBASE_H
