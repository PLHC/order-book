#ifndef ORDERBOOK_ORDERBASE_H
#define ORDERBOOK_ORDERBASE_H

#include <iostream>
#include <cstdint>
#include <utility>
#include <string>

// the enum definitions need to match the order in the enums of the proto file
enum orderType { FILL_OR_KILL, GOOD_TIL_CANCELLED };
enum orderDirection { BUY, SELL };

// destructor set virtual to enable destruction of a derived class from a base pointer
// => move constructor and move assignment deleted, resulting in fallback to their copy respective functions

// copy assignment deleted automatically because userID_ and productID_ are const
// copy constructor set private (and copy asisgnment deleted) to prevent problem of slicing
// C++ Move Semantics - The complete guide - p.76)


class OrderBase{
    const std::string userID_;
    uint32_t boID_;
    double price_;
    double volume_;
    int32_t priceInCents_;
    uint32_t volumeInHundredths_;
    const std::string productID_;
    orderDirection buyOrSell_;
    orderType boType_;
    uint32_t version_;

    OrderBase(const OrderBase& other) = default;

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

//    OrderBase(OrderBase&& other) = delete;
//    OrderBase& operator=(const OrderBase&& other) = delete;

    [[nodiscard]] const std::string& getterUserID() const {return userID_;}
    [[nodiscard]] uint64_t getterBoID() const {return boID_;}
    [[nodiscard]] double getterPrice() const {return price_;}
    [[nodiscard]] double getterVolume() const {return volume_;}
    [[nodiscard]] int32_t getterPriceInCents() const {return priceInCents_;}
    [[nodiscard]] uint32_t getterVolumeInHundredth() const {return volumeInHundredths_;}
    [[nodiscard]] orderDirection getterOrderDirection() const {return buyOrSell_;}
    [[nodiscard]] orderType getterOrderType() const {return boType_;}
    [[nodiscard]] const std::string& getterProductID() const {return productID_;}
    [[nodiscard]] uint32_t getterVersion() const {return version_;}

    [[nodiscard]] bool checkIfItHasAnOlderVersionThan(OrderBase* other) const {return other->getterVersion() >= version_;}

    void updatePrice(double newPrice);
    void updateBoID(const uint64_t newBoID) { boID_ = newBoID; }
    void updateBuyOrSell(const orderDirection newBuyOrSell) { buyOrSell_ = newBuyOrSell; }
    void updateBoType(const orderType newBoType) { boType_ = newBoType; };
    void updateVolume(double newVolume);
    void updateVersion(const uint32_t newVersion) {version_ = newVersion;}
    uint32_t incrementAndReturnVersion();
};

#endif //ORDERBOOK_ORDERBASE_H
