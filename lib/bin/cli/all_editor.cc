#pragma once
#include <cstddef>
#include <cstring>
#include <ncursesw/ncurses.h>
#include <string>
#include <locale.h>
#include "../gmeng.h"  // Replace with your actual header
#include "../utils/serialization.cpp"
#include "../cli/index.h"
#include "../utils/serialization.cpp"

using namespace Gmeng;

static std::string _ge_filename;

/// random color_pair value
/// for highlighted text
#define TEXTHIGHLIGHT 71

static Gmeng::Level _ge_level;
static Gmeng::EventLoop _ge_ev;

class glvl_editor_command_t : public Gmeng_Commandline::Subcommand {
  public:
    glvl_editor_command_t(string name, string desc) : Subcommand(name, desc) {};

    void run(vector<std::string> args) {
        if (args.size() < 1 || !filesystem::exists(args.at(0))) {
            LOG("~r~ERROR!~n~ please provide a filename.\n\t~g~Usage:~n~ " + Gmeng::global.executable + " ~b~glvl~n~ <filename>");
            return;
        };
        Gmeng::_uread_into_vgm("envs/models");
        _ge_filename = args.at(0);

        LOG("attempting to read .GLVL file " + _ge_filename);

        int ival = -1;
        if ((ival = read_level_data(_ge_filename, _ge_level)) != 0) {
            LOG("read_level_data(E"$(ival)"): Error occured while attempting to read .GLVL file: " + _ge_filename = ", check gmeng.log");
            return;
        };

        LOG("success, loading editor");
        _ge_ev.level = &_ge_level; // eventloop init

        _ge_ev.level->display.viewpoint = { { 0, 0 }, { 99, 99 } };

        auto renderscale = gmeng::get_renderscale(*_ge_ev.level);
        LOG("renderscale taken");

        _ge_ev.add_hook({ INIT }, [&](Level*, EventInfo* info) {
            _ge_ev.call_event(FIXED_UPDATE, *info);
        });

        _ge_ev.add_hook({ FIXED_UPDATE }, [&](Level* lvl, EventInfo* info) {
            if (info->EVENT == MOUSE_MOVE) return;
            renderscale = gmeng::get_renderscale(*lvl);
            auto txt_rscl = gmeng::renderscale_to_texture(renderscale);
            auto resized = gmeng::resize_texture(txt_rscl, 101, 100);

            lvl->display.camera.reset_cur();
            for (int i = 0; i < resized.width*resized.height; i++) {
                if (i % resized.width == 0) std::cout << '\n';
                std::cout << lvl->display.camera.draw_unit(resized.units.at(i));
            };
        });

        _ge_ev.add_hook({ KEYPRESS }, [&](Level*, EventInfo* info) {
            _ge_ev.call_event(FIXED_UPDATE, *info);
        });

        _ge_ev.add_hook( { MOUSE_SCROLL_UP, MOUSE_SCROLL_DOWN }, [&](Level* lvl, EventInfo* info) {

        });

        do_event_loop(&_ge_ev);
    };
};

static glvl_editor_command_t gec("edit", "edits a GLVL Level file.");

static Gmeng_Commandline::InterfaceRegistrar register_gec(
    std::make_unique<glvl_editor_command_t>( gec )
);
