#include <chrono>
#include <ostream>
#define __GMENG_ALLOW_LOG__ true
#define __GMENG_LOG_TO_COUT__ true
#include <iostream>
#include <algorithm>
#include "../lib/bin/gmeng.hpp"
#include "../lib/bin/def/renderer.cpp"

#define g_sleep std::this_thread::sleep_for
#define ms std::chrono::milliseconds

using std::endl;

int test_loadtexture() {
    Gmeng::texture test; Gmeng::CameraView<2, 2> wmap;
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
    Gmeng::Level test_level;
    auto gtx = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "g_template1");
    test_level.base.height = gtx.height;
    test_level.base.width = gtx.width;
    test_level.base.lvl_template.width = gtx.width;
    test_level.base.lvl_template.height = gtx.height;
    for ( int i = 0; i < gtx.height*gtx.width; i++ ) {
        test_level.base.lvl_template.units.push_back(gtx.units[i]);
    };
    test_level.load_chunk(
        Gmeng::r_chunk {
            .vp={
                .start = { .x=0,.y=0 },
                .end   = { .x=4,.y=4 }
            },
            .models = {
                Gmeng::Renderer::Model {
                    .width=5,
                    .height=3,
                    .size=15,
                    .position= { .x=0, .y=0 },
                    .name="tx_model1",
                    .texture = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "tx_model1"),
                    .id=235467,
                }
            }
        }
    );
    test_level.load_chunk(Gmeng::r_chunk {
        .vp = {
            .start = { .x=5, .y=5 },
            .end   = { .x=9, .y=9 },
        },
        .models = {
            Gmeng::Renderer::Model {
                .width=5,
                .height=3,
                .size=15,
                .position= { .x=0, .y=0 },
                .name="tx_model2",
                .texture = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "tx_model2"),
                .id=345678
            }
        }
    });
    test_level.display.set_resolution(5, 5);
    test_level.display.viewpoint = {
        .start = {
            .x = 0,
            .y = 0
        },
        .end = {
            .x = 5,
            .y = 5
        }
    };
    test_level.draw_camera(0);
    test_level.display.viewpoint = {
        .start = {
            .x = 4,
            .y = 4,
        },
        .end   = {
            .x = 9,
            .y = 9
        }
    };
    test_level.draw_camera(1);
    return 0;
};
int test_loadglvl() {
    std::cout << "test_loadglvl job_start" << std::endl;
    Gmeng::LevelInfo lv_inf = Gmeng::parse_glvl("envs/4.0_test.glvl");
    std::cout << "c_p1xdp - c_p2dp | vchunk_info" << std::endl;
    Gmeng::Level lv_test;
    lv_test.load_level(lv_inf);
    std::cout << Gmeng::Renderer::conv_dp(lv_test.chunks[0].vp.start) << " - " << Gmeng::Renderer::conv_dp(lv_test.chunks[0].vp.end) << endl;
    lv_test.set_player({
        .entityId=0,
        .colorId=3,
        .colored=true,
        .c_ent_tag="o"
    }, 1, 0);
    lv_test.draw_camera(0);
    std::cout << "chunk id(0) num(1) displayed ^ above ~~ works" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    lv_test.draw_camera(1);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    lv_test.move_player(0, 1);
    lv_test.refresh();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    lv_test.move_player(0, 2);
    lv_test.refresh();
    std::cout << "chunk id(1) num(2) displayed ^ above & Objects::G_Player attached to camera ~~ works" << std::endl << "~~ no modifiers required" << std::endl;
    std::cout << "player pos: " << lv_test.plcoords.x << "," << lv_test.plcoords.y << endl;
    return 0;
};

int main1() {
    _gupdate_logc_intvl();
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
    Gmeng::CameraView<1, 1> camera;
    camera.SetResolution(1, 1);
        for (int j = 0; j < Gmeng::vgm_defaults::vg_textures.size(0); j++) {
        auto tx = Gmeng::vgm_defaults::vg_textures.indx(j);
        std::cout << "drawing texture: ";
        std::cout << tx.name << " | notxtr: " << (Gmeng::notxtr.name == tx.name ? "true" : "false");
        for (int i = 0; i < tx.units.size(); i++) {
            if ((i % tx.width) == 0) std::cout << std::endl;
            std::cout << camera.draw_unit(tx.units[i]);
        };
        std::cout << std::endl;
    };
    std::cout << std::endl;
    return 0;
};

int test_caketxtr() {
    gm::texture tx = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "01_cake_txtr");
    Gmeng::CameraView<1, 1> camera;
    std::cout << "drawing texture: ";
    std::cout << tx.name << " | notxtr: " << (Gmeng::notxtr.name == tx.name ? "true" : "false") << endl;
    camera.SetResolution(1, 1);
    for (int i = 0; i < tx.units.size(); i++) {
        if ( i % tx.width == 0 ) std::cout << std::endl;
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
    std::cout << "starting test for __viewpoint_vector_move_camera__" << std::endl;
    /// bit to check text_data
    gm::global.dev_console = true;
    _udraw_display(Gmeng::logc);
    gm::global.dev_console = false;
    return 0;
};

int test_vpointrender() {
    std::cout << "starting test_vpointrender" << std::endl;
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
    level_test.display.viewpoint = { { 10, 5 }, { 20, 10 } };
    gm_log("__test_chunkvpoint__ (static test) -> __attempt_test__ ; _vcamv_gen_frame tl");
    std::cout << std::endl;
    std::string __test__ = Gmeng::_vcamv_gen_frame(level_test);
    std::cout << __test__ << std::endl;
    exit(0);
    return 0;
};

int test_vwhole_renderer() {
    std::cout << "beginning test" << std::endl;
    Gmeng::Level lvl = {
        .base = {
            .lvl_template = gm::vd_find_texture(Gmeng::vgm_defaults::vg_textures, "allah"),
            .width = 88,
            .height = 44
        },
        .name = v_str(g_mkid())
    };

    std::cout << "generate lvl" << std::endl;
    Gmeng::Renderer::Model test_model = Gmeng::Renderer::generate_empty_model(10, 5);
    test_model.position = { 0,5 };
    test_model.name = v_str(g_mkid());
    std::cout << "generate model1" << std::endl;
    lvl.load_chunk({
        .vp = { { 0,0 }, { 43,44 } },
        .models = {
            test_model
        }
    });
    std::cout << "load chunk1" << std::endl;
    Gmeng::Renderer::Model test_model_2 = {
        .position = { 20,0 }
    };
    std::cout << "generate model2" << std::endl;
    test_model_2.attach_texture(gm::vd_find_texture(Gmeng::vgm_defaults::vg_textures, "01_cake_txtr"));
    std::cout << "texture model2" << std::endl;
    lvl.load_chunk({
        .vp = { { 44,0 }, { 87,43 } },
        .models = {
            test_model_2
        }
    });
    std::cout << "load chunk2" << std::endl;

    Gmeng::Renderer::viewpoint def_vp = { { 0,0 }, { 40, 20 } };
    unsigned int def_c = 30;
    unsigned int def_ms = 25;
    lvl.display.set_resolution(Gmeng::_vcreate_vp2d_deltax(def_vp), Gmeng::_vcreate_vp2d_deltay(def_vp));
    lvl.display.viewpoint = def_vp;
    std::vector<std::string> _renderscale = Gmeng::_vget_renderscale2dpartial_scalar(lvl);
    std::cout << "renderscale done" << std::endl;
    std::string _lvlview = Gmeng::get_lvl_view(lvl, _renderscale);
    std::cout << "level_view done" << std::endl;
    Gmeng::emplace_lvl_camera(lvl, _lvlview);
    std::cout << "emplace_lvl_camera done" << std::endl;
    lvl.display.camera.clear_screen();
    for (int c_counter_t = 0; c_counter_t < def_c; c_counter_t++) {
        lvl.display.viewpoint.start.x += 1;
        lvl.display.viewpoint.end.x   += 1;
        Gmeng::emplace_lvl_camera(lvl, get_lvl_view(lvl, _renderscale));
        std::cout << lvl.display.camera.draw() << std::endl;
        g_sleep(ms(def_ms));
    };
    std::cout << "X axis movement done" << std::endl;
    g_sleep(ms(1000));
    lvl.display.viewpoint = def_vp;
    for (int c_counter_t = 0; c_counter_t < def_c; c_counter_t++) {
        lvl.display.viewpoint.start.y += 1;
        lvl.display.viewpoint.end.y   += 1;
        Gmeng::emplace_lvl_camera(lvl, get_lvl_view(lvl, _renderscale));
        std::cout << lvl.display.camera.draw() << std::endl;
        g_sleep(ms(def_ms));
    };
    std::cout << "Y axis movement done" << std::endl;
    g_sleep(ms(1000));
    lvl.display.viewpoint = def_vp;
    for (int c_counter_t = 0; c_counter_t < def_c; c_counter_t++) {
        lvl.display.viewpoint.start.y += 1;
        lvl.display.viewpoint.end.y   += 1;
        lvl.display.viewpoint.start.x += 1;
        lvl.display.viewpoint.end.x   += 1;
        Gmeng::emplace_lvl_camera(lvl, get_lvl_view(lvl, _renderscale));
        std::cout << lvl.display.camera.draw() << std::endl;
        g_sleep(ms(def_ms));
    };
    std::cout << "double axis movement done" << std::endl;
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
    &test_vwhole_renderer
};

int main(int argc, char* argv[]) {
    std::cout << "gmeng_tests -> SPAWN(1)" << std::endl;
    std::cout << "gmeng_tests -> checking platform (expecting OS X)" << std::endl;
    #if _WIN32
        std::cout << Gmeng::colors[4] << "libgmeng-abi: __excuse__" << std::endl;
        std::cout << "INTERNAL: __gmeng_platform__, __gmeng_threading__, __stdlib__, __libc++-abi__, __std_com_apple_main_pthread__" << std::endl;
        std::cout << "Gmeng is not available in a core-platform other than darwin ( apple_kernel )." << std::endl;
        std::cout << "current_platform: win32 ( WINDOWS_NT )" << std::endl;
        std::cout << "__gmeng_halt_execution__( CAUSE( gmeng::global.v_exceptions->__find__( \"controller.platform\" ) ) && CAUSE( \"__environment_not_suitable__\" ) )" << std::endl;
        return 0;
    #endif
    std::cout << "gmeng_tests -> checking arguments" << std::endl;
    gm::global.dev_console = true;
    std::vector<int> do_list = {};
    bool do_main1 = false;
    _gargv_patch_global(argc, argv);
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
        _gupdate_logc_intvl();
        gm::_uread_into_vgm("./envs/models");
        int total = 0;
        for (int i = 0; i < testids.size(); i++) {
            auto it = std::find(do_list.begin(), do_list.end(), i);
            if (it == do_list.end()) { std::cout << "skipping test_000" << i << ": since test loader do_list does not include it" << std::endl; continue; };
            std::cout << "running 000" << i << "_test_nr_" << i << " | heap_at: " << _uconv_1ihx(_uget_addr(testids[i])) << std::endl;
            total++;
            if (testids.size() == i) break;
            int value = testids[i]();
            std::cout << "test 000" << i << "_test returned heap_value: " << _uconv_1ihx(value) << std::endl;
        };
        _gthread_catchup();
        std::cout << "done running tests | total: " << total << std::endl;
        return 0;
    };
};
