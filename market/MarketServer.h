//
// Created by Paul  on 12/08/2024.
//

#ifndef ORDERBOOK_MARKETSERVER_H
#define ORDERBOOK_MARKETSERVER_H

#include <grpcpp/grpcpp.h>
#include "MarketAccess.grpc.pb.h"
#include "MarketAccess.pb.h"
#include <chrono>
#include <thread>

class CommunicationImplementation final : public marketAccess::Communication::Service {
public:
    ::grpc::Status DisplayRequest(::grpc::ServerContext* context,
                                  const ::marketAccess::DisplayParameters* request,
                                  ::marketAccess::OrderBookContent* response) override;
    ::grpc::Status DeleteRequest(::grpc::ServerContext* context,
                                 const ::marketAccess::DeletionParameters* request,
                                 ::marketAccess::Confirmation* response) override;
    ::grpc::Status InsertionRequest(::grpc::ServerContext* context,
                                    const ::marketAccess::InsertionParameters* request,
                                    ::marketAccess::Confirmation* response) override;
    ::grpc::Status UpdateRequest(::grpc::ServerContext* context,
                                 const ::marketAccess::UpdateParameters* request,
                                 ::marketAccess::Confirmation* response) override;
};

//void RunMarketServer();


#endif //ORDERBOOK_MARKETSERVER_H
