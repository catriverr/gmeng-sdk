#include "../lib/bin/src/server.cpp"
#include "../lib/bin/easy.h"
#include <iostream>
#include <string>

using namespace asio::ip;
using Gmeng::network_message;


void process_input(Gmeng::ServerConnection& connection) {
    std::cout << ">> " << std::flush;

    std::string line;
    std::getline(std::cin, line);
    std::cout << "[client] sending command: " + line + "\n";
    connection.write(line, [](network_message& message) {
            std::cout << "\r[server] " << message.content << "\n>>" << std::flush;
    });
};

int main(int argc, char** argv) {
    patch_argv_global(argc, argv);
    Gmeng::global.log_stout = true;

    auto conn = Gmeng::GameClient::connect("127.0.0.1", 7388);

    if (!conn.connected) {
        gm_log("could not connect");
        return 1;
    };

    Gmeng::create_thread([&conn]() {
        conn.poll();
    });

    std::string line;
    std::cout << "gmeng test network client communicator\nenter commands to communicate with server\n\n";

    while (conn.connected) {
        process_input(conn);
    };
};
