#pragma once

#ifndef GMENG_NO_SOL

#include <sol/sol.hpp>

#include "../src/objects.cpp"
#include "../src/textures.cpp"

#include "../types/unit.h"
#include "../types/texture.h"
#include "../types/modifier.h"


#include "colorformat.h"
#include "startup.h"

#include "global.h"
#include "functree.h"
#include "log.h"
#include "assert.h"

namespace Gmeng {
    namespace Scripts {
        static sol::state lua;
        static sol::table gmeng_lua_namespace = lua["gmeng"].get_or_create<sol::table>();
        static sol::table renderer_lua_namespace = lua["gmeng.renderer"].get_or_create<sol::table>();
        static int init_sol2();
    };
};

static int Gmeng::Scripts::init_sol2() {
    using namespace Gmeng;
    using namespace Gmeng::Scripts;
    lua.open_libraries(sol::lib::base);

    lua.set("GMENG_BUILD_NO", GMENG_BUILD_NO);

    lua.set_function("g_mkid", &g_mkid);
    lua.set_function("get_curtime", &get_curtime);
    lua.set_function("get_curdate", &get_curtime);
    lua.set_function("_uget_thread", &_uget_thread);

    lua.set_function("gm_log", [&](std::string message) {
        _gm_log("luascript", 0, "repl", message);
    });

    lua.set_function("g_splitStr", &g_splitStr);
    lua.set_function("g_joinStr", &g_joinStr);

    lua.set_function("restart_program", &restart_program);
    lua.set_function("patch_argv_global", &patch_argv_global);
    lua.set_function("get_executable_directory", &get_executable_directory);
    lua.set_function("set_terminal_title", &set_terminal_title);
    lua.set_function("gm_err", &gm_err);
    lua.set_function("gmeng_show_warning", [&](std::string x) {
        _gmeng_show_warning(x, "luascript", 0);
    });

    lua.new_usertype<TerminalSize>("TerminalSize",
        "width", &TerminalSize::width,
        "height", &TerminalSize::height
    );

    lua.set_function("get_terminal_size", &get_terminal_size);


    lua.set_function("init_logc", &init_logc);
    lua.set_function("boolstr", &boolstr);

    lua.set_function("colorformat", &colorformat);
    lua.set_function("WRITE_PARSED", [&](std::string x) { WRITE_PARSED(x); });
    lua.set_function("SAY", [&](std::string x) { SAY(x); });
    lua.set_function("INF", [&](std::string x) { INF(x); });
    lua.set_function("strip_ansi", &strip_ansi);
    lua.set_function("logical_to_physical_index", &logical_to_physical_index);
    lua.set_function("is_ansi_start", &is_ansi_start);
    lua.set_function("split_with_ansi", &split_with_ansi);

    lua.set("colornames", &colornames);
    lua.set("bgcolornames", &bgcolornames);

    lua.set_function("str_replace", &str_replace);
    lua.set_function("get_username", &get_username);

    gmeng_lua_namespace.new_enum("color_t",
        "WHITE", color_t::WHITE,
        "BLUE", color_t::BLUE,
        "GREEN", color_t::GREEN,
        "CYAN", color_t::CYAN,
        "RED", color_t::RED,
        "PINK", color_t::PINK,
        "YELLOW", color_t::YELLOW,
        "BLACK", color_t::BLACK
    );

    gmeng_lua_namespace.set("version", Gmeng::version);

    gmeng_lua_namespace.new_usertype<Gmeng::Coordinate>("coordinate",
        "x", &Gmeng::Coordinate::x,
        "y", &Gmeng::Coordinate::y
    );

    gmeng_lua_namespace.set("resetcolor", resetcolor);
    gmeng_lua_namespace.set("boldcolor", boldcolor);


    auto v3 = gmeng_lua_namespace.new_usertype<Vec3>("Vec3",
        "x", &Vec3::x,
        "y", &Vec3::y,
        "z", &Vec3::z
    );
    v3["cross"] = &Vec3::cross;
    v3["dot"] = &Vec3::dot;
    v3["length"] = &Vec3::length;
    v3["normalized"] = &Vec3::normalized;

    gmeng_lua_namespace.new_usertype<color32_t>("color32_t",
        "r", &color32_t::r,
        "g", &color32_t::g,
        "b", &color32_t::b,
        "to_vec3", &color32_t::to_vec3
    );

    gmeng_lua_namespace.set_function("uint32_from_color32", &uint32_from_color32);

    gmeng_lua_namespace.new_usertype<unitcolor_t>("unitcolor_t",
        "c8_value", &unitcolor_t::c8_value,
        "rgb_value", &unitcolor_t::rgb_value
    );

    gmeng_lua_namespace.set_function("conv_rgb_col8", &conv_rgb_col8);

    gmeng_lua_namespace.new_usertype<Gmeng::Unit>("Unit",
        "color", &Unit::color,
        "collidable", &Unit::collidable,
        "is_entity", &Unit::is_entity,
        "transparent", &Unit::transparent,
        "special", &Unit::special,
        "special_clr", &Unit::special_clr,
        "special_c_unit", &Unit::special_c_unit
    );

    auto blob = gmeng_lua_namespace.new_usertype<Gmeng::Blob>("Blob",
        "width", &Gmeng::Blob::width,
        "height", &Gmeng::Blob::height,
        "units", &Gmeng::Blob::units
    );

    blob.set_function("rgb_array", &Gmeng::Blob::rgb_array);


    auto texture = gmeng_lua_namespace.new_usertype<Gmeng::texture>("texture",
        "width", &texture::width,
        "height", &texture::height,
        "units", &texture::units,
        "collidable", &texture::collidable,
        "name", &texture::name
    );

    texture.set_function("rgb_array", &texture::rgb_array);


    gmeng_lua_namespace.set_function("set_texturemap", &set_texturemap);
    gmeng_lua_namespace.set_function("LoadTexture", &LoadTexture);

    gmeng_lua_namespace.new_usertype<modifier>("modifier",
        "name", &modifier::name,
        "value", &modifier::value
    );

    auto modlist = gmeng_lua_namespace.new_usertype<Gmeng::ModifierList>("ModifierList",
        sol::constructors<ModifierList()>(  ),
        "values", &ModifierList::values
    );

    modlist["get_value"] = &ModifierList::get_value;
    modlist["set_value"] = &ModifierList::set_value;
    modlist["has_value"] = &ModifierList::has_value;
    modlist["add_value"] = &ModifierList::add_value;
    modlist["erase"]     = &ModifierList::erase;
    modlist["sort"]      = &ModifierList::sort;

    gmeng_lua_namespace.new_usertype<Gmeng::__global_object__>("__global_object__",
        "dev_console", &__global_object__::dev_console,
        "debugger", &__global_object__::debugger,
        "log_stout", &__global_object__::log_stout,
        "dev_mode", &__global_object__::dev_mode,
        "dont_hold_back", &__global_object__::dont_hold_back,
        "shush", &__global_object__::shush,
        "weird_ass", &__global_object__::weird_ass,
        "restarted_instance", &__global_object__::restarted_instance,
        "ignore_assert", &__global_object__::ignore_assert,
        "raw_arguments", &__global_object__::raw_arguments,
        "executable", &__global_object__::executable,
        "raw_executable_name", &__global_object__::raw_executable_name,
        "user", &__global_object__::user,
        "pwd", &__global_object__::pwd,
        "prog_argc", &__global_object__::prog_argc,
        "prog_argv", &__global_object__::prog_argv,
        "window_control", &__global_object__::window_control,
        "terminal_has_sgr_pixel_support", &__global_object__::terminal_has_sgr_pixel_support,
        "terminal_has_kgp_pixel_support", &__global_object__::terminal_has_kgp_pixel_support,
        "terminal_has_keyboard_support", &__global_object__::terminal_has_keyboard_support,
        "no_advanced_keyboard", &__global_object__::no_advanced_keyboard
    );

    gmeng_lua_namespace.set("global", &Gmeng::global);


    renderer_lua_namespace.new_usertype<Gmeng::Renderer::drawpoint>("drawpoint",
        "x", &Renderer::drawpoint::x,
        "y", &Renderer::drawpoint::y
    );

    renderer_lua_namespace.new_usertype<Gmeng::Renderer::viewpoint>("viewpoint",
        "start", &Renderer::viewpoint::start,
        "end", &Renderer::viewpoint::end
    );
    
    return 0;
};
static int sol2_init_state = Gmeng::Scripts::init_sol2();

#endif // GMENG_NO_SOL
