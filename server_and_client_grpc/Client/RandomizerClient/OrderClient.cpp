#include "OrderClient.h"

OrderClient::OrderClient(uint32_t userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType,
                         std::string internalID)
        : OrderBase(userID, boID, price, volume, std::move(productID), buyOrSell, boType),
          internalID_(std::move(internalID)){}

OrderClient::OrderClient(uint32_t userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType)
        : OrderBase(userID, boID, price, volume, std::move(productID), buyOrSell, boType),
          internalID_(){}