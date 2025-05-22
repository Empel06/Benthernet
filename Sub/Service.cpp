#include <zmq.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <ctime>
#include <cstdlib>
#include <random>
#include <cmath>
#include <sstream>
#include <algorithm>

// ============================ BasketbalGame ============================
std::pair<std::string, int> parse_basketbal_message(const std::string& message) {
    size_t naam_start = message.find("?>");
    if (naam_start == std::string::npos) return {"", -1};
    size_t naam_end = message.find(">", naam_start + 2);
    if (naam_end == std::string::npos) return {"", -1};
    std::string naam = message.substr(naam_start + 2, naam_end - (naam_start + 2));

    size_t kracht_start = naam_end + 1;
    size_t kracht_end = message.find(">", kracht_start);
    if (kracht_end == std::string::npos) return {"", -1};
    int kracht = std::stoi(message.substr(kracht_start, kracht_end - kracht_start));

    return {naam, kracht};
}

// ============================ BasketbalMachine ============================
std::string extract_name(const std::string& message) {
    size_t pos = message.find("?>");
    if (pos == std::string::npos) return "";
    std::string part = message.substr(pos + 2);
    size_t end = part.find(">");
    return part.substr(0, end);
}

std::vector<std::string> basketbal_acties = {"DRIBBLE", "PASS", "SHOOT", "DUNK", "STEAL"};

std::vector<std::string> spin_basketbal_machine() {
    std::vector<std::string> result;
    for (int i = 0; i < 3; ++i) {
        int index = rand() % basketbal_acties.size();
        result.push_back(basketbal_acties[index]);
    }
    return result;
}

std::string bepaal_basketbal_winst(const std::vector<std::string>& result, int& winst) {
    if (result[0] == result[1] && result[1] == result[2]) {
        winst = 10;
        return "🏆 Perfecte play! Je wint 10 punten!";
    } else if (result[0] == result[1] || result[1] == result[2] || result[0] == result[2]) {
        winst = 3;
        return "🔥 Net play! Je wint 3 punten!";
    } else {
        winst = 0;
        return "😢 Geen score deze ronde.";
    }
}

// Maak leaderboard string van de punten map
std::string create_leaderboard_message(const std::unordered_map<std::string, int>& punten) {
    // Sorteer op score desc
    std::vector<std::pair<std::string, int>> lijst(punten.begin(), punten.end());
    std::sort(lijst.begin(), lijst.end(), [](auto& a, auto& b) { return a.second > b.second; });

    std::ostringstream oss;
    oss << "Emiel>Leaderboard!>";
    for (auto it = lijst.begin(); it != lijst.end(); ++it) {
        oss << it->first << ":" << it->second;
        if (std::next(it) != lijst.end()) oss << "|";
    }
    return oss.str();
}

int main() {
    zmq::context_t context{1};

    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    // Subscribe naar alle relevante topics
    sub_socket.set(zmq::sockopt::subscribe, "Emiel>BasketbalGame?>");
    sub_socket.set(zmq::sockopt::subscribe, "Emiel>BasketbalMachine?>");

    std::unordered_map<std::string, int> punten;
    srand(static_cast<unsigned int>(time(nullptr)));

    std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> kracht_dis(1, 10);

    std::cout << "🎮 BasketbalService gestart...\n";

    while (true) {
        zmq::message_t msg;
        if (!sub_socket.recv(msg, zmq::recv_flags::none)) continue;

        std::string bericht = msg.to_string();
        std::cout << "[Ontvangen] " << bericht << std::endl;

        bool punten_gewijzigd = false;

        // BasketbalGame: kracht vergelijken
        if (bericht.rfind("Emiel>BasketbalGame?>", 0) == 0) {
            auto [naam, kracht] = parse_basketbal_message(bericht);
            if (naam.empty() || kracht < 1 || kracht > 10) continue;

            int doel_kracht = kracht_dis(gen);
            std::string antwoord;

            if (std::abs(kracht - doel_kracht) <= 1) {
                punten[naam] += 1;
                antwoord = "Emiel>BasketbalGame!>" + naam + ">Score! Je hebt een punt gescoord. Totaal: " +
                           std::to_string(punten[naam]) + " punten.";
                punten_gewijzigd = true;
            } else {
                antwoord = "Emiel>BasketbalGame!>" + naam + ">Mis! Doelkracht was " +
                           std::to_string(doel_kracht) + ". Je blijft op " +
                           std::to_string(punten[naam]) + " punten.";
            }

            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
            std::cout << "[BasketbalGame] " << naam << ": kracht=" << kracht << ", doel=" << doel_kracht << "\n";

        // BasketbalMachine: acties spinnen
        } else if (bericht.rfind("Emiel>BasketbalMachine?>", 0) == 0) {
            std::string naam = extract_name(bericht);
            if (naam.empty()) continue;

            if (punten[naam] < 2) {
                std::string fout = "Emiel>BasketbalMachine!>" + naam + ">Niet genoeg punten om te draaien (min. 2 nodig). Je hebt " +
                                   std::to_string(punten[naam]) + " punten.";
                push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
                std::cout << "[BasketbalMachine] " << naam << ": onvoldoende punten\n";
                continue;
            }

            punten[naam] -= 2;

            auto resultaat = spin_basketbal_machine();
            int winst = 0;
            std::string feedback = bepaal_basketbal_winst(resultaat, winst);
            punten[naam] += winst;

            std::string spins = resultaat[0] + " - " + resultaat[1] + " - " + resultaat[2];
            std::string antwoord = "Emiel>BasketbalMachine!>" + naam + ">Spin: " + spins + " => " + feedback +
                                   " (-2 inzet, +" + std::to_string(winst) + " winst). Totaal: " +
                                   std::to_string(punten[naam]) + " punten.";

            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
            std::cout << "[BasketbalMachine] " << naam << ": " << spins << " => " << feedback << "\n";
            punten_gewijzigd = true;
        }

        // Als punten zijn gewijzigd, stuur nieuw leaderboard uit
        if (punten_gewijzigd) {
            std::string leaderboard_msg = create_leaderboard_message(punten);
            push_socket.send(zmq::buffer(leaderboard_msg), zmq::send_flags::none);
            std::cout << "[Leaderboard] Verzonden leaderboard update\n";
        }
    }

    return 0;
}
