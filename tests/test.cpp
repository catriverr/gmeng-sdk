#include <cstdint>
#if GMENG_SDL
#include "SDL2/SDL.h"
#endif
#include <chrono>
#include <ostream>
#define __GMENG_ALLOW_LOG__ true
#define __GMENG_LOG_TO_COUT__ true
#include <iostream>
#include <algorithm>
#include "../lib/bin/gmeng.h"
#include "../lib/bin/src/renderer.cpp"
#include <math.h>

#if GMENG_SDL
#include "../lib/bin/types/window.h"
#include "../lib/bin/utils/window.cpp"
#endif

#define g_sleep std::this_thread::sleep_for
#define ms std::chrono::milliseconds

using std::endl;

int test_loadtexture() {
    Gmeng::texture test; Gmeng::Camera<2, 2> wmap;
    test = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "g_test1");
    return 0;
};
int test_placement() {
    Gmeng::Renderer::drawpoint placement = {2, 3};  // Point where the object will be placed
    Gmeng::Renderer::drawpoint size = {4, 4};       // Size of the object
    Gmeng::Renderer::drawpoint worldSize = {10, 10};   // Size of the world map
    std::vector<Gmeng::Renderer::drawpoint> displacements = Gmeng::Renderer::get_placement(placement, size, worldSize);
    return 0;
};
int test_renderer() {
    return 0;
};
int test_loadglvl() {
    std::cout << "test_loadglvl job_start" << '\n';
    Gmeng::LevelInfo lv_inf = Gmeng::parse_glvl("envs/4.0_test.glvl");
    std::cout << "GLVL load successful\n";
    return 0;
};

int main1() {
    init_logc();
    gm::_uread_into_vgm("./envs/models");
    test_loadtexture();
    std::cout << "test_loadtexture -> status v_success" << endl;
    test_placement();
    std::cout << "test_placement -> status v_success" << endl;
    test_renderer();
    std::cout << "test_renderer -> status v_success" << endl;
    test_loadglvl();
    std::cout << "test_loadglvl -> status v_success" << endl;
    _gthread_catchup();
    return 0;
};

int test_vgmcontent() {
    std::cout << "vgm_defaults size: " << Gmeng::vgm_defaults::vg_textures.size() << endl;
    Gmeng::Camera<1, 1> camera;
    camera.SetResolution(1, 1);
        for (int j = 0; j < Gmeng::vgm_defaults::vg_textures.size(0); j++) {
        auto tx = Gmeng::vgm_defaults::vg_textures.indx(j);
        std::cout << "drawing texture: ";
        std::cout << tx.name << " | notxtr: " << (Gmeng::notxtr.name == tx.name ? "true" : "false");
        for (int i = 0; i < tx.units.size(); i++) {
            if ((i % tx.width) == 0) std::cout << '\n';
            std::cout << camera.draw_unit(tx.units[i]);
        };
        std::cout << '\n';
    };
    std::cout << '\n';
    return 0;
};

int test_caketxtr() {
    gm::texture tx = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "01_cake_txtr");
    Gmeng::Camera<1, 1> camera;
    std::cout << "drawing texture: ";
    std::cout << tx.name << " | notxtr: " << (Gmeng::notxtr.name == tx.name ? "true" : "false") << endl;
    camera.SetResolution(1, 1);
    for (int i = 0; i < tx.units.size(); i++) {
        if ( i % tx.width == 0 ) std::cout << '\n';
        std::cout << camera.draw_unit(tx.units[i]);
    };
    return 0;
};

int test_chunkvpoint() {
    Gmeng::Level level_test = {
        .base = {
            .lvl_template = gm::vd_find_texture(Gmeng::vgm_defaults::vg_textures, "allah"),
            .width = 88,
            .height = 44
        },
        .name = v_str(g_mkid())
    };
    Gmeng::Renderer::Model test_model = Gmeng::Renderer::generate_empty_model(10, 5);
    Gmeng::Renderer::Model test_model_2 = {
        .position = { 0, 6 }
    };
    test_model_2.attach_texture(gm::vd_find_texture(Gmeng::vgm_defaults::vg_textures, "01_cake_txtr"));
    test_model.name = v_str(g_mkid());
    test_model.position = { 0,0 };
    level_test.load_chunk({
        .vp = { { 0, 0 }, { 88, 22 } },
        .models = {
            test_model
        },
    });
    level_test.load_chunk({
        .vp = { { 0, 23 }, { 88, 44 } },
        .models = {
            test_model_2
        }
    });
    level_test.display.set_resolution(88, 22);
    level_test.draw_camera(0);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    level_test.draw_camera(1);
    std::cout << "starting test for __viewpoint_vector_move_camera__" << '\n';
    /// bit to check text_data
    gm::global.dev_console = true;
    _udraw_display(Gmeng::logc);
    gm::global.dev_console = false;
    return 0;
};

int test_vpointrender() {
    std::cout << "test 7 : deprecated test module \n";
    return 0;
};

int test_vwhole_renderer() {
    std::cout << "beginning test" << '\n';
    Gmeng::Level lvl = {
        .base = {
            .lvl_template = gm::vd_find_texture(Gmeng::vgm_defaults::vg_textures, "allah"),
            .width = 88,
            .height = 44
        },
        .name = v_str(g_mkid())
    };

    std::cout << "generate lvl" << '\n';
    Gmeng::Renderer::Model test_model = Gmeng::Renderer::generate_empty_model(5, 5);
    test_model.position = { 0,5 };
    test_model.name = v_str(g_mkid());
    std::cout << "generate model1" << '\n';
    lvl.load_chunk({
        .vp = { { 0,0 }, { 43,44 } },
        .models = {
            test_model
        }
    });
    std::cout << "load chunk1" << '\n';
    Gmeng::Renderer::Model test_model_2 = {
        .position = { 20,0 } // position is within the chunk, so the second chunks' 20th X position. Not overall.
    };
    std::cout << "generate model2" << '\n';
    test_model_2.attach_texture(gm::vd_find_texture(Gmeng::vgm_defaults::vg_textures, "01_cake_txtr"));
    std::cout << "texture model2" << '\n';
    lvl.load_chunk({
        .vp = { { 44,0 }, { 87,43 } },
        .models = {
            test_model_2
        }
    });
    std::cout << "load chunk2" << '\n';
    Gmeng::Renderer::viewpoint def_vp1 = { { 0,0 }, { 87, 43 } };
    Gmeng::Renderer::viewpoint def_vp = { { 0,0 }, { 20, 20 } };
    unsigned int def_cx = 30;
    unsigned int def_cy = 15;
    unsigned int def_ms = 10;
    bool paused = false;
    lvl.display.set_resolution(Gmeng::_vcreate_vp2d_deltax(def_vp), Gmeng::_vcreate_vp2d_deltay(def_vp));
    lvl.display.viewpoint = def_vp1;
    lvl.display.camera.set_modifier("cubic_render", true);
    std::vector<gmeng::Unit> _renderscale = Gmeng::_vget_renderscale2dpartial_scalar(lvl, true);
    //g_sleep(ms(5009));
    std::cout << "renderscale done" << '\n';
    std::string _lvlview = Gmeng::get_lvl_view(lvl, _renderscale, true);
    std::cout << "level_view done" << '\n';
    Gmeng::emplace_lvl_camera(lvl, _lvlview);
    std::cout << "emplace_lvl_camera done" << '\n';
    lvl.display.camera.clear_screen();
    std::cout << lvl.display.camera.draw() << '\n';
    do
    {
        cout << "Press [enter] to continue...";
    } while (cin.get() != '\n');
    lvl.display.camera.clear_screen();
    lvl.display.viewpoint = def_vp; // normal vp
    for (int c_counter_t = 0; c_counter_t < def_cx; c_counter_t++) {
        if (paused) { c_counter_t =0; continue; };
        lvl.display.viewpoint.start.x += 1;
        lvl.display.viewpoint.end.x   += 1;
        Gmeng::emplace_lvl_camera(lvl, get_lvl_view(lvl, _renderscale,true));
        lvl.display.camera.reset_cur();

        std::cout << lvl.display.camera.draw() << '\n';
        g_sleep(ms(def_ms));
    };
    lvl.display.camera.clear_screen();
    g_sleep(ms(500));
    lvl.display.viewpoint = def_vp;
    for (int c_counter_t = 0; c_counter_t < def_cx; c_counter_t++) {
        if (paused) { c_counter_t =0; continue; };
        lvl.display.viewpoint.start.x -= 1;
        lvl.display.viewpoint.end.x   -= 1;
        Gmeng::emplace_lvl_camera(lvl, get_lvl_view(lvl, _renderscale,true));
        lvl.display.camera.reset_cur();

        std::cout << lvl.display.camera.draw() << '\n';
        g_sleep(ms(def_ms));
    };
    std::cout << "X axis movement done" << '\n';
    g_sleep(ms(1000));
    lvl.display.viewpoint = def_vp;
    for (int c_counter_t = 0; c_counter_t < def_cy; c_counter_t++) {
        if (paused) { c_counter_t =0; continue; };
        lvl.display.viewpoint.start.y += 1;
        lvl.display.viewpoint.end.y   += 1;
        Gmeng::emplace_lvl_camera(lvl, get_lvl_view(lvl, _renderscale,true));
        lvl.display.camera.reset_cur();

        std::cout << lvl.display.camera.draw() << '\n';
        g_sleep(ms(def_ms));
    };
    std::cout << "Y axis movement done" << '\n';
    g_sleep(ms(1000));
    lvl.display.viewpoint = def_vp;
    for (int c_counter_t = 0; c_counter_t < def_cy; c_counter_t++) {
        if (paused) { c_counter_t =0; continue; };
        lvl.display.viewpoint.start.y += 1;
        lvl.display.viewpoint.end.y   += 1;
        lvl.display.viewpoint.start.x += 2;
        lvl.display.viewpoint.end.x   += 2;
        Gmeng::emplace_lvl_camera(lvl, get_lvl_view(lvl, _renderscale,true));
        lvl.display.camera.reset_cur();
        std::cout << lvl.display.camera.draw() << '\n';
        g_sleep(ms(def_ms));
    };
    std::cout << "double axis movement done" << '\n';
    return 0;
};

#if GMENG_SDL
int test_sdl_text() {
    _uread_into_vgm("./envs/models");
    Gmeng::GameWindow window = Gmeng::create_window("PREVIEW", 850, 700);
    SDL_Event e;
    bool quit = false;
    Gmeng::sImage img;
    gm::texture txtr = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "allah");
    img.width = 88; img.height = 44;
    for (int i = 0; i < img.width * img.height; i++) {
        img.content.push_back( (color_t)txtr.units.at(i).color );
    };
    Uint32 frame_start, frame_time;
    float fps = 0;
    Uint32 last_displayed = 0;
    Uint32 last_frame_time = 1000;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) quit = true;
        }
        Uint32 tm = SDL_GetTicks();
        frame_start = tm;
        window.clear();
        window.draw(img, { 0, 0 }, 10);
        window.text("frame time: " + v_str(frame_time) + "ms | fps: " + v_str(1000 / frame_time), {0,600}, {255, 0, 0, 255});
        window.refresh();
        frame_time = SDL_GetTicks() - frame_start;
        if (last_displayed == 0 || tm - last_displayed >= 1000) {
            last_displayed = tm;
            last_frame_time = frame_time;
            fps = 1000 / frame_time;
        } else fps = 1000 / last_frame_time;
    };
    return 0;
};
#endif

int test_network() {
    int PORT = 3333;
    gmserver_t server (PORT);
    server.create_path( path_type_t::GET, "/",
    [&](request& req, response& res) -> void {
        std::cout << "[TEST_NETWORK] GET request to " << req.path << " from " << req.remote.address << " (" << req.remote.id << ")\n";
        res.status_code = 200;
        res.body = "OK";
        return (void) 0;
    } );

    server.create_path(path_type_t::GET, "/stop",
    [&](request& req, response& res) -> void {
        std::cout << "[TEST_NETWORK] (fd " << server.server_fd <<") stopping server... (request made by " << req.remote.address << ")\n";
        res.status_code = 200;
        res.body = "STOPPED";
        server.stop();
        return (void) 0;
    } );

    std::cout << "running in port " << PORT << '\n';
    server.run();
    if (!server.running.load() && !server.exited.load()) {
        std::cout << "[TEST_NETWORK] " << server.exited << ": bind failed, wait a few seconds before restarting the server\n";
        return 1;
    };
    return 0;
};


static std::vector<int (*)()> testids = {
    &test_vgmcontent,
    &test_caketxtr,
    &test_loadtexture,
    &test_placement,
    &test_renderer,
    &test_loadglvl,
    &test_chunkvpoint,
    &test_vpointrender,
    &test_vwhole_renderer,
#if GMENG_SDL
    &test_sdl_text,
#endif
    &test_network,
};

using Gmeng::color_t;

int main(int argc, char* argv[]) {
    std::vector<int> do_list = {};
    bool do_main1 = false;
    gm_log("gmeng_tests -> SPAWN(1)");
    gm::global.dev_console = false;
    patch_argv_global(argc, argv);
    for (int i = 0; i < argc; i++) {
        char *v_arg = argv[i];
        std::string argument (v_arg);
        if (startsWith(argument, "-l=")) {
            std::vector<std::string> do_tests = g_splitStr(argument.substr(3), ",");
            for (const auto& tid : do_tests) if (tid.length() > 0) do_list.push_back(std::stoi(tid));
        };
    };
    {
        if (do_main1) {
            do_list.clear();
            for (int i = 0; i < testids.size(); i++) {
                do_list.push_back(i);
            };
        };
        init_logc();
        gm::_uread_into_vgm("./envs/models");
        int total = 0;
        std::vector<std::vector<int>> return_values;
        for (int i = 0; i < testids.size(); i++) {
            auto it = std::find(do_list.begin(), do_list.end(), i);
            if (it == do_list.end()) { std::cout << "skipping test_000" << i << ": since test loader do_list does not include it" << '\n'; continue; };
            std::cout << "running 000" << i << "_test_nr_" << i << " | heap_at: " << _uconv_1ihx(_uget_addr(testids[i])) << '\n';
            total++;
            if (testids.size() == i) break;
            try {
            int value = testids[i]();
            std::cout << "test 000" << i << "_test returned heap_value: " << _uconv_1ihx(value) << '\n';
            return_values.push_back({ value, i });
            } catch (std::exception& e) { return_values.push_back({ 1, i }); continue; };
        };
        _gthread_catchup();
        std::cout << "done running tests | total: " << total << '\n' << '\n';
        std::cout << Gmeng::resetcolor << Gmeng::boldcolor << "Test Results:\n";
        int i = 0;
        for (auto rvalue : return_values) std::cout << Gmeng::boldcolor << Gmeng::colors[rvalue[0] == 0 ? Gmeng::GREEN : Gmeng::RED] + "[" << (rvalue[0] == 0 ? "+" : "-") << "] " << Gmeng::resetcolor << " test_000" + v_str(rvalue[1]) + ": " << rvalue[0] << " " << (rvalue[0] == 0 ? Gmeng::colors[Gmeng::GREEN] + "PASS" : Gmeng::colors[Gmeng::RED] + "FAIL") << Gmeng::resetcolor << "\n", i++;
        return 0;
    };
};
