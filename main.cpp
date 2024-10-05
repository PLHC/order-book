#include <iostream>
#include <thread>
#include "market/Market.h"
#include <csignal>
#include "server_and_client_grpc/Service/ServiceAsync.h"

std::unique_ptr<grpc::Server> server;
Market* tradingPlatform;
grpc::ServerCompletionQueue *mainCompletionQueue;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        tradingPlatform->setterStopFlagToTrue();
        server->Shutdown();
        delete tradingPlatform;
        mainCompletionQueue->Shutdown();
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

    if (main_thread.joinable()) {
        main_thread.join();
    }

    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout<<" main is done "<<std::endl;

    return 0;
}
