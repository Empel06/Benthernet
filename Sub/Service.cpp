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
#include <cctype>

// ===================== HELPER FUNCTIES =====================

static inline void trim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline std::string to_lower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return s;
}

std::string clean_str(std::string s) {
    trim(s);
    return to_lower(s);
}

void print_string_chars(const std::string& label, const std::string& s) {
    std::cout << label << " (chars): ";
    for (char c : s) std::cout << int((unsigned char)c) << " ";
    std::cout << "\n";
}

// ===================== STRUCTS =====================

struct Match {
    std::string team1, team2;
    int score1, score2;
};

Match generate_random_match() {
    static std::vector<std::string> teams = {"Lakers", "Celtics"};
    static std::mt19937 gen(std::random_device{}());
    std::uniform_int_distribution<> team_dist(0, teams.size() - 1);
    std::uniform_int_distribution<> score_dist(80, 130);

    int t1 = team_dist(gen), t2;
    do { t2 = team_dist(gen); } while (t2 == t1);

    return {teams[t1], teams[t2], score_dist(gen), score_dist(gen)};
}

std::string bepaal_winnaar(const Match& m) {
    if (m.score1 > m.score2) return clean_str(m.team1);
    else if (m.score2 > m.score1) return clean_str(m.team2);
    else return "gelijkspel";
}

// ===================== PARSERS =====================

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

std::string extract_name(const std::string& message) {
    size_t pos = message.find("?>");
    if (pos == std::string::npos) return "";
    std::string part = message.substr(pos + 2);
    size_t end = part.find(">");
    return end == std::string::npos ? "" : part.substr(0, end);
}

std::string extract_guess_team(const std::string& message) {
    size_t naam_end = message.find(">", message.find("?>") + 2);
    if (naam_end == std::string::npos) return "";

    std::string rest = message.substr(naam_end + 1);
    trim(rest);
    if (!rest.empty() && rest.back() == '>') rest.pop_back();

    size_t space_pos = rest.find_first_of(" \t\r\n");
    return to_lower(space_pos != std::string::npos ? rest.substr(0, space_pos) : rest);
}

std::string extract_double_option(const std::string& message) {
    size_t naam_end = message.find(">", message.find("?>") + 2);
    if (naam_end == std::string::npos) return "";

    std::string rest = message.substr(naam_end + 1);
    trim(rest);
    size_t space_pos = rest.find_first_of(" \t\r\n");
    return (space_pos == std::string::npos) ? "" : to_lower(rest.substr(space_pos + 1));
}

std::string create_leaderboard_message(const std::unordered_map<std::string, int>& punten) {
    std::vector<std::pair<std::string, int>> lijst(punten.begin(), punten.end());
    std::sort(lijst.begin(), lijst.end(), [](auto& a, auto& b) {
        return a.second > b.second;
    });

    std::ostringstream oss;
    oss << "Emiel>Leaderboard!>";
    for (auto it = lijst.begin(); it != lijst.end(); ++it) {
        oss << it->first << ":" << it->second;
        if (std::next(it) != lijst.end()) oss << "|";
    }
    return oss.str();
}

// ===================== MAIN =====================

int main() {
    zmq::context_t context{1};

    zmq::socket_t push_socket{context, zmq::socket_type::push};
    push_socket.connect("tcp://benternet.pxl-ea-ict.be:24041");

    zmq::socket_t sub_socket{context, zmq::socket_type::sub};
    sub_socket.connect("tcp://benternet.pxl-ea-ict.be:24042");

    sub_socket.set(zmq::sockopt::subscribe, "Emiel>BasketbalGame?>");
    sub_socket.set(zmq::sockopt::subscribe, "Emiel>BasketbalMachine?>");
    sub_socket.set(zmq::sockopt::subscribe, "Emiel>DoubleOrNothing?>");

    std::unordered_map<std::string, int> punten;
    std::uniform_int_distribution<> kracht_dis(1, 10);
    std::mt19937 gen(std::random_device{}());

    std::cout << "BasketbalService gestart.\n";

    while (true) {
        zmq::message_t msg;
        if (!sub_socket.recv(msg, zmq::recv_flags::none)) continue;

        std::string bericht = msg.to_string();
        std::cout << "[Ontvangen] " << bericht << std::endl;

        bool punten_gewijzigd = false;

        // === BasketbalGame ===
        if (bericht.rfind("Emiel>BasketbalGame?>", 0) == 0) {
            auto [naam, kracht] = parse_basketbal_message(bericht);
            if (naam.empty() || kracht < 1 || kracht > 10) continue;

            int doel_kracht = kracht_dis(gen);
            std::string antwoord;
            if (std::abs(kracht - doel_kracht) <= 1) {
                punten[naam]++;
                antwoord = "Emiel>BasketbalGame!>" + naam + ">Score! Je hebt een punt gescoord. Totaal: " + std::to_string(punten[naam]) + " punten.";
                punten_gewijzigd = true;
            } else {
                antwoord = "Emiel>BasketbalGame!>" + naam + ">Mis! Doelkracht was " + std::to_string(doel_kracht) + ". Je blijft op " + std::to_string(punten[naam]) + " punten.";
            }
            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
        }
        // BasketbalMachine: gok op winnaar match
        else if (bericht.rfind("Emiel>BasketbalMachine?>", 0) == 0) {
            std::string naam = extract_name(bericht);
            std::string gok_team = extract_guess_team(bericht);

            if (naam.empty() || gok_team.empty()) continue;

            if (punten[naam] < 2) {
                std::string fout = "Emiel>BasketbalMachine!>" + naam + ">Niet genoeg punten om te gokken (min. 2 nodig). Je hebt " +
                                   std::to_string(punten[naam]) + " punten.";
                push_socket.send(zmq::buffer(fout), zmq::send_flags::none);
                std::cout << "[BasketbalMachine] " << naam << ": onvoldoende punten\n";
                continue;
            }

            // Punt inzet aftrekken
            punten[naam] -= 2;

            Match match = generate_random_match();

            // Grondig clean en lowercase van gok_team
            gok_team = clean_str(gok_team);

            std::string winnaar_norm = bepaal_winnaar(match);

            // Debugging prints inclusief char codes
            std::cout << "[DEBUG] Wedstrijd: " << match.team1 << " " << match.score1 << " - " << match.team2 << " " << match.score2 << "\n";
            std::cout << "[DEBUG] gok_team=>>" << gok_team << "<<, winnaar=>>" << winnaar_norm << "<<\n";
            print_string_chars("gok_team", gok_team);
            print_string_chars("winnaar_norm", winnaar_norm);

            std::string antwoord;

            if (winnaar_norm == gok_team) {
                // Win 5 punten
                punten[naam] += 5;
                antwoord = "Emiel>BasketbalMachine!>" + naam + ">Wedstrijd: " + match.team1 + " " + std::to_string(match.score1) +
                           " - " + match.team2 + " " + std::to_string(match.score2) + ". Je gokte correct! +5 punten. Totaal: " +
                           std::to_string(punten[naam]) + " punten.";
                punten_gewijzigd = true;
            } else {
                antwoord = "Emiel>BasketbalMachine!>" + naam + ">Wedstrijd: " + match.team1 + " " + std::to_string(match.score1) +
                           " - " + match.team2 + " " + std::to_string(match.score2) + ". Helaas, je gok was fout. Je verloor 2 punten inzet. Totaal: " +
                           std::to_string(punten[naam]) + " punten.";
                punten_gewijzigd = true;
            }

            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
            std::cout << "[BasketbalMachine] " << naam << ": gokte op " << gok_team << ", winnaar: " << winnaar_norm << "\n";
        }

        // === DoubleOrNothing ===
        else if (bericht.rfind("Emiel>DoubleOrNothing?>", 0) == 0) {
            std::string naam = extract_name(bericht);
            if (naam.empty() || punten[naam] < 50) continue;

            bool gewonnen = (rand() % 2 == 0);
            std::string resultaat = gewonnen ? "gewonnen" : "verloren";

            if (gewonnen) {
                punten[naam] *= 2;
            } else {
                punten[naam] = 0; // halveert bij verlies
            }

            std::string antwoord = "Emiel>DoubleOrNothing!>" + naam + ">" + resultaat + ">";
            push_socket.send(zmq::buffer(antwoord), zmq::send_flags::none);
            punten_gewijzigd = true;
        }

        // === Leaderboard Update ===
        if (punten_gewijzigd) {
            std::string lb_msg = create_leaderboard_message(punten);
            push_socket.send(zmq::buffer(lb_msg), zmq::send_flags::none);
            std::cout << "[Leaderboard] Verzonden.\n";
        }
    }

    return 0;
}
