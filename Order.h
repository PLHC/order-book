//
// Created by Paul  on 04/07/2024.
//

#ifndef ORDERBOOK_ORDER_H
#define ORDERBOOK_ORDER_H

#include <cstdint>
#include <utility>
#include <string>

enum orderType { FillOrKill, GoodTilCancelled, orderTypesCount };
enum orderDirection { buy, sell };

class Order {
private:
    uint32_t userID_;
    uint32_t boID_;
    int32_t price_;
    uint32_t volume_;
    std::string productID_;
    orderDirection buyOrSell_;
    orderType boType_;
    Order *prev_bo_;
    Order *next_bo_;

public:
    explicit Order(orderDirection buyOrSell);
    explicit Order(uint32_t userID,
                   uint32_t boID,
                   int32_t price,
                   uint32_t volume,
                   std::string product_ID,
                   orderDirection buyOrSell,
                   orderType boType);

    Order(Order&& other) = delete;
    Order& operator=(const Order&& other) = delete;

    [[nodiscard]] inline uint32_t getterUserID() const {return userID_;};
    [[nodiscard]] inline uint32_t getterBoID() const {return boID_;};
    [[nodiscard]] inline int32_t getterPrice() const {return price_;};
    [[nodiscard]] inline uint32_t getterVolume() const {return volume_;};
    [[nodiscard]] inline orderDirection getterOrderDirection() const {return buyOrSell_;};
    [[nodiscard]] inline orderType getterOrderType() const {return boType_;};
    [[nodiscard]] inline Order* getterPrevBO() const {return prev_bo_;};
    [[nodiscard]] inline Order* getterNextBO() const {return next_bo_;};

    void inline updateVolume(uint32_t newVolume) {volume_=newVolume;};
    void inline updatePrevBO(Order* newBO) {prev_bo_=newBO;};
    void inline updateNextBO(Order* newBO) {next_bo_=newBO;};
    bool inline checkIfOnlyVolumeUpdatedAndDown(Order* newOrder){
        return userID_==newOrder->getterUserID() &&
               price_==newOrder->getterPrice() &&
               volume_>=newOrder->getterVolume() &&
               buyOrSell_==newOrder->getterOrderDirection() &&
               boType_==newOrder->getterOrderType();
    }
};


#endif //ORDERBOOK_ORDER_H
