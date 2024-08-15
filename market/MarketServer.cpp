//
// Created by Paul  on 12/08/2024.
//

#include "MarketServer.h"


::grpc::Status CommunicationImplementation:: DisplayRequest(::grpc::ServerContext* context,
                                                            const ::marketAccess::DisplayParameters* request,
                                                            ::marketAccess::OrderBookContent* response) {
    std::cout<<"Request received for "+request->product_id()+" "+request->requestnumber()<<std::endl;
    response->set_orderbook("Orderbook for "+request->product_id()+" "+request->requestnumber());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout<<"Sending response for "+request->product_id()+" "+request->requestnumber()<<std::endl;
    return grpc::Status();
}

::grpc::Status CommunicationImplementation:: DeleteRequest(::grpc::ServerContext* context,
                                                           const ::marketAccess::DeletionParameters* request,
                                                           ::marketAccess::Confirmation* response) {
    response->set_validation(true);
    response->set_boid(request->boid());
    response->set_comment("Order deleted successfully.");
    return grpc::Status();
}

::grpc::Status CommunicationImplementation:: InsertionRequest(::grpc::ServerContext* context,
                                                              const ::marketAccess::InsertionParameters* request,
                                                              ::marketAccess::Confirmation* response) {
    response->set_validation(true);
    response->set_boid(request->boid());
    response->set_comment("Order inserted successfully.");
    return grpc::Status();
}

::grpc::Status CommunicationImplementation:: UpdateRequest(::grpc::ServerContext* context,
                                                           const ::marketAccess::UpdateParameters* request,
                                                           ::marketAccess::Confirmation* response) {
    response->set_validation(true);
    response->set_boid(request->updatedorderid());
    response->set_comment("Order updated successfully.");
    return grpc::Status();
}
