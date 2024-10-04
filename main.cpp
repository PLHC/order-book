#include <iostream>
#include <thread>
#include "market/Market.h"
#include "server_and_client_grpc/running_server_function/RunningServer.h"
#include <csignal>
#include "server_and_client_grpc/Service/ServiceAsync.h"

std::unique_ptr<grpc::Server> server;
Market* tradingPlatform;
grpc::ServerCompletionQueue *mainCompletionQueue;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout<<"in signal handler"<<std::endl;
        tradingPlatform->setterStopFlagToTrue();
        std::cout<<"STOP flag is set"<<std::endl;
        server->Shutdown();
        std::cout<<"server is shutdown"<<std::endl;

        delete tradingPlatform;
        std::cout<<"trading Platform is destructed"<<std::endl;
        mainCompletionQueue->Shutdown();
        std::cout<<"CQ is shutdown"<<std::endl;
    }
}

int main(int argc, char *argv[]) {
    GeneratorId genID(0);
    tradingPlatform = new Market(&genID);

    for (int i = 1; i < argc; ++i) {
        tradingPlatform->createNewOrderBook(argv[i]);
    }

    grpc::ServerBuilder builder;
    builder.AddListeningPort("localhost:50051", grpc::InsecureServerCredentials());
    mainCompletionQueue = builder.AddCompletionQueue().release();
    RpcServiceAsync service(mainCompletionQueue, tradingPlatform);
    builder.RegisterService(&service);
    server = std::unique_ptr<grpc::Server>(builder.BuildAndStart());

    std::signal(SIGINT, signalHandler); // signal handler for CTRL+C

    std::thread main_thread([&]() {service.handleRpcs();} );

//    std::raise(SIGINT);

    // Joining threads
    if (main_thread.joinable()) {
        main_thread.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    return 0;
}
