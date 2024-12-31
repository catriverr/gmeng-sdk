#include <csignal>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unordered_map>
#include "../lib/bin/gmeng.h"
#include "../lib/bin/utils/network.cpp"


void server(int argc, char** argv) {
    gmserver_t server(7300);
    std::string server_pass = "hithere";

    std::unordered_map<int, bool> gate;
    std::unordered_map<int, bool> await_gate;

    signal(SIGINT, [](int sig) {
        std::cout << "quit\n";
    });

    server.create_stream_path(path_type_t::GET, "/test",
    [&](std::string& command, long long latency, stream_util& util) {
        std::cout << "received msg from client("$(util.client_fd)"): " << command << ".\n";

        if (command == "&ack" || command == "&nack") return;

        if ( !await_gate.contains(util.client_fd) ) {
            util.send_data( streamstr("&echo welcome to the server\n") );
            await_gate.insert_or_assign(util.client_fd, false);
            sleep(ms(100));
        };

        if ( !gate.contains(util.client_fd) ) {
            if (command.starts_with("&input_response ")) {
                std::string pswd = command.substr(16);
                if (pswd != server_pass) gate.insert_or_assign(util.client_fd, false);
                else gate.insert_or_assign(util.client_fd, true);

                util.send_data(streamstr("&echo login status:" + ( gate.at(util.client_fd) ? "authorized" : "unauthorized" ) + "\n"));
                sleep(ms(100));
                util.send_data( streamstr(std::string(gate.at(util.client_fd) ? "&auth_ok" : "&auth_nok" )));
                return;
            };

            if ( !await_gate.at(util.client_fd) ) util.send_data( streamstr("&!input_request password") );
            else util.send_data(streamstr("&nack -- log in first. send creditentials with &input_response <passkey>\n"));

            await_gate.insert_or_assign(util.client_fd, true);
            return;
        } else {
            if (command == "&delog") {
                gate.erase(util.client_fd);
                await_gate.erase(util.client_fd);
                util.send_data( streamstr("&retry") );
                return;
            };
            if (gate.at(util.client_fd) == false) {
                util.send_data(streamstr("&nack -- user unauthorized"));
                return;
            };

            if (command == "ping") util.send_data( streamstr("&echo upstream latency: "$(latency)"\n") );
            sleep(ms(10));
            util.send_data(streamstr("&ack -- authorized"));
            return;
        };
        util.send_data(streamstr("&ack"));
    });

    server.run();
};

void client(int argc, char** argv) {
    bool done = false;
    send_stream_request(path_type_t::GET, "http://localhost:7300/&streamtest",
    [&](std::string& command, long long latency, stream_util& util) {
        std::unordered_map<std::string, bool> nolog = {
            { "&echo", true }, { "&input_request", true },
            { "&!input_request", true }
        };
        if (!nolog.contains(g_splitStr(command, " ")[0]) && command.starts_with("&")) std::cout << command << '\n';
        if (command == "&retry") {
            util.send_data( streamstr("INITCONN") );
            return;
        } else if (command.starts_with("&echo ")) {
            std::string msg = command.substr(6);
            std::cout << "(" << latency << ") remote | " << msg;
            return;
        } else if (command.starts_with("&input_request") || command.starts_with("&!input_request")) {
            bool secret = command.starts_with("&!");

            std::string prompt = command.substr(14 + (int)secret).length() < 2 ? "input" : command.substr(15);
            std::cout << "remote | " << prompt << " >> ";

            std::string input = lineinput(secret);
            util.send_data( streamstr("&input_response " + input) );
            return;
        } else if (command == "&auth_ok") {
            done = true;
            util.send_data( streamstr("&sserver") );
            return;
        } else if (command == "&auth_nok") {
            done = false;
            util.send_data(streamstr("&delog"));
            return;
        } else if (done) {
            std::cout << ":";
            std::string input;
            std::getline(std::cin, input);

            if (input.starts_with("clear")) {
                fputs("\033c", stdout);
            };


            util.send_data( streamstr(input) );
            return;
        };
    });
};

int main(int argc, char** argv) {
    if (argc > 1 && strcmp(argv[1], "server") == 0) server(argc, argv);
    else if (argc > 1 && strcmp(argv[1], "client") == 0) client(argc, argv);
    else std::cout << "usage: " << argv[0] << " <server/client>\n";
    return 0;
};
