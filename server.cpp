// server.cpp

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include "board.h"
#include "computer.h"  // Jeœli potrzebny – w trybie turowym gracz serwera gra rêcznie

using namespace std;
using boost::asio::ip::tcp;

// ===============================
// Sekcja dla klasycznego serwera (broadcast, handle_client, run_server)
// ===============================

// Globalna lista wskaŸników na obiekty socketów klientów
std::vector<tcp::socket*> clients;
std::mutex clients_mutex;

// Funkcja rozsy³aj¹ca wiadomoœæ do wszystkich pod³¹czonych klientów poza Ÿród³em
void broadcast(const std::string& message, tcp::socket* source) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (auto client : clients) {
        if (client != source) {
            boost::system::error_code ignored_ec;
            boost::asio::write(*client, boost::asio::buffer(message), ignored_ec);
        }
    }
}

// Funkcja obs³uguj¹ca pojedynczego klienta (wywo³ywana w osobnym w¹tku)
void handle_client(tcp::socket* socket) {
    try {
        boost::asio::streambuf buffer;
        while (true) {
            boost::system::error_code error;
            // Odczytujemy dane a¿ do znaku nowej linii ("\n")
            std::size_t len = boost::asio::read_until(*socket, buffer, "\n", error);
            if (error) {
                std::cerr << "Blad odczytu: " << error.message() << std::endl;
                break;
            }

            std::istream is(&buffer);
            std::string line;
            std::getline(is, line);
            if (line.empty())
                continue;

            std::cout << "Otrzymano ruch: " << line << std::endl;
            // Rozsy³amy odebran¹ wiadomoœæ do innych klientów
            broadcast(line + "\n", socket);
        }
    }
    catch (std::exception& e) {
        std::cerr << "Wyjatek w obsludze klienta: " << e.what() << std::endl;
    }

    // Po roz³¹czeniu klienta usuwamy jego socket z listy
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        auto it = std::find(clients.begin(), clients.end(), socket);
        if (it != clients.end()) {
            clients.erase(it);
        }
    }
    socket->close();
    delete socket;
}

// Funkcja uruchamiaj¹ca serwer LAN (tryb broadcast)
void run_server() {
    try {
        boost::asio::io_context io_context;
        // Nas³uchujemy na porcie 5000
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5000));
        std::cout << "Serwer uruchomiony, nasluchiwanie na porcie 5000..." << std::endl;

        // Nieskoñczona pêtla przyjmuj¹ca nowe po³¹czenia
        while (true) {
            tcp::socket* socket = new tcp::socket(io_context);
            acceptor.accept(*socket);

            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.push_back(socket);
            }

            // Uruchamiamy obs³ugê klienta w osobnym w¹tku
            std::thread(handle_client, socket).detach();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Wyjatek serwera: " << e.what() << std::endl;
    }
}

// ===============================
// Funkcja do gry LAN w trybie turowym (pojedynczy klient)
// ===============================

void playTurnBasedLanServer() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 5000));
        std::cout << "Oczekiwanie na polaczenie klienta..." << std::endl;

        tcp::socket socket(io_context);
        acceptor.accept(socket);
        std::cout << "Klient podlaczony!" << std::endl;

        // Inicjalizacja planszy – standardowy uk³ad startowy
        ChessBoard board;
        board.from_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

        // £añcuch promocji – jeœli ruch ma 5 znaków, ostatni oznacza promocje
        std::string promotion_board = " KPNBRQ  kpnbrq";

        while (true) {
            // --- TURA SERWERA (gracz1) ---
            board.visualise();
            std::cout << "Twoj ruch: ";
            std::string move_str;
            std::cin >> move_str;

            int promotion = 0;
            if (move_str.size() == 5)
                promotion = (int)promotion_board.find(move_str[4]);

            auto piece = board.get_piece_type(((int)move_str[1] - '1') * 8 + ((int)move_str[0] - 'a'));
            move_t move = board.encode_move(
                (piece_no_color_t)(((int)piece) % 8),
                ((int)move_str[1] - '1') * 8 + ((int)move_str[0] - 'a'),
                ((int)move_str[3] - '1') * 8 + ((int)move_str[2] - 'a'),
                (piece_t)promotion);

            std::vector<move_t> moves = board.generate_moves();
            if (std::find(moves.begin(), moves.end(), move) == moves.end()) {
                std::cout << "Nielegalny ruch. Sprobuj ponownie." << std::endl;
                continue;
            }
            board.move(move);

            // Po ruchu serwera – jeœli ju¿ brak legalnych ruchów, serwer wygrywa
            if (board.generate_moves().empty()) {
                board.visualise();
                std::cout << "Gra zakonczona. Wygrywasz!" << std::endl;
                std::string msg = "LOSE: Gra zakonczona. Przegrywasz.\n";  // klient otrzyma komunikat, ¿e przegra³
                boost::asio::write(socket, boost::asio::buffer(msg));
                break;
            }

            // Wysy³amy aktualny stan planszy (FEN) do klienta
            std::string fen = board.get_fen();
            fen += "\n";
            boost::asio::write(socket, boost::asio::buffer(fen));

            // --- TURA KLIENTA (gracz2) ---
            bool legalMoveReceived = false;
            while (!legalMoveReceived) {
                boost::asio::streambuf buf;
                boost::asio::read_until(socket, buf, "\n");
                std::istream is(&buf);
                std::string client_move_str;
                std::getline(is, client_move_str);
                if (client_move_str.empty())
                    continue;

                promotion = 0;
                if (client_move_str.size() == 5)
                    promotion = (int)promotion_board.find(client_move_str[4]);

                auto client_piece = board.get_piece_type(((int)client_move_str[1] - '1') * 8 +
                    ((int)client_move_str[0] - 'a'));
                move_t client_move = board.encode_move(
                    (piece_no_color_t)(((int)client_piece) % 8),
                    ((int)client_move_str[1] - '1') * 8 + ((int)client_move_str[0] - 'a'),
                    ((int)client_move_str[3] - '1') * 8 + ((int)client_move_str[2] - 'a'),
                    (piece_t)promotion);

                moves = board.generate_moves();
                if (std::find(moves.begin(), moves.end(), client_move) == moves.end()) {
                    std::cout << "Ruch klienta nielegalny!" << std::endl;
                    std::string errorMsg = "ERR: Nielegalny ruch, prosze sprobuj ponownie.\n";
                    boost::asio::write(socket, boost::asio::buffer(errorMsg));
                    continue;
                }
                legalMoveReceived = true;
                board.move(client_move);
            }

            // Po ruchu klienta – jeœli brak legalnych ruchów, oznacza to zwyciêstwo klienta
            if (board.generate_moves().empty()) {
                board.visualise();
                std::cout << "Gra zakonczona. Przegrywasz!" << std::endl;
                std::string winMsg = "WIN: Gra zakonczona. Wygrywasz.\n";
                boost::asio::write(socket, boost::asio::buffer(winMsg));
                break;
            }
        }
    }
    catch (std::exception& e) {
        std::cerr << "Wyjatek w funkcji playTurnBasedLanServer: " << e.what() << std::endl;
    }
}


