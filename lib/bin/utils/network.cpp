#pragma once

#include <string>
#if _WIN32 == false
#include <curl/curl.h>
#endif
#include <cstring>
#include <unistd.h>
#if _WIN32 == false
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <functional>
#include <unordered_map>
#include <chrono>
#include <map>
#include "../gmeng.h"
#include <vector>
#include <sstream>

/// the default port used by gmeng, default is 7388.
#define GMENG_DEFAULT_PORT 7388


// client prequisites
struct prequisite_client {
    char* IP_ADDRESS;
    int req_id = g_mkid();
};


/// checks if a port is available, without disrupting
/// operations using SO_REUSEADDR, which does not cast TIME_WAIT
/// after the port is closed.
bool check_port_availability(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return false;
    }

    // Allow immediate reuse of the port
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    // Try to bind the socket to the given port
    bool available = ::bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0;

    // Close the socket
    close(sock);

    return available;
};


long long current_time_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch())
        .count();
}

// Stream handler function type definition
struct stream_util;

using stream_handler_func = std::function<void(std::string&, long long, stream_util&)>;

#define streamstr(x) std::to_string(current_time_ms()) + "|" + x


// Stream utility struct
struct stream_util {
    // Client file descriptor for the connection
    int client_fd;
    // path of the host's server
    std::string host;
    // port of the host's server
    int port;
    // prequisites of the client
    prequisite_client prequisites;

    // default constructor
    stream_util(int fd, const std::string& h, int p, prequisite_client pr) : client_fd(fd), host(h), port(p), prequisites(pr) {};

    // Method to send data to the stream
    void send_data(const std::string& data) {
        send(client_fd, data.c_str(), data.size(), 0);
    }
};


/// CLIENT
/// ONLY FOR REQUESTS, NOT USED BY SERVER


struct server_t {
    std::string address;
    int port = 7388;
};

struct response_t {
    long status_code;
    std::string body;
    int ping = 0;
};

// Callback function to write received data
size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

// Function to send an HTTP GET request using libcurl
response_t send_request(const std::string& url, const std::string& authorization = "NONE") {
#if _WIN32 == false
    __functree_call__(send_request);
    CURL* curl;
    CURLcode res;
    struct response_t response;

    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set headers
        struct curl_slist* headers = NULL;
        if (!authorization.empty()) {
            std::string auth_header = "Authorization: " + authorization;
            headers = curl_slist_append(headers, auth_header.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set callback function to receive data
        std::string data;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Perform the request
        auto tm1 = GET_TIME();
        res = curl_easy_perform(curl);
        response.ping = GET_TIME() - tm1;
        // Get the response code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);

        // Check for errors
        if (res != CURLE_OK) {
            response.status_code = -1; // Return -1 for request error
            response.body = "ERROR: " + std::string(curl_easy_strerror(res));
        } else {
            response.body = data;
        }

        // Clean up
        curl_easy_cleanup(curl);
        if (headers)
            curl_slist_free_all(headers);
    } else {
        response.status_code = -2; // Return -2 for initialization error
        response.body = "ERROR: Failed to initialize libcurl";
    }

    return response;
#endif
};

/// OVERLOAD for post requests
response_t send_request(const std::string& url, const std::string& authorization = "NONE", const std::string& method = "POST", const std::string& body = "") {
#if _WIN32 == false
    __functree_call__(Gmeng::send_request::overload::POST_BODY);
    CURL* curl;
    CURLcode res;
    struct response_t response;

    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Set headers
        struct curl_slist* headers = NULL;
        if (!authorization.empty() && authorization != "NONE") {
            std::string auth_header = "Authorization: " + authorization;
            headers = curl_slist_append(headers, auth_header.c_str());
        }
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        // Set request method and body for POST
        if (method == "POST") {
            curl_easy_setopt(curl, CURLOPT_POST, 1L);
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
            headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        }

        // Set callback function to receive data
        std::string data;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        // Perform the request
        auto tm1 = GET_TIME();
        res = curl_easy_perform(curl);
        response.ping = GET_TIME() - tm1;
        // Get the response code
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response.status_code);

        // Check for errors
        if (res != CURLE_OK) {
            response.status_code = -1; // Return -1 for request error
            response.body = "ERROR: " + std::string(curl_easy_strerror(res));
        } else {
            response.body = data;
        }

        // Clean up
        curl_easy_cleanup(curl);
        if (headers)
            curl_slist_free_all(headers);
    } else {
        response.status_code = -2; // Return -2 for initialization error
        response.body = "ERROR: Failed to initialize libcurl";
    }

    return response;
#endif
};


/// BASIC SERVER

// Define enum for path types
enum class path_type_t {
    GET,
    POST
};

// Define request and response types
struct request {
  using string = std::string;
  typedef struct { string address; int id; } remote_t;

  public:
    string method; string path; string body;
    remote_t remote;
};

struct response {
    int status_code;
    std::string body;
};

// Server class definition
class gmserver_t {
public:
    int port; int server_fd;
    std::atomic<bool> exited;
    std::atomic<bool> running;

    gmserver_t() = default;
    gmserver_t(int port) : port(port), server_fd(-1), running(false) {
        __functree_call__(gmserver_t::__constructor__::gmserver_t);
    };

    void run() {
#if _WIN32 == false
        __functree_call__(gmserver_t::run);
        // Create socket
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            gm_log( "Error creating socket");
            return;
        };

        int opt = 1;
        /// for reusable sockets, to prevent kernel quitting the program because
        /// the program is attempting to load a TIME_WAIT port (if another program recently used the port)
        int sb = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (sb < 0) gm_log("set sockopt failed, if the port is in TIME_WAIT this server  won't start\n");

        // Prepare sockaddr_in structure
        sockaddr_in address;
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        // Bind socket to address
        // C++ is retarded, bind defaults to std::bind even if the std namespace isn't being used
        int bind_result = ::bind(server_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address));
        if (bind_result != 0) {
            gm_log( "Bind failed");
            close(server_fd);
            return;
        };

        // Listen for incoming connections
        if (listen(server_fd, 10) < 0) {
            gm_log( "Listen failed");
            close(server_fd);
            return;
        };

        gm_slog(Gmeng::YELLOW, "SERVER_"+v_str(port), "Server running on port " + v_str(port));

        running = true;

        /// while the server is running / atomic bool
        while (this->running.load()) {
            // Accept incoming connection
            sockaddr_in client_addr;
            socklen_t client_len = sizeof (client_addr);

            int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

            if (client_fd < 0) {
                gm_slog(Gmeng::YELLOW, "SERVER_"+v_str(port), "accept request failed");
                continue;
            }

            char ipaddr[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), ipaddr, INET_ADDRSTRLEN);

            prequisite_client client_data = {
                .IP_ADDRESS = ipaddr,
            };

            // Handle client request
            handle_request(client_fd, client_data);

            // Close client connection
            close(client_fd);
        };

        this->exited = true;
#endif
    }

    void create_path(path_type_t type, std::string path, std::function<void(request&, response&)> func) {
        __functree_call__(gmserver_t::create_path);
        if (type == path_type_t::GET) {
            get_paths[path] = func;
        } else if (type == path_type_t::POST) {
            post_paths[path] = func;
        }
    }

    void create_stream_path(path_type_t type, std::string path, stream_handler_func handler) {
        if (type == path_type_t::GET) get_stream_paths.insert_or_assign(path, handler);
        else post_stream_paths.insert_or_assign(path, handler);
    };

    void stop() {
        __functree_call__(gmserver_t::stop);
        this->running = false;

        ::shutdown(server_fd, SHUT_RDWR);
        ::close(server_fd);

        this->server_fd = -1;
        this->exited = true;
        this->exited = true;
    }

private:
    void handle_request(int client_fd, prequisite_client& prequisites) {
#ifndef _WIN32
        __functree_call__(gmserver_t::__private__::handle_request);

        const int max_length = 1024;
        char buffer[max_length] = {0};
        int valread = read(client_fd, buffer, max_length - 1);

        if (valread <= 0) {
            gm_slog(Gmeng::YELLOW, "SERVER_"+v_str(port), "Error reading request");
            return;
        }

        // Parse HTTP request
        request req = parse_request(buffer, prequisites);

        // Prepare HTTP response
        response res;
        res.status_code = 201;
        /// whether or not the request is to start a stream.
        std::string stream_prefix = "/&stream";

        // on_request event listener can be added here
        //std::cout << req.path << " " << req.body << " " << req.remote.address << '\n';

        if (req.method == "GET" && req.path.starts_with(stream_prefix)) {
            auto stream_path = "/" + req.path.substr(stream_prefix.size());
            if (get_stream_paths.find(stream_path) != get_stream_paths.end()) {
                handle_stream(client_fd, prequisites, stream_path, get_stream_paths[stream_path]);
                return;
            }
        } else if (req.method == "POST" && req.path.starts_with(stream_prefix)) {
            auto stream_path = "/" + req.path.substr(stream_prefix.size());
            if (post_stream_paths.find(stream_path) != post_stream_paths.end()) {
                handle_stream(client_fd, prequisites, stream_path, post_stream_paths[stream_path]);
                return;
            }
        }

        // Normal request handling
        if (req.method == "GET" && get_paths.count(req.path)) {
            get_paths[req.path](const_cast<request&>(req), res);
        } else if (req.method == "POST" && post_paths.count(req.path)) {
            post_paths[req.path](const_cast<request&>(req), res);
        } else {
            res.status_code = 404;
            res.body = "Not Found";
        }

        send_response(client_fd, res);
#endif
    };

    std::string stream_initconn = "INITCONN";

    void handle_stream(int client_fd, prequisite_client prequisites, const std::string& path, stream_handler_func handler_func) {
#ifndef _WIN32
        char buffer[1024];
        stream_util util(client_fd, path, this->port, prequisites);

        handler_func(stream_initconn, 0, util);

        while (this->running.load()) {
            int read_bytes = read(client_fd, buffer, sizeof(buffer) - 1);
            if (read_bytes <= 0) break;
            buffer[read_bytes] = '\0';

            std::string data(buffer);
            auto delimiter_pos = data.find('|');
            if (delimiter_pos != std::string::npos) {
                long long client_timestamp = std::stoll(data.substr(0, delimiter_pos));
                long long latency = current_time_ms() - client_timestamp;

                std::string message = data.substr(delimiter_pos + 1);

                if (message == "&endstream") break;

                handler_func(message, latency, util);

            } else {
                std::string response = streamstr("&nack.body");
                send(client_fd, response.c_str(), response.size(), 0);
            };
        };
#endif
    }

    request parse_request(const char* buffer, prequisite_client prequisites) {
        __functree_call__(gmserver_t::__private__::parse_request);
#if _WIN32 == false
        request req;

        // Read method
        const char* end_method = strchr(buffer, ' ');
        if (!end_method) {
            gm_slog(Gmeng::YELLOW,"SERVER_"+v_str(port),"Invalid request format");
            return req;
        }
        req.method.assign(buffer, end_method - buffer);

        // Read path
        const char* start_path = end_method + 1;
        const char* end_path = strchr(start_path, ' ');
        if (!end_path) {
            gm_slog(Gmeng::YELLOW, "SERVER_"+v_str(port), "Invalid request format");
            return req;
        }
        req.path.assign(start_path, end_path - start_path);

        // Read body if present
        const char* body_start = strstr(buffer, "\r\n\r\n");
        if (body_start) {
            req.body = body_start + 4; // Skip "\r\n\r\n"
        };

        req.remote = { prequisites.IP_ADDRESS, prequisites.req_id };

        return req;
#endif
    }

    void send_response(int client_fd, response& res) {
#if _WIN32 == false
        __functree_call__(gmserver_t::__private__::send_response);
        std::string response_str = "HTTP/1.1 " + std::to_string(res.status_code) + " OK\r\n";
        response_str += "Content-Length: " + std::to_string(res.body.size()) + "\r\n";
        response_str += "\r\n"; // End of headers
        response_str += res.body;
        send(client_fd, response_str.c_str(), response_str.size(), 0);
#endif
    }

    std::unordered_map<std::string, std::function<void(request&, response&)>> get_paths;
    std::unordered_map<std::string, std::function<void(request&, response&)>> post_paths;

    /// stream paths
    std::unordered_map<std::string, stream_handler_func> get_stream_paths;
    std::unordered_map<std::string, stream_handler_func> post_stream_paths;
};


struct GM_SVHOLD {
    std::string value;
    std::map<std::string, std::string> params;
};

std::map<std::string, GM_SVHOLD> _vget_sv_data(const std::string& input) {
    __functree_call__(_vget_sv_data);
    std::map<std::string, GM_SVHOLD> result;

    std::istringstream iss(input);
    std::string line;

    std::string current_tag;
    bool in_tag = false;

    while (std::getline(iss, line)) {
        size_t start_pos = line.find('<');
        size_t end_pos = line.find('>');
        if (start_pos != std::string::npos && end_pos != std::string::npos) {
            std::string tag_content = line.substr(start_pos + 1, end_pos - start_pos - 1);

            // Check if it's a closing tag
            if (tag_content[0] == '/') {
                std::string closing_tag = tag_content.substr(1);
                if (closing_tag == current_tag) {
                    current_tag.clear();
                    in_tag = false;
                }
                continue;
            }

            // Extract tag name
            size_t space_pos = tag_content.find(' ');
            std::string tag_name = (space_pos == std::string::npos) ? tag_content : tag_content.substr(0, space_pos);

            // Remove attributes if present
            if (space_pos != std::string::npos) {
                tag_content = tag_content.substr(space_pos + 1);
            } else {
                tag_content.clear();
            }

            // Remove closing '/'
            if (!tag_content.empty() && tag_content.back() == '/') {
                tag_content.pop_back();
            }

            // Check if the tag is open or closed
            if (tag_content.empty()) {
                current_tag = tag_name;
                in_tag = true;
            } else {
                // Extract tag content and attributes
                size_t attr_start_pos = 0;
                while (attr_start_pos < tag_content.size()) {
                    size_t eq_pos = tag_content.find('=', attr_start_pos);
                    if (eq_pos == std::string::npos) break;

                    std::string attr_name = tag_content.substr(attr_start_pos, eq_pos - attr_start_pos);
                    if (attr_name.back() == ' ') attr_name.pop_back();
                    if (attr_name.empty()) continue;

                    attr_start_pos = eq_pos + 1;

                    char quote = tag_content[attr_start_pos];
                    if (quote != '"' && quote != '\'') continue;

                    size_t end_pos = tag_content.find(quote, attr_start_pos + 1);
                    if (end_pos == std::string::npos) break;

                    std::string attr_value = tag_content.substr(attr_start_pos + 1, end_pos - attr_start_pos - 1);
                    if (attr_value.empty()) continue;

                    result[tag_name].params[attr_name] = attr_value;
                    attr_start_pos = end_pos + 1;
                }
            }
        } else {
            if (in_tag) {
                result[current_tag].value += line;
            }
        }
    }

    return result;
}

#ifndef __GMENG_MODELRENDERER__INIT__
    #include "../src/renderer.cpp"
#endif

namespace Gmeng::Networking {
    namespace conversion {
        Renderer::drawpoint c_drawpoint(std::string val) {
            //__functree_call__(Gmeng::Networking::conversion::c_drawpoint);
            auto v = g_splitStr(val, ",");
            return { std::stoi(v[0]), std::stoi(v[1]) };
        };
        Renderer::viewpoint c_viewpoint(std::string val) {
            //__functree_call__(Gmeng::Networking::conversion::c_viewpoint);
            auto v = g_splitStr(val, "|");
            return { c_drawpoint(v[0]), c_drawpoint(v[1]) };
        };
    };

    /// definition for rcon server
    struct rcon_server_def_t {
        int port = 7389; std::string password = v_str(g_mkid());

        bool allow_multiple_clients = false;
        bool unsafe_responses = false;

        bool enabled = false;
    };
};



#ifndef _WIN32
int send_stream_request(path_type_t method, const std::string& full_path, stream_handler_func handler_func) {
    size_t protocol_end = full_path.find("://") + 3;
    size_t host_end = full_path.find('/', protocol_end);
    std::string host_port = full_path.substr(protocol_end, host_end - protocol_end);
    std::string path = full_path.substr(host_end);

    // Find the port
    size_t colon_pos = host_port.find(':');
    std::string host = host_port.substr(0, colon_pos);
    int port = std::stoi(host_port.substr(colon_pos + 1));  // Extract port number

    // Create socket
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        std::cout << "Error opening socket\n";
        return 1;
    }

    struct hostent* server = gethostbyname(host.c_str());
    if (server == nullptr) {
        std::cerr << "Error resolving host\n";
        close(client_fd);
        return 2;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    memcpy(&server_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Connect to the server
    if (connect(client_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection failed.\n";
        close(client_fd);
        return 3;
    }

    // Prepare the HTTP request
    std::string request_str = std::string(method == path_type_t::GET ? "GET" : "POST") + " " + path + " HTTP/1.1\r\n";
    request_str += "Host: " + host + "\r\n";
    request_str += "Connection: close\r\n";
    request_str += "\r\n";

    // Send the HTTP request to the server
    if (send(client_fd, request_str.c_str(), request_str.size(), 0) < 0) {
        std::cerr << "Error sending HTTP request\n";
        close(client_fd);
        return 4;
    };

    // Prepare the stream initialization message with timestamp
    std::string message = "INITCONN"; // Stream initialization command
    std::string message_with_timestamp = streamstr(message);
    // Send the stream initialization message
    if (send(client_fd, message_with_timestamp.c_str(), message_with_timestamp.size(), 0) < 0) {
        std::cerr << "Error sending stream message\n";
        close(client_fd);
        return 5;
    }

    stream_util util(client_fd, host, port, *new prequisite_client);

    // Read server response
    char buffer[1024];
    while (true) {
        int read_bytes = read(client_fd, buffer, sizeof(buffer) - 1);
        if (read_bytes <= 0) {
            std::cerr << "Server closed connection or error occurred\n";
            break;
        }
        buffer[read_bytes] = '\0';

        // Process received data and calculate latency
        std::string data(buffer);
        auto delimiter_pos = data.find('|');
        if (delimiter_pos != std::string::npos) {
            long long server_timestamp = std::stoll(data.substr(0, delimiter_pos));
            long long latency = current_time_ms() - server_timestamp;

            std::string message = data.substr(delimiter_pos + 1);

            if (message == "&endstream") break;

            // Call the handler function with the message, latency, and stream_util
            handler_func(message, latency, util);
        } else {
            util.send_data(streamstr("unparsable data / ignored"));
        };
    }

    // Close the connection
    close(client_fd);
    return 0;
}
#endif
