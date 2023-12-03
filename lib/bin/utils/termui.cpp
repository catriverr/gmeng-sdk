#include <iostream>
#include <array>
#include <vector>
#include <cstdio>
#include <string>
#include <map>

#ifdef __GMENG_INIT__

/// t_init structures
namespace Gmeng {
    struct t_display {
        Objects::coord pos; int v_cursor;
        int v_width; int v_height;
        std::vector< std::vector<std::string> > v_drawpoints;
    };
    struct t_charpos { int p1; /* x axis */ int p2; /* y axis */ };
};

static Objects::coord v_conv_1dicp(int cpos) {};
static int v_conv_2dvcp(Objects::coord cpos) {};

static void _udisplay_init(Gmeng::t_display display) {
    int v_size = display.v_width * display.v_height;
    for ( int i = 0; i < v_size; i++ ) display.v_drawpoints.push_back(" ");
};

static bool gm_validate_dvs1dc(Gmeng::t_display __d) {
    if (__d.v_width * __d.v_height != __d.v_drawpoints.size()) return false;
    return true;
};



static void _uchar_control(Gmeng::t_display display, Gmeng::t_charpos _px) {
    if (!gm_valodate_dvs1dc(display)) throw std::invalid_argument("gmeng/utils *termui -> _uchar_control >> gm_validate_dvs1dc return FALSE : 'display' object parsed into method _uchar_control (with type Gmeng::t_display) is invalid.");
};

static void _utext(Gmeng::t_display display, std::string _ln) {
    
};

static void _wmap

#endif
