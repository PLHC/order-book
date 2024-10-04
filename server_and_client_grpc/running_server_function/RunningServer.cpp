#include "RunningServer.h"

void RunServer(Market *market, const std::string& serverAddress) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

    grpc::ServerCompletionQueue *mainCompletionQueue = builder.AddCompletionQueue().release();

    std::unordered_map<std::string, std::vector<std::string>> orderBookVector;

    RpcServiceAsync service(mainCompletionQueue, market);
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    std::thread main_thread([&]() {
        service.handleRpcs();
    });

    // Joining threads
    if (main_thread.joinable()) {
        main_thread.join();
    }
    std::cout<<"out of joining thread in runserver"<<std::endl;
//    std::this_thread::sleep_for(std::chrono::seconds(10));

    mainCompletionQueue->Shutdown();
    server->Shutdown();

    std::cout<<"server is shutdown"<<std::endl;

}