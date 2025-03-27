#include <string>
#include <iostream>
#include <zmq.hpp>
#include <vector>
#include <cstdlib>
#include <ctime>

// Struct om een basketbalwedstrijd te representeren
struct BasketbalWedstrijd {
    std::string team1;
    std::string team2;
    int score1;
    int score2;
};

// Lijst met EuroLeague teams
const std::vector<std::string> teams = {
    "Real Madrid", "FC Barcelona", "Olympiacos", "Panathinaikos", "Fenerbahce",
    "Anadolu Efes", "CSKA Moscow", "Maccabi Tel Aviv", "Baskonia", "Virtus Bologna",
    "Zalgiris Kaunas", "AS Monaco", "Partizan Belgrade", "Bayern Munich", "Alba Berlin",
    "Valencia Basket", "Crvena Zvezda", "LDLC ASVEL"
};

// Genereer een willekeurige basketbalwedstrijd
BasketbalWedstrijd genereerWedstrijd() {
    std::srand(std::time(nullptr));
    BasketbalWedstrijd wedstrijd;
    wedstrijd.team1 = teams[std::rand() % teams.size()];
    wedstrijd.team2 = teams[std::rand() % teams.size()];
    while (wedstrijd.team1 == wedstrijd.team2) {
        wedstrijd.team2 = teams[std::rand() % teams.size()];
    }
    wedstrijd.score1 = std::rand() % 100;
    wedstrijd.score2 = std::rand() % 100;
    return wedstrijd;
}

int main() {
    zmq::context_t context{1};

    // PUSH socket voor antwoorden
    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    // SUB socket om berichten te ontvangen
    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    // Luister op topic "service>basketbal?>"
    std::string topic = "service>basketbal?>";
    sub_socket.setsockopt(ZMQ_SUBSCRIBE, topic.c_str(), topic.length());

    std::cout << "Basketbalwedstrijd-service actief..." << std::endl;

    while (true) {
        zmq::message_t request;
        sub_socket.recv(request, zmq::recv_flags::none);

        std::string bericht = request.to_string();
        std::cout << "Verzoek ontvangen: " << bericht << std::endl;

        // Genereer een willekeurige basketbalwedstrijd
        BasketbalWedstrijd wedstrijd = genereerWedstrijd();

        // Antwoordbericht opbouwen
        std::string antwoord = "service>basketbal!>" + wedstrijd.team1 + " " + std::to_string(wedstrijd.score1) + " - " +
                                std::to_string(wedstrijd.score2) + " " + wedstrijd.team2 + ">";
        push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);

        std::cout << "Wedstrijd verstuurd: " << antwoord << std::endl;
    }

    return 0;
}