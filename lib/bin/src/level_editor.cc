/*
 * GMENG Version 12.0.0 SDL-BASED LEVEL FILE EDITOR
 * (c) catriverr, 2026, All rights reserved
 * ------------------------------------------------
 * https://gmeng.org/
 * https://gmeng.org/git
 *
 * licenced with Zlib
 * ------------------------------------------------
 * FOR EDITING .GLVL FILES
 *
 * BUILD WITH
 *      make use-external compile-file filename=lib/bin/src/level_editor.cc
 * or
 *      make use-external level-editor
 * ------------------------------------------------
 * USAGE
 *      ./editor -file=your_filename.glvl (edit existing file, file must exist)
 *      ./editor -init-file=your_filname.glvl (create new level file, file must not exist)
 * */

#include "../easy.h"
#include "../utils/serialization.cpp"
#include "../utils/window.cpp"
#include <iostream>

using namespace Gmeng;



/// GMENG PROTOTYPE FOR LEVEL EDITING

static std::string level_file = ".SKIPGMENGLEVELFILECHECK";

int main(int argc, char* argv[]) {
    patch_argv_global(argc, argv);

    for (int i = 0; i < argc; i++) {
        std::string v_arg = ( argv[i] );
        std::cout << argc << " " <<  v_arg << std::endl;
        if ( v_arg.starts_with("-file=") ) {
            level_file = v_arg.substr(6);
        };
    };

    if (!filesystem::exists( level_file ) || filesystem::is_directory( level_file )) {
        std::cout << Gmeng::colors[ Gmeng::RED ] << "ERR:\t" << Gmeng::resetcolor << " level_file " << level_file << " is invalid\n";
        return 0;
    };

    vgm_load("envs/models");

    auto lvl = std::make_unique<Gmeng::Level>();
    read_level_data(level_file, *lvl);

    lvl->display.viewpoint = { { 0, 0 }, { 100, 100 } };

    Gmeng::EventLoop ev;
    ev.level = lvl.get(); // Pass the pointer to the heap object

    auto win = std::make_unique<Gmeng::GameWindow>("GMENG Preview", 1920, 915);
    Gmeng::Renderer::drawpoint last_pos = { 0, 0 };

    ev.add_hook({ FIXED_UPDATE }, [&ev, &win](Level* lvl, EventInfo*) {
        auto renderscale = get_renderscale( *lvl );

        Gmeng::sImage _source;
        _source.width = vp_width(lvl->display.viewpoint);
        _source.height = vp_height(lvl->display.viewpoint);

        for ( auto unit : renderscale )
            if (!unit.is_entity) _source.content.push_back( (color_t) unit.color );

        Gmeng::sImage source = stretchImage(_source, win->frame_width, win->frame_height);
        win->main_camera(source);
    });

    ev.add_hook({ KEYPRESS }, [&ev](Level* lvl, EventInfo* info) {
        if (info->KEYPRESS_SYM == SDLK_w) {
            ev.level->entities.at(0)->position.y--;
        } else if (info->KEYPRESS_SYM == SDLK_s) {
            ev.level->entities.at(0)->position.y++;
        } else if (info->KEYPRESS_SYM == SDLK_a) {
            ev.level->entities.at(0)->position.x--;
        } else if (info->KEYPRESS_SYM == SDLK_d) {
            ev.level->entities.at(0)->position.x++;
        };
    });

    ev.add_hook({ MOUSE_CLICK_LEFT_START }, [&win, &last_pos](Level* lvl, EventInfo* info) {
            auto descaled_pos = unscale_drawpoint( { info->MOUSE_X_POS, info->MOUSE_Y_POS },
                                                    vp_width(lvl->display.viewpoint),
                                                    vp_height(lvl->display.viewpoint),
                                                    win->frame_width, win->frame_height);
            last_pos = descaled_pos;
    });

    ev.add_hook({ EXIT }, [](auto* lvl, auto) {
        write_level_data(level_file, *lvl);
    });

    do_event_loop(win.get(), &ev);
}
