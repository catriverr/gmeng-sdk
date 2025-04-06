#include <cstring>
#include <iostream>
#include <sstream>

#define __GMENG_LOG_TO_COUT__ true

#include "../lib/bin/gmeng.h"
#include "../lib/bin/utils/serialization.cpp"




using namespace Gmeng;

#define GMENG_DEFAULT_PORT_1 7388



void server(int argc, char** argv) {
    gmserver_t server(GMENG_DEFAULT_PORT_1);

    server.create_path(path_type_t::GET, "/",
    [&](request& req, response& res) -> void {
        std::cout << ("REQUEST to '/' .. by " + req.remote.address) << '\n';
        res.status_code = 200;
        res.body = "hello, world!";
    });

    Level level;
    std::stringstream out;
    read_level_data("envs/proto_level.glvl", level);
    write_level_data(out, level);

    server.create_path(path_type_t::GET, "/level",
    [&](request& req, response& res) -> void {
        std::cout << ("REQUEST to '/level' .. by " + req.remote.address) << '\n';
        res.status_code = 200;
        res.body = out.str();
    });

    server.run();
};

void client(int argc, char** argv) {
#define LOG(x) std::cout << "[CLIENT] " << x << '\n'
    Level level;
    std::stringstream ss;
    response_t req = send_request("http://localhost:" + v_str(GMENG_DEFAULT_PORT_1) + "/level", "NONE", "GET");
    ss << req.body;
    read_level_data(ss, level);
    LOG("LEVEL file fetch took "$(req.ping)"ms");
    level.display.set_resolution(50, 25);
    level.display.viewpoint = { {0, 0}, {50, 25} };
    auto renderscale = get_renderscale(level);
    auto view = get_lvl_view(level, renderscale);
    emplace_lvl_camera(level, view);
    std::cout << level.display.camera.draw() << '\n';
};

int main(int argc, char** argv) {
    patch_argv_global(argc, argv);
    if ( argc < 2 || strcmp(argv[1], "server") != 0 ) client(argc, argv);
    else server(argc, argv);
};
