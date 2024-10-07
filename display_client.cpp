#include "server_and_client_grpc/Client/DisplayClient/DisplayClient.h"
#include <csignal>

DisplayClient* marketDisplay;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        marketDisplay->setterStopFlagToTrue();
    }
}

int main(int argc, char *argv[]){
    // argv:
//    1: userID
//    2: nb of lines per product
//    3... : traded products
    std::vector<std::string> tradedProducts;
    for(int i = 3; i<argc; ++i){
        tradedProducts.emplace_back(argv[i]);
    }

    std::signal(SIGINT, signalHandler);

    marketDisplay = new DisplayClient(grpc::CreateChannel("localhost:50051", grpc::InsecureChannelCredentials() ),
                                      argv[1],
                                      tradedProducts,
                                      std::stoi(argv[2]) ,
                                      1); // 1 thread enough to process the display requests

}