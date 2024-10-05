#include "DisplayClient.h"

DisplayClient::DisplayClient(const std::shared_ptr<grpc::Channel> &channel,
                             uint32_t userID,
                             const std::vector<std::string> & tradedProducts,
                             uint32_t nbOfLinesPerProduct)
    : ClientAsync(channel),
      userID_(userID),
      stopFlag_(false),
      nbOfLinesPerProduct_(nbOfLinesPerProduct),
      mapMtx_(){
    for(const auto & tradedProduct : tradedProducts){
        tradedProductsToOrderbookContentMap_[tradedProduct] = {};
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    process();
}

void DisplayClient::printOrderBook(const std::string &orderbookContent){
    std::istringstream stream(orderbookContent);
    std::string line;
    auto lineCount = nbOfLinesPerProduct_;

    while (lineCount > 0 && std::getline(stream, line)) {
        std::cout << line << std::endl;
        lineCount--;
    }
    while(lineCount--){
        std::cout<<std::endl;
    }
}

void DisplayClient::printAllOrderbooks() {
    system("clear");
    std::unique_lock mapLock(mapMtx_);
    for(const auto & [product, orderbookContent]: tradedProductsToOrderbookContentMap_){
        std::cout<<"Product: "<<product<<std::endl;
        printOrderBook(orderbookContent);
        std::cout<<std::endl;
    }
}

void DisplayClient::process() {
    while(!stopFlag_) {
        std::unique_lock mapLock(mapMtx_);
        for(const auto & [product, orderbookContent]: tradedProductsToOrderbookContentMap_){
            auto p = product;
            generateDisplayRequestAsync("abc", std::move(p));
        }
        mapLock.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
