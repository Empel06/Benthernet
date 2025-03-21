#include <string>
#include <iostream>
#include <zmq.hpp>
#include <unordered_set>

// Helperfunctie om naam uit het bericht te halen
std::string extract_name(const std::string& message) {
    // Verwacht formaat: service>subcounter?>[NAAM]>
    std::size_t pos = message.find("?>");
    if (pos != std::string::npos) {
        std::string part = message.substr(pos + 2);
        std::size_t end = part.find(">");
        return part.substr(0, end);
    }
    return "";
}

int main() {
    zmq::context_t context{1};

    // PUSH socket voor antwoorden
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    // SUB socket om berichten te ontvangen
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    // Luister op topic "service>subcounter?>"
    std::string topic = "service>subcounter?>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.length());

    std::cout << "Service actief: wacht op subscribers..." << std::endl;

    int subscriber_count = 0;
    std::unordered_set<std::string> geregistreerde_namen;

    while (true) {
        zmq::message_t request;
        sub_socket.recv(request, zmq::recv_flags::none);

        std::string bericht = request.to_string();
        std::string naam = extract_name(bericht);

        if (naam.empty()) {
            std::cerr << "Ongeldig bericht ontvangen: " << bericht << std::endl;
            continue;
        }

        if (geregistreerde_namen.find(naam) == geregistreerde_namen.end()) {
            // Nieuwe subscriber
            geregistreerde_namen.insert(naam);
            subscriber_count++;
        }

        // Antwoordbericht opbouwen
        std::string antwoord = "service>subcounter!>" + naam + ">je bent subscriber nummer: " + std::to_string(subscriber_count) + ">";
        push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);

        std::cout << "Nieuwe subscriber: " << naam << " (#" << subscriber_count << ")" << std::endl;
    }

    return 0;
}
