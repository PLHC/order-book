#include "OrderClient.h"

OrderClient::OrderClient(std::string userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType,
                         std::string internalID)
        : OrderBase(std::move(userID), boID, price, volume, std::move(productID), buyOrSell, boType),
          internalID_(std::move(internalID)){}

OrderClient::OrderClient(std::string userID,
                         uint64_t boID,
                         double price,
                         double volume,
                         std::string productID,
                         orderDirection buyOrSell,
                         orderType boType)
        : OrderBase(std::move(userID), boID, price, volume, std::move(productID), buyOrSell, boType),
          internalID_(){}