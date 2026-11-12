#ifndef GMENG_OBJ_IMPLEMENTATION

#define GMENG_OBJ_IMPLEMENTATION 1

#define GLUE(a, b) a ## b
#define EVAL_TYPE(x) GLUE(TYPE_KEY_, x)

#define TYPE_KEY_unsigned_int 1
#define TYPE_KEY_int          2

#ifdef ImDrawIdx
    #undef ImDrawIdx
    #define ImDrawIdx unsigned int
#endif

#include "../../../include/imgui/imgui.cpp"
#include "../../../include/imgui/imgui_draw.cpp"
#include "../../../include/imgui/imgui_tables.cpp"
#include "../../../include/imgui/imgui_widgets.cpp"

#include "../gmeng.h"

/// COMPATIBILITY FOR EXPORTING TEMPLATE CLASS REFERENCES

template class Gmeng::Camera<0, 0>;
template class GridMap<Gmeng::EntityBase>;


#define GM_FORCE(name, expr) __attribute__((used)) static auto const name = expr

GM_FORCE(_f_move_by,  &Gmeng::Renderer::viewpoint::move_by);
GM_FORCE(_f_cancel,   &Gmeng::EventLoop::cancel);
GM_FORCE(_f_find,     &Gmeng::Level::find_models_at);
GM_FORCE(_f_blur,     &Gmeng::Renderer::Display::DisplayController::blur);
GM_FORCE(_f_refresh,  &Gmeng::Renderer::Display::refresh);
GM_FORCE(_f_mvby,     &Gmeng::Renderer::Display::move_viewpoint_by);
GM_FORCE(_f_curvis,   &Gmeng::Renderer::Display::set_cursor_visibility);
GM_FORCE(_f_addhook,  &Gmeng::EventLoop::add_hook);
GM_FORCE(_f_lvlbase,
    static_cast<void (Gmeng::Renderer::LevelBase::*)(const std::size_t&, const std::size_t&)>(
        &Gmeng::Renderer::LevelBase::resize));

__attribute__((used, noinline, optnone))
void gmeng_force_emit_special_members() {
    Gmeng::color32_t a(uint8_t(1), uint8_t(2), uint8_t(3));
    Gmeng::color32_t b(1u);
    Objects::coord c{0, 0};
    Gmeng::Renderer::drawpoint d = c;           // forces coord::operator drawpoint
    alignas(Gmeng::EventLoop) char buf[sizeof(Gmeng::EventLoop)];
    reinterpret_cast<Gmeng::EventLoop*>(buf)->~EventLoop();
    (void)a; (void)b; (void)d;
}

static const char* __GMENG_OBJ_BUILD_VERSION_ID = Gmeng::version.c_str();

#endif
