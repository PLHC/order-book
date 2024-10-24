#include "DisplayClient.h"

DisplayClient::DisplayClient(const std::shared_ptr<grpc::Channel> &channel,
                             std::string userID,
                             const std::vector<std::string> & tradedProducts,
                             const uint32_t nbOfLinesPerProduct,
                             const uint32_t nbOfThreadsInThreadPool)
    : ClientAsync(channel, nbOfThreadsInThreadPool),
      userID_(std::move(userID)),
      stopFlag_(false),
      nbOfLinesPerProduct_(nbOfLinesPerProduct),
      mapMtx_(){
    for(const auto & tradedProduct : tradedProducts){
        tradedProductsToOrderbookContentMap_[tradedProduct] = nullptr;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    process();
}

DisplayClient::~DisplayClient() {
    std::unique_lock mapLock(mapMtx_);
    for(const auto & [product, orderbookContent]: tradedProductsToOrderbookContentMap_){
        delete orderbookContent;
    }
}

void DisplayClient::printAllOrderbooks() {
    std::unique_lock mapLock(mapMtx_);
    system("clear");
    for(const auto & [product, orderbookContent]: tradedProductsToOrderbookContentMap_){
        if(orderbookContent) {
            std::cout << "Product: " << product << std::endl;
            std::cout << *orderbookContent;
        }
    }
}

void DisplayClient::process() {
    while( !stopFlag_.load() ) {
        std::unique_lock mapLock(mapMtx_);
        for(const auto & [product, orderbookContent]: tradedProductsToOrderbookContentMap_){
            auto p = product;
            generateDisplayRequestAsync(std::move(p), nbOfLinesPerProduct_);
        }
        mapLock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void DisplayClient::handleResponse(marketAccess::OrderBookContent *responseParams){
    if( !( responseParams->validation() ) ) {
        return;
    }
    std::unique_lock mapLock(mapMtx_);
    delete tradedProductsToOrderbookContentMap_[responseParams->product()];
    // release_orderbook assigns the string to a new pointer and returns that pointer
    // and an empty string is built in orderbook in responseParams
    // the new owner is the hash map
    tradedProductsToOrderbookContentMap_[responseParams->product()] = responseParams->release_orderbook();

    mapLock.unlock();

    printAllOrderbooks();
}
