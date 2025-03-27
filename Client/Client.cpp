#include <string>
#include <iostream>
#include <zmq.hpp>

int main() {
    zmq::context_t context{1};

    // PUSH socket voor het versturen van het verzoek
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    // SUB socket voor het ontvangen van de wedstrijdgegevens
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    // Juiste topic instellen voor subscription
    std::string sub_topic = "service>basketbal!>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, sub_topic.c_str(), sub_topic.length());

    // Zolang de client actief is, blijven verzoeken sturen
    while (true) {
        std::cout << "Druk op ENTER om een nieuw verzoek te sturen..." << std::endl;
        std::cin.get(); // Wacht op ENTER toets

        // Stuur verzoek voor een basketbalwedstrijd
        std::string push_message = "service>basketbal?>";
        std::cout << "Verzoek verstuurd: " << push_message << std::endl;
        push_socket.send(zmq::buffer(push_message), zmq::send_flags::none);

        // Wacht op reactie
        zmq::message_t reply;
        sub_socket.recv(reply, zmq::recv_flags::none);
        std::string wedstrijd = reply.to_string();

        std::cout << "Ontvangen wedstrijd: " << wedstrijd << std::endl;
    }

    return 0;
}
