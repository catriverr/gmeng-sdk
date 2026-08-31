#include "lib/bin/easy.h"
#include <iostream>

int main(int argc, char** argv) {
    patch_argv_global(argc, argv);
    vgm_load("envs/models");

    auto level = std::make_unique<Gmeng::Level>();

    read_level_data("envs/planets.glvl", *level);

    level->display.viewpoint = { { 0, 0 }, { 75, 60 } };

    level->display.refresh( level.get() );
    level->display.camera.clear_screen();

    for ( auto ch : level->chunks ) {
        for ( auto model : ch.models ) {
            Gmeng::Renderer::viewpoint vp_m = {
                { ch.vp.start.x + model.position.x,
                  ch.vp.start.y + model.position.y },
                { ch.vp.start.x + model.position.x + (int)model.width,
                  ch.vp.start.y + model.position.y + (int)model.height }
            };
            auto vls = Gmeng::viewpoint_includes( level->display.viewpoint, vp_m );
            std::cout << model.texture.name + ": vp: " + Gmeng::Renderer::conv_vp(vp_m) + " " << vls.empty() << "\n";

            for ( auto dp : vls )
                std::cout << "\t" << Gmeng::Renderer::conv_dp( dp ) << "\n";

        };
    };
    return 0;
};
