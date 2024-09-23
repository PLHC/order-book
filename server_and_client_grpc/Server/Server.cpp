#include "Server.h"

void RunServer(Market *market, const std::string& serverAddress) {
    grpc::ServerBuilder builder;
    builder.AddListeningPort(serverAddress, grpc::InsecureServerCredentials());

    grpc::ServerCompletionQueue *mainCompletionQueue = builder.AddCompletionQueue().release();

    std::unordered_map<std::string, std::vector<std::string>> orderBookVector;

    RpcService service(mainCompletionQueue, market);
    builder.RegisterService(&service);
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());

    std::thread main_thread([&]() {
        service.handleRpcs();
    });

    // Joining threads
    if (main_thread.joinable()) {
        main_thread.join();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

}