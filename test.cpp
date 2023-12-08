#include <chrono>
#define __GMENG_ALLOW_LOG__ true

#include <iostream>
#include "./lib/bin/gmeng.hpp"
#include "./lib/bin/def/renderer.cpp"

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
                    .name="tx_model1",
                    .id=235467,
                    .width=5,
                    .height=3,
                    .position= { .x=0, .y=0 },
                    .size=25,
                    .texture = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "tx_model1"),
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
                .name="tx_model2",
                .id=345678,
                .width=5,
                .height=3,
                .position= { .x=0, .y=0 },
                .size=25,
                .texture = gm::vd_find_texture(gm::vgm_defaults::vg_textures, "tx_model2")
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
    lv_test.set_player({ .colorId=3, .colored=true, .entityId=0, .c_ent_tag="o" }, 0, 0);
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

int main() {
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
