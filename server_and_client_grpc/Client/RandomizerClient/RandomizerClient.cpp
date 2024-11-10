#include "RandomizerClient.h"

RandomizerClient::RandomizerClient(const std::shared_ptr<grpc::Channel>& channel,
                                   std::string userID,
                                   const uint32_t expectedNbOfOrders,
                                   const uint32_t spread,
                                   const std::vector<int>& priceForecasts,
                                   const std::vector<std::string>& tradedProducts,
                                   const uint32_t nbOfThreadsInThreadPool)
        : ClientAsync{ channel, nbOfThreadsInThreadPool }
        , OrdersMonitoring{ expectedNbOfOrders }
        , userID_{ std::move(userID) }
        , spread_{ spread }
        , expectedNbOfOrdersOnEachSide_{ expectedNbOfOrders }{
    for(int i = 0; i<priceForecasts.size(); ++i) {
        addTradedProductOrderbook(tradedProducts[i]);
        priceForecastsInCents_[tradedProducts[i]] = priceForecasts[i] * 100;
    }
}

void RandomizerClient::generateInsertionRequestAsync(std::shared_ptr<OrderClient> & order) {
    auto internalID = nextInternalID();
    // record internal ID to track results
    order->updateInternalID(std::to_string( internalID ) );

    //Create request
    marketAccess::InsertionParameters request;
    request.set_info(std::stoll( order->getterInternalID() ) );
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
    auto orderbook = getterSharedPointerToOrderbook(responseParams->product());

    if(!orderbook) return;

    // update order, or delete order if fully traded
    if (responseParams->validation()) { // update boID and version in local Monitoring version of the order
        if(responseParams->volume()==0){
            orderbook->deleteOrder(responseParams->info());
        }else {
            orderbook->updateOrder(responseParams->info(),
                                   responseParams->boid(),
                                   responseParams->price(),
                                   responseParams->volume(),
                                   responseParams->version());
        }
    } else { // insertion failed
        orderbook->deleteOrder(responseParams->info());
    }
}

void RandomizerClient::handleResponse(const marketAccess::UpdateConfirmation *responseParams) {
    if(!responseParams->validation()){
        return;
    }

    auto orderbook = getterSharedPointerToOrderbook(responseParams->product());

    if(!orderbook) return;
    // update order (delete if volume=0 is tested in UpdateOrder)
    orderbook->updateOrder(responseParams->info(),
                           responseParams->boid(),
                           responseParams->price(),
                           responseParams->volume(),
                           responseParams->version());
}

void RandomizerClient::handleResponse(const marketAccess::DeletionConfirmation *responseParams) {
    if(!responseParams->validation()){
        return;
    }
    auto orderbook = getterSharedPointerToOrderbook(responseParams->product());

    if(!orderbook) return;
    orderbook->deleteOrder(responseParams->info());
}



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
        auto fcast = priceForecastsInCents_[product]/100.0;
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
        if(counter.first == -1) continue; // if orderbook has been deleted from client monitoring
        if(counter.first + counter.second < 1.8 * (expectedNbOfOrdersOnEachSide_)){
            // insert if less than 90% of expected nb of orders
            auto direction = (counter.first < counter.second)? BUY : SELL;
            auto orderPtr = generateRandomOrder(direction, product);
            generateInsertionRequestAsync(orderPtr);
            continue;
        }

        std::bernoulli_distribution distribution(0.02); // 2% delete / 98% update
        if(distribution(mtGen_)){
            deleteRandomOrders(product);
        }else{
            updateRandomOrders(product);
        }
    }
}

std::shared_ptr<OrderClient> RandomizerClient::generateRandomOrder(const orderDirection direction, std::string product) {
    std::uniform_real_distribution<double> distributionVolumes(0.10, 20);
    std::uniform_real_distribution<double> distributionPrices;
    auto fcast = priceForecastsInCents_[product]/100.0;
    double price;
    orderType boType;

    if(direction==BUY) {
        distributionPrices = std::uniform_real_distribution<double>(fcast - spread_ , fcast + 1);
    }else {
        distributionPrices = std::uniform_real_distribution<double>(fcast - 1, fcast + spread_);
    }
    price = distributionPrices(mtGen_);

    // if order is more aggressive than forecast, 20% of the time is a FILL OR KILL
    if( (direction==BUY && price >= fcast) || (direction==SELL && price <= fcast) ){
        std::bernoulli_distribution distributionBoType(0.2);
        boType = distributionBoType(mtGen_)? FILL_OR_KILL : GOOD_TIL_CANCELLED;
    } else{
        boType = GOOD_TIL_CANCELLED;
    }

    return std::make_shared<OrderClient>(direction,
                                        userID_,
                                        0,
                                        price,
                                        distributionVolumes(mtGen_),
                                        std::move(product),
                                        boType);
}

std::shared_ptr<OrderClient> RandomizerClient::getterRandomOrder(const std::string &product) {
    auto distribution = std::uniform_int_distribution<uint32_t> (0, 10000);
    auto randomOrderNumber = distribution(mtGen_) % (2*expectedNbOfOrdersOnEachSide_);
    std::shared_ptr<OrderClient> selectedOrder;

    auto orderbook = getterSharedPointerToOrderbook( product );

    if(!orderbook) return nullptr;

    std::unique_lock<std::mutex> orderbookLock(orderbook->internalIdToOrderMapMtx_);
    if (orderbook->freeIndexes_.size() == 2*expectedNbOfOrdersOnEachSide_) return nullptr;

    while(orderbook->pointersToOrders_[randomOrderNumber] == nullptr){
        randomOrderNumber = (randomOrderNumber+1) % (2*expectedNbOfOrdersOnEachSide_);
    }

    selectedOrder = orderbook->pointersToOrders_[randomOrderNumber];

    return selectedOrder;
}
