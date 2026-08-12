#include "../lib/bin/src/server.cpp"
#include "../lib/bin/easy.h"

using namespace asio::ip;
using Gmeng::network_message;

int main(int argc, char** argv) {
    patch_argv_global(argc, argv);
    Gmeng::global.log_stout = true;

    auto conn = Gmeng::GameClient::connect("127.0.0.1", 7388);

    if (!conn.connected) {
        gm_log("could not connect");
        return 1;
    };

    conn.write("first_message", [&](network_message& message) {
        std::cout << "response1: " << message.content << '\n';

        conn.write("second_message", [&](network_message& message) {
            std::cout << "response2: " << message.content << '\n';
        });
    });

    conn.poll();

    conn.disconnect();
};
