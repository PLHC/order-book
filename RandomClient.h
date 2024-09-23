#ifndef ORDERBOOK_RANDOMCLIENT_H
#define ORDERBOOK_RANDOMCLIENT_H

#include "./market/Order.h"
#include <unordered_map>
#include <unordered_set>
#include <array>


class Quantiles{
public:
    double forecast_;
    std::array<uint32_t, 10> quantilePopulation_;
    std::unordered_map< int, std::unordered_set<OrderClient*> > quantileToOrdersMap_;

    explicit Quantiles(double forecast);

    [[nodiscard]] inline int mostFilledQuantile() const {
        return std::max_element(begin(quantilePopulation_), end(quantilePopulation_)) -
        begin(quantilePopulation_);
    };

    void updateForecast();
    void updateQuantiles(); // recreate a new unordered_map
};

class RandomOrderGeneratorClient{
    std::unordered_map<std::string, OrderClient*> activeOrdersMap_; // internalID, Order object

};


#endif //ORDERBOOK_RANDOMCLIENT_H
