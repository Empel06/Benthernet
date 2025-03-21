#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unordered_set>

int main() {
    zmq::context_t context{1};

    // PUSH socket voor het versturen van het verzoek
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    // SUB socket voor het ontvangen van het antwoord
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    std::string naam = "Emiel Mangelschots";
    std::string sub_topic = "service>subcounter!>" + naam + ">";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, sub_topic.c_str(), sub_topic.length());

    // Stuur subscribe-verzoek naar de subcounter-service
    std::string push_message = "service>subcounter?>" + naam + ">";
    std::cout << "Verzonden: " << push_message << std::endl;
    push_socket.send(zmq::buffer(push_message), zmq::send_flags::none);

    // Wacht op reactie
    while (true) {
        zmq::message_t reply;
        sub_socket.recv(reply, zmq::recv_flags::none);
        std::string antwoord = reply.to_string();

        std::cout << "Ontvangen reactie: " << antwoord << std::endl;
    }

    return 0;
}
