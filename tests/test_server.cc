#include "../lib/bin/src/server.cpp"
#include "../lib/bin/easy.h"

#include <system_error>

using namespace asio::ip;


void on_message( Gmeng::GameServer* server, Gmeng::GameServer::ClientMessage& message ) {
    std::cout << message.client->info_string() + ": " + message.content + "\n";

    std::error_code ec;
    message.respond("server received: " + message.content);
};

int main(int argc, char** argv) {
    patch_argv_global( argc, argv );
    vgm_load("envs/models");

    Gmeng::global.log_stout = true;
    Gmeng::GameServer server(7388, 10);

    server.run( on_message );
    return 0;
};
