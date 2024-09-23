#ifndef ORDERBOOK_ORDER_H
#define ORDERBOOK_ORDER_H

#include <cstdint>
#include <utility>
#include <string>

enum orderType { FILL_OR_KILL, GOOD_TIL_CANCELLED, ORDER_TYPES_COUNT };
enum orderDirection { BUY, SELL };

class Order {
private:
    uint32_t userID_;
    uint32_t boID_;
    double price_;
    double volume_;
    int32_t priceInCents_;
    uint32_t volumeInHundredths_;
    std::string productID_;
    orderDirection buyOrSell_;
    orderType boType_;
    Order *prevBO_;
    Order *nextBO_;

public:
     explicit Order(orderDirection buyOrSell);
     Order(uint32_t userID,
           uint64_t boID,
           double price,
           double volume,
           std::string productID,
           orderDirection buyOrSell,
           orderType boType);

    Order(Order&& other) = delete;
    Order& operator=(const Order&& other) = delete;

    [[nodiscard]] inline uint32_t getterUserID() const {return userID_;};
    [[nodiscard]] inline uint64_t getterBoID() const {return boID_;};
    [[nodiscard]] inline double getterPrice() const {return price_;};
    [[nodiscard]] inline double getterVolume() const {return volume_;};
    [[nodiscard]] inline int32_t getterPriceInCents() const {return priceInCents_;};
    [[nodiscard]] inline uint32_t getterVolumeInHundredth() const {return volumeInHundredths_;};
    [[nodiscard]] inline orderDirection getterOrderDirection() const {return buyOrSell_;};
    [[nodiscard]] inline orderType getterOrderType() const {return boType_;};
    [[nodiscard]] inline Order* getterPrevBO() const {return prevBO_;};
    [[nodiscard]] inline Order* getterNextBO() const {return nextBO_;};

    void inline updateVolume(double newVolume) {
        volumeInHundredths_ = static_cast<int>(newVolume * 100);
        volume_ = volumeInHundredths_/100.0;
    };
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
