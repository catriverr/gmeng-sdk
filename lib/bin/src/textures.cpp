#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <array>
#include <vector>
#include <algorithm>
#include <fstream>

#include "../gmeng.h"

#define stob(str) (str == std::string("true") || str.substr(1) == std::string("true"))
#define cpps(str) ( std::string(str) )



namespace Gmeng {
    inline void set_texturemap(Gmeng::texture &g_tx, Gmeng::texture &g_tx2) {
        __functree_call__(Gmeng::set_texturemap);
        int indx = 0;
        for ( const auto& unit : g_tx2.units ) {
            g_tx.units[indx] = unit;
            indx++;
        };
    };
    inline Gmeng::texture LoadTexture(std::string __fname) {
        __functree_call__(Gmeng::LoadTexture);
        std::ifstream file(__fname); Gmeng::texture texture;
        if (!file.is_open()) {
            gm_log(Gmeng::colors[4] + "[gm0:core] Gmeng::LoadTexture(): could not load texturemap file " + __fname + ": e_file_unavailable");
            return { 0, 0, false, {}, "NO_CONTENT_FILE_INVALID" };
        };


        std::vector<std::string> lines; std::string line;
        while (std::getline(file, line)) { lines.push_back(line); };
        file.close(); int i = 0;
        if (Gmeng::global.dont_hold_back) gm_log(g_joinStr(lines, "\n"));
        for ( const auto& ln : lines ) {
            // break if all units have been rendered
            // fixes broken textures and also allows copying
            // textures as linked to a megatexture but scaled down.
            // (may be implemented in the future)
            if ( i != 0 && i == (texture.width*texture.height)+1 ) break;
            if (startsWith(ln, ";")) continue; // comment
            if (ln.length() < 5) continue;
            // ln1: name=str,width=number,height=number
            // rest: Gmeng::Unit
            if ( i == 0 ) {
                // case_of: texture_data definition
                std::vector<std::string> posx = g_splitStr(ln, ",");
                // name=string (name= length = 5)
                texture.name  = posx[0].substr(5);
                // width=size_t (width= length = 6)
                texture.width = std::stoi(posx[1].substr(6));
                // height=size_t (height= length =  7)
                texture.height = std::stoi(posx[2].substr(7));
                // collision=bool (collision= length = 10)
                texture.collidable = stob(posx[3].substr(10));
            } else {
                // case_of: unit definition
                // same as the map.hpp unit declaration
                // but this declaration does not include an xy pos
                // since delegate units of a texture are written line-by line.
                // so 0,0 will be line 0, 0,1 will be 1, 1,1 will be 2 and so on.
                // it will also not include the following information:
                // - collision modifier
                // - is_entity modifier
                // - is_player modifier
                // special unit data will be included.
                // so, final string of a texture unit delegate is:
                // CLRID SPECIAL SPECIAL_CLR SPECIAL_CHAR
                //   2    false     3            x
                std::vector<std::string> unit_ln = g_splitStr(ln, " ");
                Gmeng::Unit cur_unit; int l_i = 0;
                for ( const auto& param : unit_ln ) {
                    if (l_i == 0 && param == "__gtransparent_unit__") { cur_unit.transparent = true; break; }; // no value is pushed back since this is a parameter loop, doing so would push the current_unit value twice to the vector.
                    cur_unit.collidable = texture.collidable;
                    cur_unit.is_entity = false; cur_unit.is_player = false;
                    if ( l_i == 0 ) { cur_unit.color = std::stoi(param);       };
                    if ( l_i == 1 ) { cur_unit.special = stob(param);          };
                    if ( l_i == 2 ) { cur_unit.special_clr = std::stoi(param); };
                    if ( l_i == 3 ) { cur_unit.special_c_unit = param;         };
                    l_i++;
                };
                texture.units.push_back(cur_unit);
            };
            i++;
        };
        gm_log("texture '" + texture.name + "' of size " + v_str(texture.width) + "x" + v_str(texture.height) + " compiled from source " + __fname);
        return texture;
    };
};

#define __GMENG_TEXTUREMGR_INIT__ true
