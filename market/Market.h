#ifndef ORDERBOOK_MARKET_H
#define ORDERBOOK_MARKET_H

#include "OrderBook.h"
#include "proto/MarketAccess.grpc.pb.h"
#include "proto/MarketAccess.pb.h"

#include <thread>
#include <iostream>


struct StringHash{
    using is_transparent = void; // this indicates to the STL containers the hash function can handle multiple key types.
    size_t operator()(const std::string & str) const {
        return std::hash<std::string>{}(str);
    }
    size_t operator()(const std::string_view str) const {
        return std::hash<std::string_view>{}(str);
    }
    size_t operator()(const char* str) const {
        return std::hash<std::string_view>{}(str);
    }
};

class Market {
private:
    std::mutex orderbookMapMtx_;
    GeneratorId* genId_;

public:
    std::unordered_map<std::string, OrderBook*, StringHash, std::equal_to<>> productToOrderBookMap_;

    Market(): genId_(GeneratorId::getInstance()), productToOrderBookMap_(){}
    ~Market();

    Market(Market& other) = delete;
    Market(Market&& other) = delete;
    Market& operator=(Market&&) = delete;

    void createNewOrderBook(const std::string& product_ID);
    void deleteOrderBook(const std::string& product_ID);

    inline OrderBook* getterOrderBookPointer(const std::string& productID) {return productToOrderBookMap_[productID];};
};

#endif //ORDERBOOK_MARKET_H
