#include "OrderClient.h"

OrderClient::OrderClient(orderDirection buyOrSell,
                         std::string userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderType boType,
                         std::string internalID)
        : OrderBase(buyOrSell,
                    std::move(userID),
                    boID,
                    price,
                    volume,
                    std::move(productID),
                    boType)
        , internalID_(std::move(internalID)) {}
