#include "RandomizerClient.h"

RandomizerClient::RandomizerClient(const std::shared_ptr<grpc::Channel>& channel,
                                   const uint32_t userID,
                                   const uint32_t expectedNbOfOrders,
                                   const uint32_t spread,
                                   const int priceForecast,
                                   const std::string& tradedProduct)
        : ClientAsync(channel),
          OrdersMonitoring(),
          userID_(userID),
          priceForecastInCents_(priceForecast * 100),
          spread_(spread),
          expectedNbOfOrdersOnEachSide_(expectedNbOfOrders){
    addTradedProductOrderbook(tradedProduct);
    insertOrdersAtConstruction(expectedNbOfOrdersOnEachSide_, spread_);
}

void RandomizerClient::insertOrdersAtConstruction(uint32_t nbOfOrders, uint32_t initialSpread){
    initialSpread = std::max(1u, initialSpread); // minimum initial spread is set to 2
    auto tradedProductsList = extractListOfTradedProducts();
    auto fcast = priceForecastInCents_/100.0;
    std::uniform_real_distribution<double> distributionVolumes(0.10, 20);
    std::uniform_real_distribution<double> distributionBuyPrices(fcast - spread_ , fcast - 1);
    std::uniform_real_distribution<double> distributionSellPrices(fcast + 1, fcast + spread_);

    while(nbOfOrders--){
        for(const auto & tradedProduct : tradedProductsList){
            auto buyOrder = std::make_shared<OrderClient>(userID_,
                                                          0,
                                                          distributionBuyPrices(mtGen_),
                                                          distributionVolumes(mtGen_),
                                                          tradedProduct,
                                                          BUY,
                                                          GOOD_TIL_CANCELLED);
            generateInsertionRequestAsync(buyOrder);

            auto sellOrder = std::make_shared<OrderClient>(userID_,
                                                           0,
                                                           distributionSellPrices(mtGen_),
                                                           distributionVolumes(mtGen_),
                                                           tradedProduct,
                                                           SELL,
                                                           GOOD_TIL_CANCELLED);
            generateInsertionRequestAsync(sellOrder);
        }
    }
}





void RandomizerClient::generateInsertionRequestAsync(std::shared_ptr<OrderClient> & order) {
    auto internalID = nextInternalID();
    // record internal ID to track results
    order->updateInternalID(std::to_string( internalID ) );

    //Create request
    marketAccess::InsertionParameters request;
    request.set_info(std::stoi( order->getterInternalID() ) );
    request.set_userid(order->getterUserID());
    request.set_price(order->getterPrice());
    request.set_volume(order->getterVolume());
    request.set_buyorsell(static_cast<marketAccess::orderDirection>(order->getterOrderDirection()));
    request.set_botype(static_cast<marketAccess::orderType>(order->getterOrderType()));

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", order->getterProductID());

    if (!insertOrderInLocalMonitoring(order)){
        order = nullptr;
        return;
    };

    // Create an async responseParameters_ reader
    auto response = new marketAccess::InsertionConfirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::InsertionConfirmation>> rpc(
            stub_->AsyncInsertion(context, request, &cq_));
    // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context, response, status, *this});
}

void RandomizerClient::generateUpdateRequestAsync(std::shared_ptr<OrderClient> & order,
                                                  const double newPrice,
                                                  const double newVolume){

    //Create request
    marketAccess::UpdateParameters request;
    request.set_info(std::stoi( order->getterInternalID() ) );
    request.set_userid(order->getterUserID());
    request.set_price(newPrice);
    request.set_volume(newVolume);
    request.set_buyorsell(static_cast<marketAccess::orderDirection>(order->getterOrderDirection()));
    request.set_botype(static_cast<marketAccess::orderType>(order->getterOrderType()));
    request.set_boid(order->getterBoID());
    request.set_version(order->getterVersion() + 1);

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", order->getterProductID());

    // Create an async responseParameters_ reader
    auto response = new marketAccess::UpdateConfirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::UpdateConfirmation>> rpc(
            stub_->AsyncUpdate(context, request, &cq_));
    // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context,  response, status, *this});
}

void RandomizerClient::generateDeleteRequestAsync(std::shared_ptr<OrderClient> & order) {
    //Create request
    marketAccess::DeletionParameters request;
    request.set_info(std::stoi( order->getterInternalID() ) );
    request.set_userid(order->getterUserID());
    request.set_boid(order->getterBoID());

    // Create a new context_
    auto context = new grpc::ClientContext();
    context->AddMetadata("product_id", order->getterProductID());

    // Create an async responseParameters_ reader
    auto response = new marketAccess::DeletionConfirmation;
    auto status = new grpc::Status;

    std::unique_ptr<grpc::ClientAsyncResponseReader<marketAccess::DeletionConfirmation>> rpc(
            stub_->AsyncDelete(context, request, &cq_));
    // RequestHandler the async call to finish
    rpc->Finish(response, status, (void*)new RequestData{context, response, status, *this});
}

void RandomizerClient::handleResponse(const marketAccess::InsertionConfirmation *responseParams) {
    // find orderbook
    auto orderbook = getterSharedPointerToOrderbook(responseParams->product());

    if(!orderbook){
        std::cout << "Orderbook of insertion request for order: "<<responseParams->info()<<" does not exist" << std::endl;
        return;
    }

    // update order  or delete order if fully traded
    if (responseParams->validation()) { // update boID and version in local Monitoring version of the order
        if(responseParams->volume()==0){
            orderbook->deleteOrder(responseParams->info());
        }else {
            orderbook->updateOrder(responseParams->info(),
                                   responseParams->boid(),
                                   responseParams->price(),
                                   responseParams->volume(),
                                   responseParams->version());
        } // What happens if they fail?
    } else {
        orderbook->deleteOrder(responseParams->info());
        std::cout << "Insertion request for order: "<<responseParams->info()<<" failed" << std::endl;
    }
}

void RandomizerClient::handleResponse(const marketAccess::UpdateConfirmation *responseParams) {
    if(!responseParams->validation()){
        return;
    }

    auto orderbook = getterSharedPointerToOrderbook(responseParams->product());

    if(!orderbook){
        std::cout << "Orderbook of update request for order: "<<responseParams->info()<<" does not exist" << std::endl;
        return;
    }

    // update order (delete if volume=0 is tested in UpdateOrder
    orderbook->updateOrder(responseParams->info(),
                           responseParams->boid(),
                           responseParams->price(),
                           responseParams->volume(),
                           responseParams->version());
    // What happens if they fail?
}

void RandomizerClient::handleResponse(const marketAccess::DeletionConfirmation *responseParams) {
    if(!responseParams->validation()){
        return;
    }
    auto orderbook = getterSharedPointerToOrderbook(responseParams->product());

    if(!orderbook){
        std::cout << "Orderbook of delete request for order: "<<responseParams->info()<<" does not exist" << std::endl;
        return;
    }

    orderbook->deleteOrder(responseParams->info());
}




//void RandomizerClient::insertOrder(std::unique_ptr<OrderClient> &orderPtr) {
//    generateInsertionRequestAsync(orderPtr.get());
//
//    std::unique_lock<std::mutex> mapLock (requestIdToOrderClientMapMtx_);
//    conditionVariableRequestIdToOrderClientMapMtx_.wait(mapLock, [](){return true;});
//    requestIdToOrderClientMap_[orderPtr->getterRequestID()] = std::move(orderPtr);
//
//    mapLock.unlock();
//    conditionVariableRequestIdToOrderClientMapMtx_.notify_all();
//}

void RandomizerClient::deleteRandomOrders(const std::string & product) {
    auto randomOrder = getterRandomOrder(product);
    if(!randomOrder) return;

    generateDeleteRequestAsync(randomOrder);
}

void RandomizerClient::updateRandomOrders(const std::string & product) {
    auto orderPtr = getterRandomOrder(product);
    if(!orderPtr) return;

    std::bernoulli_distribution distribution(0.5);

    double newPrice = orderPtr->getterPrice();
    double newVolume = orderPtr->getterVolume();

    auto isPriceUpdated = distribution(mtGen_);
    if ( isPriceUpdated ) { // 50% chance to update price
        auto fcast = priceForecastInCents_/100.0;
        std::uniform_real_distribution<double> distributionPrices;
        if(orderPtr->getterOrderDirection()==BUY) {
            distributionPrices = std::uniform_real_distribution<double>(fcast - spread_ , fcast + 1);
        }else {
            distributionPrices = std::uniform_real_distribution<double>(fcast - 1, fcast + spread_);
        }
        newPrice = distributionPrices(mtGen_);
    }
    // volume updated if price not updated and otherwise 50% chance to update volume
    if( !isPriceUpdated || distribution(mtGen_) ) {
        std::uniform_real_distribution<double> distributionVolumes(0.10, 20);
        newVolume = distributionVolumes(mtGen_);
    }
    generateUpdateRequestAsync(orderPtr, newPrice, newVolume);
}

void RandomizerClient::randomlyInsertOrUpdateOrDelete() {
    for(const auto & product: extractListOfTradedProducts()){
        auto counter = getterBuyAndSellNbOrders(product);
        if(counter.first == -1) continue;
        if(counter.first + counter.second < 1.8 * (expectedNbOfOrdersOnEachSide_)){
            std::cout<<"inserting"<<std::endl;
            // insert if less than 90% of expected nb of orders
            auto direction = (counter.first < counter.second)? BUY : SELL;
            auto orderPtr = generateRandomOrder(direction, product);
            generateInsertionRequestAsync(orderPtr);
            continue;
        }

        std::bernoulli_distribution distribution(0.02); // 2% delete / 98% update
        if(distribution(mtGen_)){
            std::cout<<"deleting"<<std::endl;
            deleteRandomOrders(product);
        }else{
            std::cout<<"updating"<<std::endl;
            updateRandomOrders(product);
        }
    }
}

std::shared_ptr<OrderClient> RandomizerClient::generateRandomOrder(const orderDirection direction,
                                                                   std::string product) {
    std::uniform_real_distribution<double> distributionVolumes(0.10, 20);

    std::uniform_real_distribution<double> distributionPrices;
    auto fcast = priceForecastInCents_/100.0;
    if(direction==BUY) {
        distributionPrices = std::uniform_real_distribution<double>(fcast - spread_ , fcast + 1);
    }else {
        distributionPrices = std::uniform_real_distribution<double>(fcast - 1, fcast + spread_);
    }

    return std::make_shared<OrderClient>(userID_,
                                        0,
                                        distributionPrices(mtGen_),
                                        distributionVolumes(mtGen_),
                                        std::move(product),
                                        direction,
                                        GOOD_TIL_CANCELLED);
}

std::shared_ptr<OrderClient> RandomizerClient::getterRandomOrder(const std::string &product) {
    auto distribution = std::uniform_int_distribution<uint32_t> (0, 10000);
    auto randomOrderNumber = distribution(mtGen_);
    std::shared_ptr<OrderClient> selectedOrder;

    auto orderbook = getterSharedPointerToOrderbook(product);

    if(!orderbook) return nullptr;

    std::unique_lock<std::mutex> orderbookLock(orderbook->internalIdToOrderMapMtx_);
    orderbook->internalIdToOrderMapConditionVariable_.wait(orderbookLock, [](){ return true;});

    randomOrderNumber %= orderbook->getterNbBuyOrders() + orderbook->getterNbSellOrders();

    for(const auto & [ID, orderPtr] : orderbook->internalIdToOrderMap_){
        if(!randomOrderNumber--){
            selectedOrder = orderPtr;
        }
    }

    orderbookLock.unlock();
    orderbook->internalIdToOrderMapConditionVariable_.notify_all();

    return selectedOrder;
}
