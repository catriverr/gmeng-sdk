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
#endif
#include <functional>
#include <unordered_map>
#include <chrono>
#include <map>
#include "../gmeng.h"
#include <vector>
#include <sstream>

#define time_rn std::chrono::system_clock::now().time_since_epoch()
#define GET_TIME() ( std::chrono::duration_cast<std::chrono::milliseconds>(time_rn).count() )



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
response_t send_request(const std::string& url, const std::string& authorization = "NONE", const std::string& method = "GET", const std::string& body = "") {
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
    std::string method;
    std::string path;
    std::string body;
};

struct response {
    int status_code;
    std::string body;
};

// Server class definition
class gmserver_t {
public:
    gmserver_t(int port) : port(port), server_fd(-1), running(false) {
        __functree_call__(gmserver_t::gmserver_t);
    };

    void run() {
#if _WIN32 == false
        __functree_call__(gmserver_t::run);
        // Create socket
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1) {
            gm_log( "Error creating socket");
            return;
        }

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

        while (true) {
            // Accept incoming connection
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) {
                gm_slog(Gmeng::YELLOW, "SERVER_"+v_str(port), "accept request failed");
                continue;
            }

            // Handle client request
            handle_request(client_fd);

            // Close client connection
            close(client_fd);
        }
        running = true;
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

    void stop() {
        __functree_call__(gmserver_t::stop);
        running = false;
        if (server_fd != -1) {
            close(server_fd);
            server_fd = -1;
        }
    }

private:
    void handle_request(int client_fd) {
#if _WIN32 == false
        __functree_call__(gmserver_t::__private__::handle_request);
        const int max_length = 1024;
        char buffer[max_length] = {0};
        int valread = read(client_fd, buffer, max_length - 1);

        if (valread <= 0) {
            gm_slog(Gmeng::YELLOW, "SERVER_"+v_str(port), "Error reading request");
            return;
        }

        // Parse HTTP request
        request req = parse_request(buffer);

        // Prepare HTTP response
        response res;
        res.status_code = 200; // Default status code

        if (req.method == "GET" && get_paths.count(req.path)) {
            get_paths[req.path](req, res);
        } else if (req.method == "POST" && post_paths.count(req.path)) {
            post_paths[req.path](req, res);
        } else {
            res.status_code = 404;
            res.body = "Not Found";
        }

        // Send HTTP response
        send_response(client_fd, res);
#endif
    }

    request parse_request(const char* buffer) {
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
        }

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

    int port;
    int server_fd;
    std::atomic<bool> running;
    std::unordered_map<std::string, std::function<void(request&, response&)>> get_paths;
    std::unordered_map<std::string, std::function<void(request&, response&)>> post_paths;
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
};
