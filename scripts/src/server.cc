#include "../lib/bin/src/server.cpp"
#include "../lib/bin/gmeng.h"
#include <memory>

using namespace Gmeng;

extern "C" int gmeng_script_command(EventLoop* ev) {

    Gmeng::create_thread([ev]() {
        GAME_LOG("initialized server thread");
        GAME_LOG("launching server");

        /// Allocate on heap via unique_ptr inside the thread context.
        /// Even if create_thread copies the outer wrapper, the server object
        /// cannot be cloned because unique_ptr guarantees a single source of truth.
        auto server = std::make_unique<GameServer>(7388, 10);

        /// Fetch a raw tracking pointer to safely pass down into the callback scope.
        GameServer* raw_server_ptr = server.get();

        auto on_message = [ev, raw_server_ptr](GameServer* s, GameServer::ClientMessage& message) {
            GAME_LOG("message to server: " + message.content);
            message.respond("server received: " + message.content + "!");
            if (message.content == "stop") {
                GAME_LOG("stop command received (" + v_str(s->active) + ")");
				s->active = false;
                return;
            }
            
            if (message.content.starts_with("!")) {
            	gmeng_run_dev_command(ev, message.content.substr(1));
            };
        };

        /// Execution blocks here
        server->run(on_message);

        GAME_LOG("Server exited");

        /// reset the server unique_ptr
        server.reset();

        GAME_LOG("unique_ptr to GameServer dropped (0)");
    });

    return 0;
};




