// client.cpp

#include <iostream>
#include <boost/asio.hpp>
#include <string>
#include "board.h"  // Plik powinien definiowaæ klasê ChessBoard oraz metody: from_fen(), get_fen(), visualise(), generate_moves(), itd.

using namespace std;
using namespace boost::asio;
using ip::tcp;

void playTurnBasedLanClient() {
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

            // Rozró¿niamy odebrane komunikaty:
            // Komunikat b³êdu
            if (message.rfind("ERR:", 0) == 0) {
                cout << "Serwer: " << message << endl;
                cout << "Twoj ruch: ";
                string new_move;
                cin >> new_move;
                new_move += "\n";
                boost::asio::write(socket, boost::asio::buffer(new_move));
                continue;
            }
            // Komunikat o przegranej klienta (czyli serwer wygra³)
            else if (message.rfind("LOSE:", 0) == 0) {
                cout << "Serwer: " << message << endl;
                // Klient wie, ¿e przegra³ – koñczy rozgrywkê
                break;
            }
            // Komunikat o zwyciêstwie klienta
            else if (message.rfind("WIN:", 0) == 0) {
                cout << "Serwer: " << message << endl;
                break;
            }
            else {
                // Jeœli to nie jest specjalny komunikat, to traktujemy wiadomoœæ jako ci¹g FEN
                board.from_fen(message);
                board.visualise();
            }

            // Sprawdzamy, czy gra zakoñczona (brak legalnych ruchów)
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
