#include "RandomOrderGeneratorClientImplementation.h"

RandomOrderGeneratorClient::RandomOrderGeneratorClient(const std::shared_ptr<grpc::Channel>& channel,
                                                       const uint32_t userID,
                                                       const int nbOfOrdersAtConstruction,
                                                       const int initialSpread,
                                                       const int priceForecast,
                                                       const std::string& tradedProduct)
        : Client(channel),
          OrdersInOrderbooks(),
          userID_(userID),
          priceForecastInCents_(priceForecast * 100){
    addTradedProduct(tradedProduct);
    insertOrdersAtConstruction(nbOfOrdersAtConstruction, initialSpread);
}

void
RandomOrderGeneratorClient::handleResponse(const marketAccess::InsertionConfirmation *responseParams) {
    if (responseParams->validation()) {
        std::cout <<"Insertion request for order: "<<responseParams->info()<<" successful, new BO ID: " <<
                  responseParams->boid() << std::endl;
        auto buyOrder = std::make_unique<OrderClient>(userID_,
                                                      responseParams->boid(),
                                                      10,
                                                      10,
                                                      "to be mapped",
                                                      BUY,
                                                      GOOD_TIL_CANCELLED,
                                                      responseParams->info() );
        insertOrder(buyOrder);
    }else {
        std::cout << "Insertion request for order: "<<responseParams->info()<<" failed" << std::endl;
    }
}

void RandomOrderGeneratorClient::handleResponse(const marketAccess::UpdateConfirmation *responseParams) {
    Client::handleResponse(responseParams);
}

void
RandomOrderGeneratorClient::handleResponse(const marketAccess::DeletionConfirmation *responseParams) {
    Client::handleResponse(responseParams);
}

void RandomOrderGeneratorClient::insertOrdersAtConstruction(int nbOfOrders, int initialSpread){
    auto tradedProductsList = extractListOfTradedProducts();
    while(nbOfOrders--){
        for(const auto & tradedProduct : tradedProductsList){
            generateInsertionRequestAsync(const_cast<std::string &&>(tradedProduct),
                                          userID_,
                                          10,
                                          10,
                                          BUY,
                                          GOOD_TIL_CANCELLED);
            generateInsertionRequestAsync(const_cast<std::string &&>(tradedProduct),
                                          userID_,
                                          20,
                                          20,
                                          SELL,
                                          GOOD_TIL_CANCELLED);
        }
    }
}