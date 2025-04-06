#include "../../lib/bin/strings/split.cc"
#include "../../lib/bin/strings/startswith.cc"
#include "../../lib/bin/strings/trim.cc"
#ifndef _WIN32
#include "../../lib/bin/easy.h" // gmeng import
#include "../../lib/bin/utils/network.cpp" // gmeng networking utility
#include <map>

/// GMENG INTERNAL SCRIPT FOR CLIENT-SIDE SERVER INSTANCES
/// gmeng 10.4.0
///
/// This utility provides the ability to connect to any gmeng instance
/// using an external application just by providing a port.
///
/// The method returns the port that the program will use, in case the default port is preoccupied.
/// The system will attempt to bind to the directory in 7388 to 7488, and will fail if none of the ports in
/// the area is available.


extern "C" int gmeng_script_command( Gmeng::EventLoop* ev ) {

    int use_port = GMENG_DEFAULT_PORT; unsigned int attempts = 0; bool failed = true;
    while ( (failed = !check_port_availability(use_port)) && attempts < 100 ) use_port++, attempts++; // until a port is open in the 7388-7488 range, decides fail at 100 attempts.
    if (failed) {
        gmeng_show_warning("port");
        ev->cancelled = true;
        return -1;
    };

    // check state path
    ev->server.create_path(path_type_t::GET, "/", [&](request& req, response& res) {
        res.status_code = 200;
        res.body = "alive";
    });

    // stop server path
    ev->server.create_path(path_type_t::POST, "/stop", [&](request&, response& res) {
        res.status_code =  200;
        res.body = "ok close";
        ev->server.stop();
    });

    std::map<int, bool> log_acceptance;
    ev->server.create_stream_path(path_type_t::POST, "/console",
    [&](std::string& command, long long latency, stream_util& util) -> void {
     if (trim(command).length() < 1) { util.send_data(streamstr("&nack")); return; };
     GAME_LOG("(rcon) " + std::string(util.prequisites.IP_ADDRESS) + " console >> " + command);
        std::string cmd = trim(command);
        if ( starts_with(command, "!login") ) {
            auto args = split_string(cmd, ' ');

            if (args.size() < 2) { util.send_data(streamstr("fail")); return; };

            std::string arg2 = args.at(1);

            try {
                if ( std::to_string(ev->modifiers.get_value("server_passkey")) != arg2 )
                {   util.send_data( streamstr("fail") );
                    return; };
            } catch (std::exception& e) { util.send_data(streamstr("fail")); return; };

            log_acceptance.insert_or_assign( util.client_fd, true );
            util.send_data( streamstr("pass") );

            return;
        };

        if ( starts_with(command, "!logoff") ) {
            log_acceptance.insert_or_assign( util.client_fd, false );
            util.send_data( streamstr("ok") );
            return;
        };

        if ( starts_with(command, "&") ) {
            if (!log_acceptance.contains(util.client_fd) || !log_acceptance.at(util.client_fd)) { util.send_data(streamstr("noauth")); return; };
            gmeng_run_dev_command(ev, command.substr(1), true);
            util.send_data( streamstr("run") );
            gmeng_run_dev_command(ev, "force_update", true);
            return;
        };
        if (command == "INITCONN") util.send_data( streamstr("this is a gmeng server instance running version " + gmeng_version
                                                        + ".\nlog in for authorization with `!login <passkey>`.") );
        else util.send_data(streamstr( (log_acceptance.contains(util.client_fd) && log_acceptance.at(util.client_fd) ? "&ack" : "noauth" )));
    });

    // start the server on the selected port
    ev->server.port = use_port;
    ev->init_server(true);


    return use_port;
};
#endif
