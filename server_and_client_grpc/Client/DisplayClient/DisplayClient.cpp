#include "DisplayClient.h"

DisplayClient::DisplayClient(const std::shared_ptr<grpc::Channel> &channel,
                             std::string userID,
                             const std::vector<std::string> & tradedProducts,
                             uint32_t nbOfLinesPerProduct)
    : ClientAsync(channel),
      userID_(std::move(userID)),
      stopFlag_(false),
      nbOfLinesPerProduct_(nbOfLinesPerProduct),
      mapMtx_(){
    for(const auto & tradedProduct : tradedProducts){
        tradedProductsToOrderbookContentMap_[tradedProduct] = {};
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    process();
}

void DisplayClient::printAllOrderbooks() {
    std::unique_lock mapLock(mapMtx_);
    system("clear");
    for(const auto & [product, orderbookContent]: tradedProductsToOrderbookContentMap_){
        std::cout<<"Product: "<<product<<std::endl;
        std::cout<<orderbookContent;
    }
}

void DisplayClient::process() {
    while(!stopFlag_.load()) {
        std::unique_lock mapLock(mapMtx_);
        for(const auto & [product, orderbookContent]: tradedProductsToOrderbookContentMap_){
            auto p = product;
            generateDisplayRequestAsync(std::move(p), nbOfLinesPerProduct_);
        }
        mapLock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void DisplayClient::handleResponse(const marketAccess::OrderBookContent *responseParams){
    if(!(responseParams->validation())) {
        return;
    }
    std::unique_lock mapLock(mapMtx_);
    tradedProductsToOrderbookContentMap_[responseParams->product()] = responseParams->orderbook();
    mapLock.unlock();

    printAllOrderbooks();
}
