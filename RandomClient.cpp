#include "RandomClient.h"

Quantiles::Quantiles(double forecast)
    : forecast_(forecast),
      quantilePopulation_(),
      quantileToOrdersMap_(){
    quantilePopulation_.fill(0);
}

