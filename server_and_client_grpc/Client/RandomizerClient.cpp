#include "RandomizerClient.h"

RandomizerClient::RandomizerClient(const std::shared_ptr<grpc::Channel>& channel,
                                   const uint32_t userID,
                                   const uint32_t nbOfOrdersAtConstruction,
                                   const uint32_t initialSpread,
                                   const uint32_t priceForecast,
                                   const std::string& tradedProduct)
        : ClientAsync(channel),
          OrdersInOrderbooks(),
          userID_(userID),
          priceForecastInCents_(priceForecast * 100),
          requestID_(1){
    addTradedProduct(tradedProduct);
    insertOrdersAtConstruction(nbOfOrdersAtConstruction, initialSpread);
}

void
RandomizerClient::handleResponse(const marketAccess::InsertionConfirmation *responseParams) {
    if (responseParams->validation()) {
        std::cout <<"Insertion request for order: "<<responseParams->info()<<" successful, new BO ID: " <<
                  responseParams->boid() << std::endl;

        std::unique_lock<std::mutex> mapLock (requestIdToOrderClientMapMtx_);
        conditionVariableRequestIdToOrderClientMapMtx_.wait(mapLock, [](){return true;});

        insertOrderInLocalMonitoring(requestIdToOrderClientMap_[responseParams->info()]);
        requestIdToOrderClientMap_.erase(responseParams->info());

        mapLock.unlock();
        conditionVariableRequestIdToOrderClientMapMtx_.notify_all();
    }else {
        std::cout << "Insertion request for order: "<<responseParams->info()<<" failed" << std::endl;
    }
}

void RandomizerClient::handleResponse(const marketAccess::UpdateConfirmation *responseParams) {
    return;
}

void
RandomizerClient::handleResponse(const marketAccess::DeletionConfirmation *responseParams) {
    return;
}

void RandomizerClient::insertOrdersAtConstruction(uint32_t nbOfOrders, uint32_t initialSpread){
    initialSpread = std::max(1u, initialSpread); // minimum initial spread is set to 2
    auto tradedProductsList = extractListOfTradedProducts();
    auto fcast = priceForecastInCents_/100.0;
    std::uniform_real_distribution<double> distributionVolumes(0.10, 20);
    std::uniform_real_distribution<double> distributionBuyPrices(fcast - initialSpread , fcast - 1);
    std::uniform_real_distribution<double> distributionSellPrices(fcast + initialSpread , fcast + 1);

    while(nbOfOrders--){
        for(const auto & tradedProduct : tradedProductsList){
            auto buyOrder = std::make_unique<OrderClient>(userID_,
                                                          0,
                                                          distributionBuyPrices(gen),
                                                          distributionVolumes(gen),
                                                          tradedProduct,
                                                          BUY,
                                                          GOOD_TIL_CANCELLED);
            insertOrder(buyOrder);

            auto sellOrder = std::make_unique<OrderClient>(userID_,
                                                          0,
                                                           distributionSellPrices(gen),
                                                           distributionVolumes(gen),
                                                          tradedProduct,
                                                          SELL,
                                                          GOOD_TIL_CANCELLED);
            insertOrder(sellOrder);
        }
    }
}

uint64_t RandomizerClient::nextRequestID() {
    std::unique_lock<std::mutex> requestIdLock(requestIdMtx_);
    conditionVariableRequestIdMtx_.wait(requestIdLock, [](){return true;});

    auto nextID = requestID_++;

    requestIdLock.unlock();
    conditionVariableRequestIdMtx_.notify_all();

    return nextID;
}

void RandomizerClient::insertOrder(std::unique_ptr<OrderClient> &orderPtr) {
    orderPtr->updateRequestID(std::to_string(nextRequestID()));
    generateInsertionRequestAsync(orderPtr.get());

    std::unique_lock<std::mutex> mapLock (requestIdToOrderClientMapMtx_);
    conditionVariableRequestIdToOrderClientMapMtx_.wait(mapLock, [](){return true;});
    std::cout<<orderPtr->getterRequestID()<<std::endl;
    requestIdToOrderClientMap_[orderPtr->getterRequestID()] = std::move(orderPtr);

    mapLock.unlock();
    conditionVariableRequestIdToOrderClientMapMtx_.notify_all();
}
