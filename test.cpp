#include <chrono>
#define __GMENG_ALLOW_LOG__ true

#include <iostream>
#include "./lib/bin/gmeng.hpp"
#include "./lib/bin/def/renderer.cpp"

using std::endl;



int test_loadtexture() {
    gm_err(0, "v_errortest -> error (this exception can be ignored)");
    std::cout << "test_error success" << v_endl;
    Gmeng::texture test; Gmeng::CameraView<2, 2> wmap;
    test = Gmeng::LoadTexture("texture.gt");
    std::cout << "texture.gt:" << endl;
    std::cout << "name: " << test.name << endl;
    std::cout << "width: " << test.width << endl;
    std::cout << "height: " << test.height << endl;
    std::cout << "collision: " << (test.collidable ? "true" : "false") << endl;
    std::cout << "preview:" << endl;

    int lndx = 0;
    for ( auto indx : test.units ) {
        if ( lndx == test.width ) lndx = 0, std::cout << endl;
        std::cout << wmap.draw_unit(indx);
        lndx++;
    };
    std::cout << endl; return 0;
};

int test_placement() {
    Gmeng::Renderer::drawpoint placement = {2, 3};  // Point where the object will be placed
    Gmeng::Renderer::drawpoint size = {4, 4};       // Size of the object
    Gmeng::Renderer::drawpoint worldSize = {10, 10};   // Size of the world map
    std::vector<Gmeng::Renderer::drawpoint> displacements = Gmeng::Renderer::get_placement(placement, size, worldSize);
    for ( auto point : displacements ) {
        std::cout << "x: " << point.x << " - y: " << point.y << endl;
    };
    return 0;
};


int test_renderer() {
    std::cout << "begining test_renderer" << endl;
    Gmeng::Level test_level;
    std::cout << "initialization works" << endl;
    auto gtx = Gmeng::LoadTexture("sky.gt");
    test_level.base.height = gtx.height;
    test_level.base.width = gtx.width;
    test_level.base.lvl_template.width = gtx.width;
    test_level.base.lvl_template.height = gtx.height;
    std::cout << "width&height OK - LoadTexture sky.gt OK" << endl;
    std::cout << "level_base -> width: " << test_level.base.width << " - height: " << test_level.base.height << endl;
    std::cout << "level_size -> total: " << test_level.base.width*test_level.base.height << endl;
    for ( int i = 0; i < gtx.height*gtx.width; i++ ) {
        std::cout << "compiling unit " << i << endl;
        std::cout << "preview: " << test_level.display.camera.draw_unit(gtx.units[i]) << endl;
        test_level.base.lvl_template.units.push_back(gtx.units[i]);
    };
    std::cout << "set_base works" << endl;
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
                    .texture = Gmeng::LoadTexture("tx_model1.gt"),
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
                .texture = Gmeng::LoadTexture("tx_model2.gt")
            }
        }
    });
    std::cout << "load_chunk works" << endl;
    test_level.display.set_resolution(5, 5);
    std::cout << "set_resolution works" << endl;
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
    std::cout << "viewpoint works" << endl;
    test_level.draw_camera(0);
    std::cout << "draw_camera works" << endl;
    std::cout << "drawing chunk [2]" << endl;
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
    std::cout << "sky.gt preview: " << endl;
    int lndx = 0;
    std::cout << " ";
    for ( auto unit : gtx.units ) {
        if (lndx % gtx.width == 0) std::cout << endl << " ";
        std::cout << test_level.display.camera.draw_unit(unit);
        lndx++;
    };
    std::cout << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    test_level.draw_camera(1);
    std::cout << "draw_camera 2 works" << endl;
    return 0;
};

int test_loadglvl() {
    std::cout << "test" << std::endl;
    Gmeng::LevelInfo lv_inf = Gmeng::parse_glvl("envs/4.0_test.glvl");
    std::cout << "vinf" << std::endl;
    Gmeng::Level lv_test;
    lv_test.load_level(lv_inf);
    std::cout << Gmeng::Renderer::conv_dp(lv_test.chunks[0].vp.start) << " - " << Gmeng::Renderer::conv_dp(lv_test.chunks[0].vp.end) << endl;
    lv_test.draw_camera(0);
    std::cout << "chunk id(0) num(1) displayed ^ above ~~ works" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    lv_test.draw_camera(1);
    std::cout << "chunk id(1) num(2) displayed ^ above ~~ works" << std::endl;
    return 0;
};

int main() {
    test_loadtexture();
    std::cout << "test_loadtexture -> status v_success" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    test_placement();
    std::cout << "test_placement -> status v_success" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    test_renderer();
    std::cout << "test_renderer -> status v_success" << endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    test_loadglvl();
    std::cout << "test_loadglvl -> status v_success" << endl;
    return 0;
};
