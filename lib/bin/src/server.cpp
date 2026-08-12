#pragma once


#include <array>
#include <chrono>
#include <cstdio>
#include <exception>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <map>
#include <vector>

#include "../utils/log.h"
#include "../utils/functree.h"
#include "../utils/tracefunc.h"
#include "../utils/commandset.h"

#include <asio/asio.hpp>


namespace Gmeng {
    class GameServer; /// type declaration

    struct network_message {
        int id;
        int response_of = -1;
        std::string content;
    };

    static constexpr std::string network_message_delimeter = "\x0f\x1f";
    static constexpr std::string network_message_splitter = "\x2f\x3f";

    namespace {
        network_message parse_regular( const std::string& message, std::vector<std::string> split ) {
            try {
                int id = std::stoi( split.at(0) );
                /// delete the first item (message_id)
                split.erase( split.begin() );
                /// if by some unlikely, basically impossible combination of events,
                /// some wicked, crack-addled developer sends the bytes '\x0f\x1f'
                /// through a stream, we make sure that the bytes are re-added
                /// when joining the message string.
                return { id, -1, g_joinStr( split, network_message_delimeter ) };
            } catch (std::exception& e) {
                /// holy shit, we're catching an error and throwing it back. this is cursed.
                throw std::runtime_error("network_message_read_error: cannot parse message_id: " + (std::string)e.what());
            };
        };

        network_message parse_response( const std::string& message, std::vector<std::string> split ) {
            if ( !split.at(0).starts_with("R") )
                throw std::invalid_argument("network_message_read_error: cannot parse response: " + message);
            if ( split.size() < 3 )
                throw std::invalid_argument(
                        "network_message_read_error: response " + message +
                        "is too short. format is: R<original_message_id>,<message_id>,<message_content>"
                );
            /// Format should be:
            /// R<original_message_id>,<message_id>,<message_content>
            try {
                /// remove the 'R' response prefix and get the original id.
                int original_id = std::stoi( split.at(0).substr(1) );
                /// get the ID of the response message
                int message_id = std::stoi( split.at(1) );

                split.erase( split.begin(), split.begin()+2 );

                return { message_id, original_id, g_joinStr( split, network_message_delimeter ) };
            } catch (std::exception& e) {
                /// again, catching an error just to throw it again. cursed.
                throw std::runtime_error("network_message_read_error: cannot parse message_id: " + (std::string)e.what());
            };
        };
    };

    /// serializes a network message
    std::string network_serialize_message( const std::string& message, int message_id, int response_of = -1 ) {
        std::string id = v_str(message_id);

        if ( response_of != -1 ) return "R" + v_str(response_of) + network_message_delimeter + id + network_message_delimeter + message;
        else return id + network_message_delimeter + message + network_message_splitter;
    };

    network_message network_parse_message( const std::string& message ) {
        auto split = g_splitStr( message, network_message_delimeter );
        if (split.size() < 2) throw std::runtime_error( " network_message_read_error: unable to read message: " + message );

        /// R -> response. format:
        /// R<original_message_id><delimeter><message_id><delimeter><message_content>
        if ( split.at(0).starts_with("R") ) return parse_response( message, split );
        else return parse_regular( message, split );
    };

    Gmeng::GameServer* game_server = nullptr;
    /// Since 13.2.0: Gmeng Networking with Asio
    /// Game Server for Multiplayer game instances.
    class GameServer {
      public:
        /// Client ID type ( aka unsigned int )
        using client_id = unsigned int;
        /// Client instance for the Server.
        /// Contains its client_id, IP address,
        /// connection date and num_packets.
        struct ServerClient {
            /// Socket with the client.
            std::shared_ptr<asio::ip::tcp::socket> socket;
            /// Number of packets sent by the client.
            unsigned int num_packets = 0;
            /// Time (in date milliseconds) of connection by the client.
            long long connected_at = GET_TIME();
            /// ID of the client. Unique for all clients.
            /// See `g_mkid()` for ID creation logic.
            client_id id = -1;
            /// IP address (in string form) of the client.
            std::string address = "unknown";
            /// Sends a message to the client.
            /// set `response_of` to a valid message sent by the client to send a response,
            /// instead of a brand new message. This will fire the `response` callback of that message (client-side).
            void send_message( std::string message, int response_of = -1, int id = g_mkid() ) {
                gm_log("sending message: " + message);
                /// if the socket isn't open, don't write to it.
                if (!this->socket->is_open()) return;
                /// serialize the message for the Gmeng websocket data protocol
                auto serialized_message = Gmeng::network_serialize_message( message, id, response_of );
                std::error_code ec;
                /// wait until the socket is read to write to.
                this->socket->wait(this->socket->wait_write);
                asio::write( *this->socket, asio::buffer( serialized_message ), ec );
                if (ec) gm_log("network_message_send_error: " + ec.message());
            };

            std::string info_string() {
                return v_str(this->id) + " @ " + this->address +
                " (connected_at:" + v_str(this->connected_at) +
                " | num_packets: "+ v_str(this->num_packets) +")";
            };
        };
        /// Message from a connected client.
        /// Contains client information as well its message.
        struct ClientMessage {
            /// ID of the message.
            unsigned int id = g_mkid();
            /// Client information.
            ServerClient* client;
            /// contents of the message.
            std::string content = "";
            /// method to respond to the message.
            void respond( std::string reply_text ) {
                this->client->send_message( reply_text, this->id );
                this->client->socket->wait(this->client->socket->wait_write);
            };
        };
        /// TCP server port of the server. Gmeng internally sets this to 7388.
        /// See `GMENG_DEFAULT_PORT` in `networking.cc` for more information.
        unsigned int port = 7388;
        /// maximum number of allowed clients into
        /// the GameServer instance. default is 10.
        unsigned int max_clients = 8;
        /// Asio IO context of the server.
        /// Handles connectivity as well as the socket streaming.
        asio::io_context context;
        /// Asio TCP connection acceptor of the server.
        /// Accepts incoming clients and manages them.
        asio::ip::tcp::acceptor acceptor;
        /// Whether the server is running. When set to false,
        /// the server will close all active connections and exit.
        bool active = true;
        /// default in-game chat stream.
        std::stringstream chat;
        /// Clients connected to the server.
        std::map<client_id, ServerClient> clients;

        GameServer( int _port = 7388, int _max_clients = 10 )
        : port ( _port ), max_clients( _max_clients ),
        acceptor( context, asio::ip::tcp::endpoint( asio::ip::tcp::v4(), port ) ) {
            __functree_call__( Gmeng::GameServer::constructor );
            /// Multiple server instances in a single
            /// program is not supported, error out and exit.
            if ( Gmeng::game_server != nullptr ) {
                gm_log("gmeng::game_server is not nullptr, exitting...");
                throw std::runtime_error("a Gmeng::GameServer instance is already present.");
            };

            gm_log("created a game server instance for port "$(this->port)".");
            /// set the current server to this instance
            Gmeng::game_server = this;
        };

        /// Launches the server. This will create a while loop
        /// with a callback to the parameter `user_callback`. Best
        /// to run in its own thread, and use the main program
        /// thread only for the game updates.
        void run( std::function<void( GameServer*, ClientMessage& )> user_callback ) {
            gm_log("Server at port "$(this->port)" is alive");

            this->acceptor.non_blocking(true);

            while ( this->active ) {
                using asio::ip::tcp;
                /// attempt to create a new socket (if there are incoming connections)
                auto new_socket = std::make_shared<tcp::socket>( this->context );

                std::error_code accept_ec;
                auto try_accept = this->acceptor.accept(*new_socket, accept_ec );

                if ( !accept_ec ) {
                    /// Cannot accept because the server is full.
                    if ( this->clients.size() >= this->max_clients ) {
                        std::string response = Gmeng::network_serialize_message("err server_full", g_mkid());
                        asio::write( *new_socket, asio::buffer( response ), accept_ec );
                        new_socket->close();
                    } else {
                        new_socket->non_blocking(true);

                        auto endpoint = new_socket->remote_endpoint();
                        auto ip_addr = endpoint.address().to_string();

                        ServerClient new_client { new_socket, 0, GET_TIME(), (unsigned int)g_mkid(), ip_addr };
                        this->clients[ new_client.id ] = new_client;
                        /// set the client id by sending the new ID to the client.
                        new_client.send_message("set_id " + v_str( new_client.id ));
                    };
                } else {
                    /// no clients requesting to join right now.
                };

                /// Poll connected clients
                for (auto it = this->clients.begin(); it != this->clients.end(); ) {
                    char data[1024]; /// Buffer for incoming TCP stream chunk
                    std::error_code read_ec;

                    /// read_some grabs whatever is currently in the OS buffer
                    size_t length = it->second.socket->read_some(asio::buffer(data), read_ec);

                    if (read_ec == asio::error::would_block || read_ec == asio::error::try_again) {
                        /// no data available to read right now, keep the connection alive
                        ++it;
                    }
                    else if (read_ec || length == 0) {
                        // client disconnected (EOF) or a socket error occurred
                        it->second.socket->close();
                        gm_log("client ("$(it->second.id)") disconnected (EOF). connection time: "
                                    $(GET_TIME() - it->second.connected_at)", total packages: "
                                    $(it->second.num_packets)", address: " +
                                    it->second.address);
                        it = this->clients.erase(it);
                    }
                    else {
                        /// increment packet count
                        it->second.num_packets++;
                        DEBUGGER gm_log("message from client("$(it->second.id)":" + it->second.address + "): " + std::string(data, length));

                        /// Messages can be sent too consecutively. To prevent this, when serializing messages
                        /// the gmeng websocket data protocol adds the `network_message_splitter` bytes to the
                        /// end of the message. this way, if they are written over each other too quickly, the
                        /// protocol will split the commands by these bytes and interpret them individually.
                        auto all_messages = g_splitStr(std::string(data, length), network_message_splitter);

                        for ( auto __message: all_messages ) {
                            if ( __message.empty() ) continue;
                            DEBUGGER gm_log("running next command in buffer: " + __message);
                            ClientMessage msg;
                            msg.client = &it->second;
                            auto v = network_parse_message(__message);
                            msg.id = v.id; msg.content = v.content;
                            /// callback with the message
                            user_callback(this, msg);
                            ++it;
                        };
                    }
                }
            };
            /// this->active has to be false, the while loop scope is closed.
            /// disconnect all clients. grace period of 300ms so other operations
            /// are closed.
            std::this_thread::sleep_for( std::chrono::milliseconds(300) );
            delete this;
        };


        ~GameServer() {
            gm_log("disconnecting all clients...");
            /// close all active connections
            for ( auto& client : this->clients ) {
                std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
                DEBUGGER gm_log("disconnecting client " + client.second.info_string());
                /// protocol_command
                client.second.send_message("disconnect");
                client.second.socket->close();
            };
        };
    };


    class GameClient; // type decl
    struct ServerConnection; // type decl
    namespace network_protocol {
        using std::string, std::vector;
        static CommandSet<int( ServerConnection*, vector<string> )> commands;
    };

    struct ServerConnection {
        std::shared_ptr<asio::io_context> context;
        std::shared_ptr<asio::ip::tcp::socket> socket;
        bool connected = false;
        unsigned int client_id = 0;

        std::function<void(network_message&)> message_handler = nullptr;


        /// global message handler. this function will handle all messages received
        /// by the client. this includes responses to messages sent by the client as well.
        void handle_message( decltype(message_handler) _message_handler ) {
            this->message_handler = _message_handler;
        };

        std::map<int, decltype(message_handler)> response_callbacks;

        /// Sends a message instantly. Doesn't block.
        /// If on_response is provided, it fires the next time the server sends data.
        void write(const std::string& message, decltype(message_handler) on_response = nullptr) {
            if (!connected || !socket) return;
            /// ID of the new message.
            int message_id = g_mkid();
            /// serialize the message.
            std::string serialized_message = Gmeng::network_serialize_message( message, message_id );

            std::error_code ec;
            /// Wait until the socket is writable.
            this->socket->wait(this->socket->wait_write);
            /// asio::write blocks ONLY until the OS buffer accepts the bytes (usually instantaneous),
            /// it does not wait for the network or the server.
            asio::write(*socket, asio::buffer(serialized_message), ec);
            /// if there is a response callback, register it to the client.
            if ( on_response != nullptr ) this->response_callbacks[ message_id ] = on_response;
            /// if there is an error, log it.
            if (ec) {
                gm_log("error writing: " + ec.message());
                return;
            };
        }

        /// Handles Gmeng websocket data protocol messages.
        void handle_protocol( network_message& message ) {
            if ( message.response_of == -1 ) {
                auto spl = g_splitStr( message.content, " " );
                auto command = spl.at(0);
                spl.erase( spl.begin() );
                try {
                    if (network_protocol::commands.commands.contains(command)) {
                        DEBUGGER gm_log("running protocol command: " + message.content + "...");
                        int return_value = network_protocol::commands.run( command, this, spl );
                        DEBUGGER gm_log("protocol command returned value: " + v_str( return_value ));
                    };
                } catch(std::exception& e) {
                    gm_log("error while running protocol command: " + (std::string)e.what());
                };
            };
        };

        /// creates a message loop. This is a blocking function, so
        /// it is recommended to run this method in a seperate thread.
        /// See `Gmeng::_ucreate_thread` for information about threading with Gmeng.
        void poll() {
            if (!connected || !socket) return;

            std::error_code ec;

            while ( this->connected ) {
                /// if the socket is closed, disconnect from the server.
                if ( !socket->is_open() ) {
                    gm_log("disconnected from server | ASSUME(socket->is_open() != 0) is false");
                    this->disconnect(); this->connected = false;
                    break;
                };
                /// Check if there is any data available to be read.
                if ( socket->available() > 0 ) {
                    /// wait until reading is possible.
                    socket->wait( socket->wait_read );
                    /// recall socket->available(). required, because socket->available()
                    /// will show only what is available at the instance of when it is called.
                    /// When this method is called every frame in an event loop, the time required
                    /// to receive the entire message is not available. So, we call
                    /// `socket->wait( socket->wait_read )` to wait until the buffer is complete for
                    /// reading. Afterwards, we call socket->available() again to check the updated
                    /// size of the buffer, and we are able to access the correct length of the message.
                    std::size_t bytes_available = socket->available();
                    /// create the readbuffer for the new data with a size of the available bytes.
                    std::vector<char> read_buffer(bytes_available);
                    /// Read the data on the buffer
                    socket->read_some( asio::buffer( read_buffer.data(), read_buffer.size() ), ec );
                    /// create the raw message string
                    std::string raw_message( read_buffer.data() );

                    /// Messages can be sent too consecutively. To prevent this, when serializing messages
                    /// the gmeng websocket data protocol adds the `network_message_splitter` bytes to the
                    /// end of the message. this way, if they are written over each other too quickly, the
                    /// protocol will split the commands by these bytes and interpret them individually.
                    auto all_messages = g_splitStr(raw_message, network_message_splitter);
                    DEBUGGER gm_log("received "$(all_messages.size())" messages in a single buffer");
                    for ( auto message : all_messages ) {
                        /// no text, probably a splitting error
                        if (message.empty()) continue;
                        /// Parse the message. Returns a message_id and message_content.
                        auto parsed = Gmeng::network_parse_message( message );
                        /// Log the received message ( if DEBUG mode is enabled `[program] -debug` )
                        DEBUGGER gm_log("received message " + parsed.content + " (resp: "$(parsed.response_of)"|"$(parsed.id)")");
                        /// handle the message (if it is part of the gmeng websocket data protocol).
                        /// `handle_protocol( ... )` only uses the message if it is part of the protocol.
                        this->handle_protocol( parsed );
                        /// hand off the message to the message handler (if there is one)
                        /// users can set the handler via `this->handle_message( ... )`.
                        if ( this->message_handler != nullptr ) this->message_handler( parsed );
                        /// Check if the message is a response ( response_of != =1 ) OR (edge-case)
                        /// if the response_callbacks method contains a callback awaiting this ID.
                        if ( this->response_callbacks.contains( parsed.response_of ) ) {
                            /// Log debug message about the callback existing.
                            DEBUGGER gm_log("firing callback for message "$(parsed.response_of)"...");
                            /// call the response callback.
                            this->response_callbacks.at( parsed.response_of )( parsed );
                            /// delete the callback (since it's no longer needed.)
                            this->response_callbacks.erase( parsed.response_of );
                        };
                    };
                } else {
                    /// no data right now.
                };
            };
        };

        void disconnect() {
            if (socket && connected) {
                std::error_code ec;
                auto err = socket->close(ec);
                connected = false;

                if ( err ) gm_log("(ignorable) error when closing socket: " + err.message() + ". caused by asio.");
            };
        }
    };


    /// Since 13.2.0: Gmeng Networking with Asio
    /// Game Client for Multiplayer game instances.
    class GameClient {
      public:


        static ServerConnection connect(const std::string& ip_address, unsigned int port = 7388, TRACEFUNC) {
            ServerConnection conn;
            conn.context = std::make_shared<asio::io_context>();
            conn.socket = std::make_shared<asio::ip::tcp::socket>(*conn.context);

            gm_log("attempting to connect to a Gmeng::GameServer at " + ip_address + ":"$(port)"... (from " + TRACEFUNC_STR + ")");

            std::error_code ec;
            asio::ip::tcp::endpoint endpoint(asio::ip::make_address(ip_address, ec), port);

            if (!ec) {
                (void)conn.socket->connect(endpoint, ec);
            } else gm_log("unable to connect: " + ec.message());

            if (!ec) {
                gm_log("successfully connected to " + ip_address + ":"$(port)".");
                conn.connected = true;
                // Set the client socket to non-blocking so read_some() doesn't freeze the game
                auto nb = conn.socket->non_blocking(true, ec);
            } else {
                conn.connected = false;
                auto close = conn.socket->close(ec);
            }

            return conn;
        }
    };
};

int set_network_protocol_commands() {
    using std::string, std::vector, Gmeng::GameClient;
    Gmeng::network_protocol::commands = Gmeng::CommandSet<int( Gmeng::ServerConnection*, vector<string> )>({
            { "set_id", [](Gmeng::ServerConnection* client, vector<string> args) -> int {
                if (args.size() < 1) throw std::invalid_argument("at least 1 argument for set_id is required");
                try {
                    client->client_id = std::stoi( args.at(0) );
                    gm_log("(Gmeng::ServerConnection*)->client_id set to: " + args.at(0));
                } catch(std::exception& e) {
                    throw std::invalid_argument("argument provided to set_id is invalid (requires set_id <number>)");
                };
                return client->client_id;
            } },
            { "disconnect", [](Gmeng::ServerConnection* client, vector<string>) -> int {
                gm_log("protocol: disconnect() requested");
                client->disconnect();
                return 0;
            } },
        });
    return 0;
};

static int set_network_protocol_commands_status = set_network_protocol_commands();
