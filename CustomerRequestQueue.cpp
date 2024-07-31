//
// Created by Paul  on 30/07/2024.
//

#include "CustomerRequestQueue.h"
#include <utility>


CustomerRequestQueue::CustomerRequestQueue():
    queueMutex_(),
    queueConditionVariable_(),
    requestQueue_(){}

// delete_node constructor
Request::Request(customerRequestType nodeType,
                 int32_t userID,
                 std::string product_ID,
                 uint64_t boID):
        nodeType_(nodeType),
        userID_(userID),
        product_ID_(std::move(product_ID)),
        boID_(boID),
        price_(),
        volume_(),
        buyOrSell_(),
        boType_(),
        updatedOrderID_(){}

// insertion_node constructor
Request::Request(customerRequestType nodeType,
                 int32_t userID,
                 std::string product_ID,
                 uint64_t boID,
                 double price,
                 double volume,
                 orderDirection buyOrSell,
                 orderType boType):
        nodeType_(nodeType),
        userID_(userID),
        product_ID_(std::move(product_ID)),
        boID_(boID),
        price_(price),
        volume_(volume),
        buyOrSell_(buyOrSell),
        boType_(boType),
        updatedOrderID_(){}

// update_node constructor
Request::Request(customerRequestType nodeType,
                 int32_t userID,
                 std::string product_ID,
                 uint64_t boID,
                 double price,
                 double volume,
                 orderDirection buyOrSell,
                 orderType boType,
                 uint64_t updatedOrderID):
     nodeType_(nodeType),
     userID_(userID),
     product_ID_(std::move(product_ID)),
     boID_(boID),
     price_(price),
     volume_(volume),
     buyOrSell_(buyOrSell),
     boType_(boType),
     updatedOrderID_(updatedOrderID){}
