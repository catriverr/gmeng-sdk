#include <iostream>
#include <array>
#pragma once
#include <vector>
#include <cstdio>
#include <string>
#include <map>
#include <math.h>

#ifdef __GMENG_OBJECTINIT__


/// t_init structures
namespace Gmeng {
    struct t_display {
        Objects::coord pos; int v_cursor = 0; int v_outline_color = 1;
        int v_width; int v_height; bool init = false; int v_textcolor = 2;
        std::vector< std::string > v_drawpoints; std::string title;
    };
    const char d_endl = '\x0f'; /// newline | formatter 0
    struct t_charpos { int p1; /* x axis */ int p2; /* y axis */ };
};

namespace Gmeng::display_thread_controller {
};

static void _uset_cursor_pos(int x, int y) {
    std::cout << "\033[" << y << ";" << x << "H";
};

static Objects::coord v_conv_1dirp(int cpos, int wsize) {
    return Objects::coord {
        .x=(cpos % wsize),
        .y=(cpos / wsize),
    };
 };
static int v_conv_2dvcp(Objects::coord cpos, int wsize2) {
    return (cpos.y*wsize2)+cpos.x;
};

static void _udisplay_init(Gmeng::t_display& display, bool force = false) {
    if (display.init && !force) return;
    const auto base = display.v_drawpoints;
    int v_size = display.v_width * display.v_height;
    for ( int i = 0; i < v_size; i++ ) if (i >= base.size()) display.v_drawpoints.push_back("\x0b");
    display.init = true;
    display.v_cursor = 0;
};

static bool gm_validate_dvs1dc(Gmeng::t_display __d) {
    if (__d.v_width * __d.v_height != __d.v_drawpoints.size() || !__d.init) return false;
    return true;
};

static int _uget_textsize(Gmeng::t_display& display) {
    std::vector<std::string> v_nchars;
    for (const auto& vl : display.v_drawpoints) {
        if (vl != "\x0b") v_nchars.push_back(vl);
    };
    return v_nchars.size();
};


/// flushed v_drawpoints vector within a display
static void _uflush_vdisplay(Gmeng::t_display& display) {
    std::vector<std::string> vl_nchars;
    for (const std::string& str : display.v_drawpoints) {
        if (str != "\x0d") vl_nchars.push_back(str);
    }
    for (int vndx = 0; vndx < (display.v_drawpoints.size() - vl_nchars.size()); vndx++) {
        vl_nchars.push_back("\x0b");
    };
    display.v_drawpoints = vl_nchars;
    _udisplay_init(display, true);
};

static void _uflush_display(Gmeng::t_display& display, int keep_last = 0) {
    if (!gm_validate_dvs1dc(display)) throw std::invalid_argument("gmeng/utils *termui -> _uflush_display() >> gm_invalid_display");
    for (int v_cch = 0; v_cch < display.v_width; v_cch++) {
        display.v_drawpoints.erase(display.v_drawpoints.begin());
    };
    _uflush_vdisplay(display);
};



static void _uchar_control(Gmeng::t_display &display, Objects::coord r_pos, std::string char1) {
    if (!gm_validate_dvs1dc(display)) throw std::invalid_argument("gmeng/utils *termui -> _uchar_control >> gm_validate_dvs1dc return FALSE : 'display' object parsed into method _uchar_control (with type Gmeng::t_display) is invalid.");
    display.v_drawpoints[v_conv_2dvcp(r_pos, display.v_height)] = char1;
    display.v_cursor = v_conv_2dvcp(r_pos, display.v_height) + char1.length();
};

static void _utext(Gmeng::t_display &display, std::string _ln) {
    int v_countchars = 0;
    for (const auto& ch : _ln) {
        if (ch == '\n' || ch == Gmeng::d_endl) { v_countchars += (display.v_width - v_countchars); continue; };
        if (display.v_drawpoints.size()-1 < display.v_cursor+v_countchars) continue;
        display.v_drawpoints[display.v_cursor+v_countchars] = ch;
        v_countchars++;
    };
    display.v_cursor += v_countchars;
};

static std::string _uconv_addr_str(const void* address) {
    std::stringstream ss;
    ss << address;
    return ss.str();
}

static void _udraw_display(Gmeng::t_display& __d) {
    const void* address = static_cast<const void*>(&__d);
    std::string v_addr = _uconv_addr_str(address);
    if (!gm_validate_dvs1dc(__d)) gm_err(1, "_udraw_display(Gmeng::t_display& __d) -> display object &__d (memaddr0 " + v_addr + ") not valid | gm_validate_dvs1dc(*__d) failed");
    Objects::coord __vcurr_pos = { .x = __d.pos.x, .y = __d.pos.y };
    int vc_vector_count = 0x00;
    _uset_cursor_pos(__vcurr_pos.x, __vcurr_pos.y);
    std::cout << Gmeng::colors[__d.v_outline_color] << "┌┤" << __d.title << "├" << repeatString("─", __d.v_width-__d.title.length()) << "┐" << Gmeng::resetcolor;
    _uset_cursor_pos(__vcurr_pos.x, __vcurr_pos.y+1);
    for (const auto& dp : __d.v_drawpoints) {
        if (vc_vector_count % __d.v_width == 0) { __vcurr_pos.y += 1; std::cout << Gmeng::colors[__d.v_outline_color] << " │"; _uset_cursor_pos(__vcurr_pos.x, __vcurr_pos.y); std::cout << Gmeng::colors[__d.v_outline_color] << "│ " << Gmeng::resetcolor; };
        _uset_cursor_pos(__vcurr_pos.x+2 + (vc_vector_count % __d.v_width), __vcurr_pos.y);
        std::cout << Gmeng::colors[__d.v_textcolor] << (dp == "\x0b" ? " " : dp) << Gmeng::resetcolor; vc_vector_count++;
    };
    _uset_cursor_pos(__vcurr_pos.x, __vcurr_pos.y);
    std::cout << Gmeng::colors[__d.v_outline_color] << "└" << repeatString("─", __d.v_width+2) << "┘" << Gmeng::resetcolor << endl;
};


#define __GMENG_DISPLAYMGR_INIT__ true
#endif
