#include <iostream>
#include <array>
#include <vector>
#include <cstdio>
#include <string>
#include <map>
#include <math.h>

#ifndef __GMENG_DISPLAYMGR_INIT__

/// t_init structures
namespace Gmeng {
    struct t_display {
        Objects::coord pos; int v_cursor;
        int v_width; int v_height;
        std::vector< std::string > v_drawpoints;
    };
    struct t_charpos { int p1; /* x axis */ int p2; /* y axis */ };
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

static void _udisplay_init(Gmeng::t_display display) {
    int v_size = display.v_width * display.v_height;
    for ( int i = 0; i < v_size; i++ ) display.v_drawpoints.push_back(" ");
};

static bool gm_validate_dvs1dc(Gmeng::t_display __d) {
    if (__d.v_width * __d.v_height != __d.v_drawpoints.size()) return false;
    return true;
};



static void _uchar_control(Gmeng::t_display &display, Objects::coord r_pos, std::string char1) {
    if (!gm_validate_dvs1dc(display)) throw std::invalid_argument("gmeng/utils *termui -> _uchar_control >> gm_validate_dvs1dc return FALSE : 'display' object parsed into method _uchar_control (with type Gmeng::t_display) is invalid.");
    display.v_drawpoints[v_conv_2dvcp(r_pos, display.v_height)] = char1;
    display.v_cursor = v_conv_2dvcp(r_pos, display.v_height) + char1.length();
};

static void _utext(Gmeng::t_display &display, std::string _ln) {
    int v_countchars = 0;
    for (const auto& ch : _ln) {
        display.v_drawpoints[display.v_cursor+v_countchars] = ch;
        v_countchars++;
    };
};


#define __GMENG_DISPLAYMGR_INIT__ true
#endif
