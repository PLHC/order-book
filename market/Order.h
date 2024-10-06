#ifndef ORDERBOOK_ORDER_H
#define ORDERBOOK_ORDER_H

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
    explicit OrderBase(orderDirection buyOrSell);

    OrderBase(std::string userID,
            uint64_t boID,
            double price,
            double volume,
            std::string productID,
            orderDirection buyOrSell,
            orderType boType);

    OrderBase(std::string userID,
              uint64_t boID,
              double price,
              double volume,
              std::string productID,
              orderDirection buyOrSell,
              orderType boType,
              uint32_t version);

    virtual ~OrderBase() = default ;

    OrderBase(OrderBase&& other) = delete;
    OrderBase& operator=(const OrderBase&& other) = delete;

    [[nodiscard]] inline std::string getterUserID() const {return userID_;};
    [[nodiscard]] inline uint64_t getterBoID() const {return boID_;};
    [[nodiscard]] inline double getterPrice() const {return price_;};
    [[nodiscard]] inline double getterVolume() const {return volume_;};
    [[nodiscard]] inline int32_t getterPriceInCents() const {return priceInCents_;};
    [[nodiscard]] inline uint32_t getterVolumeInHundredth() const {return volumeInHundredths_;};
    [[nodiscard]] inline orderDirection getterOrderDirection() const {return buyOrSell_;};
    [[nodiscard]] inline orderType getterOrderType() const {return boType_;};
    [[nodiscard]] inline std::string getterProductID() const {return productID_;};
    [[nodiscard]] inline uint32_t getterVersion() const {return version_;};

    inline bool checkIfItHasAnOlderVersionThan(OrderBase* other) const {return other->getterVersion() >= version_;};
    inline void updateVersion(const uint32_t newVersion) {version_ = newVersion;};
    inline uint32_t incrementAndReturnVersion() {
        if(version_==std::numeric_limits<uint32_t>::max()){
            throw std::overflow_error("Overflow in order version");
        }
        return ++version_;
    };

    inline void updateVolume(double newVolume) {
        if(newVolume<0){
            throw std::out_of_range("Volume is being updated with a negative value");
        }
        volumeInHundredths_ = static_cast<int>(newVolume * 100);
        volume_ = volumeInHundredths_/100.0;
    };
};

class Order : public OrderBase{
    Order *prevBO_;
    Order *nextBO_;

public:
     explicit Order(orderDirection buyOrSell);

     Order(std::string userID,
           uint64_t boID,
           double price,
           double volume,
           std::string productID,
           orderDirection buyOrSell,
           orderType boType);

    Order(std::string userID,
          uint64_t boID,
          double price,
          double volume,
          std::string productID,
          orderDirection buyOrSell,
          orderType boType,
          uint32_t version);

    explicit Order(Order* other);

    Order(Order&& other) = delete;
    Order& operator=(const Order&& other) = delete;

    [[nodiscard]] inline Order* getterPrevBO() const {return prevBO_;};
    [[nodiscard]] inline Order* getterNextBO() const {return nextBO_;};

    void inline updatePrevBO(Order* newBO) { prevBO_=newBO;};
    void inline updateNextBO(Order* newBO) { nextBO_=newBO;};
    bool inline checkIfOnlyVolumeUpdatedAndDown(Order* newOrder){
        return userID_==newOrder->getterUserID() &&
               price_==newOrder->getterPrice() &&
               volume_>=newOrder->getterVolume() &&
               buyOrSell_==newOrder->getterOrderDirection() &&
               boType_==newOrder->getterOrderType();
    }
};

#endif //ORDERBOOK_ORDER_H