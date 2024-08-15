//
// Created by Paul  on 13/08/2024.
//

#ifndef ORDERBOOK_MARKETCLIENT_H
#define ORDERBOOK_MARKETCLIENT_H

#include <grpcpp/grpcpp.h>
#include "MarketAccess.grpc.pb.h"
#include "MarketAccess.pb.h"
#include <memory>

void DisplayRequest(std::shared_ptr<grpc::Channel> channel, const std::string & product);

void RunMarketClient(const std::string product);

#endif //ORDERBOOK_MARKETCLIENT_H


