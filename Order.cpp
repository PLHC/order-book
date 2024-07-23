//
// Created by Paul  on 04/07/2024.
//

#include "Order.h"

Order::Order(orderDirection buyOrSell):
        userID_(),
        boID_(),
        price_(),
        volume_(),
        productID_(),
        buyOrSell_(buyOrSell),
        boType_(),
        prev_bo_(nullptr),
        next_bo_(nullptr) {}

Order::Order(uint32_t userID,
             uint32_t boID,
             int32_t price,
             uint32_t volume,
             std::string product_ID,
             orderDirection buyOrSell,
             orderType boType):
        userID_(userID),
        boID_(boID),
        price_(price),
        volume_(volume),
        productID_(std::move(product_ID)),
        buyOrSell_(buyOrSell),
        boType_(boType),
        prev_bo_(nullptr),
        next_bo_(nullptr) {}