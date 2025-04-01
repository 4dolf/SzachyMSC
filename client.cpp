// client.cpp

#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include "board.h"  // Plik powinien definiować klasę ChessBoard oraz metody: from_fen(), get_fen(), visualise(), generate_moves(), itd.

using namespace std;
using namespace boost::asio;
using ip::tcp;

void playTurnBasedLanClient(atomic<char>* fen) {
    try {
        io_context ioContext;
        tcp::resolver resolver(ioContext);
        auto endpoints = resolver.resolve("127.0.0.1", "5000");
        tcp::socket socket(ioContext);
        connect(socket, endpoints);
        cout << "Polaczono z serwerem." << endl;

        ChessBoard board;

        while (true) {
            boost::asio::streambuf buf;
            boost::asio::read_until(socket, buf, "\n");
            istream is(&buf);
            string message;
            getline(is, message);

            if (message.empty())
                continue;

            // Rozróżniamy odebrane komunikaty:
            // Komunikat błędu
            if (message.rfind("ERR:", 0) == 0) {
                cout << "Serwer: " << message << endl;
                cout << "Twoj ruch: ";
                string new_move;
                cin >> new_move;
                new_move += "\n";
                boost::asio::write(socket, boost::asio::buffer(new_move));
                continue;
            }
            // Komunikat o przegranej klienta (czyli serwer wygrał)
            else if (message.rfind("LOSE:", 0) == 0) {
                cout << "Serwer: " << message << endl;
                // Klient wie, że przegrał – kończy rozgrywkę
                break;
            }
            // Komunikat o zwycięstwie klienta
            else if (message.rfind("WIN:", 0) == 0) {
                cout << "Serwer: " << message << endl;
                break;
            }
            else {
                // Jeśli to nie jest specjalny komunikat, to traktujemy wiadomość jako ciąg FEN
                board.from_fen(message);
                string fen_string = board.get_fen();
                for (int i = 0; i < fen_string.size(); i++)
                    fen[i] = fen_string[i];

                fen[fen_string.size()] = '\n';
                board.visualise();
            }

            // Sprawdzamy, czy gra zakończona (brak legalnych ruchów)
            if (board.generate_moves().empty()) {
                cout << "Gra zakonczona." << endl;
                break;
            }

            cout << "Twoj ruch: ";
            string move_str;
            cin >> move_str;
            move_str += "\n";
            boost::asio::write(socket, boost::asio::buffer(move_str));
        }
    }
    catch (std::exception& e) {
        cerr << "Wyjatek w kliencie: " << e.what() << endl;
    }
}
