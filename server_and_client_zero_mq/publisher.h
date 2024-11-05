#ifndef ORDERBOOK_PUBLISHER_H
#define ORDERBOOK_PUBLISHER_H

#include <zmq.h>
#include <string>
#include <iostream>
#include <cstring>
#include <unistd.h> // For sleep

void PublishOrderUpdates(void* publisher) {
    int update_count = 0;
    while (update_count < 10) {  // Limit to 10 updates for now
        std::string order_id = "ORD" + std::to_string(update_count);
        std::string action = (update_count % 3 == 0) ? "EXECUTE" : (update_count % 3 == 1 ? "UPDATE" : "DELETE");
        std::string message = "Order " + order_id + " Action: " + action;

        zmq_msg_t zmq_message;
        zmq_msg_init_size(&zmq_message, message.size());
        memcpy(zmq_msg_data(&zmq_message), message.c_str(), message.size());

        zmq_send(publisher, &zmq_message, zmq_msg_size(&zmq_message), 0); // size added
        zmq_msg_close(&zmq_message); // Free the message
        std::cout << "Published: " << message << std::endl;

        sleep(1); // Simulate delay between updates
        update_count++;
    }
}

//int main() {
//    // Create a ZeroMQ context
//    void* context = zmq_ctx_new();
//
//    // Create a PUB socket
//    void* publisher = zmq_socket(context, ZMQ_PUB);
//
//    // Bind to a TCP endpoint
//    zmq_bind(publisher, "tcp://*:5555");
//
//    std::cout << "Publisher started, sending order updates..." << std::endl;
//    PublishOrderUpdates(publisher);
//
//    // Clean up
//    zmq_close(publisher);
//    zmq_ctx_destroy(context);
//    return 0;
//}

#endif //ORDERBOOK_PUBLISHER_H
