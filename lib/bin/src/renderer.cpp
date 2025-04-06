#pragma once
#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <map>
#include <unordered_map>
#include <random>
#include <filesystem>
#include <sstream>
#include "../gmeng.h"
#include "../src/textures.cpp"

#include "../utils/serialization_def.cpp"
#ifndef _WIN32
#include <termios.h>
#include <sys/select.h>
#endif
#include <unistd.h>
#include <typeinfo>
#include <functional>

namespace fs = std::filesystem;


namespace Gmeng {
    /// All Renderer utilities, structs, objects
    /// controllers, handlers and more.
    namespace Renderer {

        struct drawpoint { int x; int y; };
        struct viewpoint { drawpoint start; drawpoint end; };

        /// Generates an empty texture with a pink-black layer
        /// (the default null layer), can be placed into a Model
        /// to become a textureless object.
        inline Gmeng::texture generate_empty_texture(int width, int height) {
            __functree_call__(Gmeng::Renderer::generate_empty_texture);
            Gmeng::texture __t; __t.width = width; __t.height = height; __t.collidable = false;
            int color_value = 0;
            for (int i = 0; i < height; i++) {
                for (int j = 0; j < width; j++)
                    __t.units.push_back(Gmeng::Unit {
                        .color = j % 2 == color_value ? Gmeng::color_t::PINK : Gmeng::color_t::BLACK
                    });
                if (color_value == 0) color_value++;
                else color_value = 0;
            };
            return __t;
        };

        /// Models
        ///
        /// @since 4.1-glvl
        class Model {
            private:
              inline Objects::coord get_pointXY(int pos) {
                  __functree_call__(Gmeng::Renderer::Model::get_pointXY);
                  int __p1_x = (pos % this->width);
                  int __p1_y = (pos / this->width);
                  return Objects::coord {
                        .x=__p1_x,
                        .y=__p1_y
                  };
              };
            public:
              std::size_t width; std::size_t height; std::size_t size; drawpoint position;
              std::string name; Gmeng::texture texture; int id;
              /// Resets the texture into the default null-layer with the same height
              /// as the previously attached texture.
              ///
              /// @since 4.1-glvl
              inline void reset_texture() {
                  __functree_call__(Gmeng::Renderer::Model::reset_texture);
                  this->texture = generate_empty_texture(this->width, this->height);
              };

              /// Attaches a given texture to the model.
              ///
              /// @since 4.1-glvl
              inline void attach_texture(Gmeng::texture __t) {
                  __functree_call__(Gmeng::Renderer::Model::attach_texture);
                  this->texture = __t; this->width = __t.width; this->height = __t.height;
              };

              /// Loads a texture from a file.
              ///
              /// @since 4.1-glvl
              inline void load_texture(std::string __tf) {
                  __functree_call__(Gmeng::Renderer::Model::load_texture);
                  this->texture = Gmeng::LoadTexture(__tf);
                  this->width = this->texture.width; this->height = this->texture.height;
              };

              /// sets the border of the model.
              /// Models do not have borders by default.
              /// this method can be used for debug_render rendering modes.
              ///
              /// @since 10.4.0-d
              inline void set_border(color_t border_color) {
                  // no functree call here
                  for (int x, y = 0; (y*width)+x < width*height; x++) {
                      if (x != 0 && x % width == 0) y++, x = 0;
                      if ( y == 0        || x == 0      ||
                              y == height-1 || x == width-1   ) this->texture.units.at(x) = {
                          .color = border_color, .transparent = false, .special = false
                      };
                  };
              };
        };

        inline Model generate_empty_model(int width, int height) {
            __functree_call__(Gmeng::Renderer::generate_empty_model);
            int id = g_mkid();
            return Model {
                .width=static_cast<std::size_t>(width), .height=static_cast<std::size_t>(height), .size=static_cast<std::size_t>(width*height),
                .position = { 0, 0 }, .name = "unknown_" + v_str(id),
                .texture = generate_empty_texture(width, height), .id = id
            };
        };













        struct LevelBase {
            Gmeng::texture lvl_template; std::size_t width; std::size_t height;
        };

        struct gnmdl {
            Model m;
            v_title txname;
        };

        /// parses a string.gmdl file to a gmdl
        gnmdl parse_gmdl(std::string _indx) {
            __functree_call__(Gmeng::Renderer::parse_gmdl);
            /// syntax:
            /// mdl_0x px=0  py=0  w=10   h=10    tx=tx_0x
            /// name   posX  posY  width  height  texture
            std::vector<v_title> v_index = g_splitStr(_indx, " ");
            gnmdl returnvalue;
            returnvalue.m.id = Gmeng::CONSTANTS::vl_nomdl_id; // set until values are initialized, so if any errors occur the model is set as null
            returnvalue.m.name = v_index[0];
            returnvalue.m.position.x = std::stoi(v_index[1].substr(3));
            returnvalue.m.position.y = std::stoi(v_index[2].substr(3));
            returnvalue.m.width  = std::stoi(v_index[3].substr(2));
            returnvalue.m.height = std::stoi(v_index[4].substr(2));
            returnvalue.m.size = returnvalue.m.width*returnvalue.m.height;
            returnvalue.txname = v_index[5].substr(3);
            returnvalue.m.id = g_mkid();
            return returnvalue;
        };

        // compiles Renderer::Model objects into a Gmeng::Unit vector
        vector<Unit> draw_model(Model __m) {
            //__functree_call__(Gmeng::Renderer::draw_model);
            std::vector<Gmeng::Unit> unitmap;
            if (Gmeng::global.dont_hold_back) gm_log("job_render *draw_model -> MODEL * size: " + v_str(__m.width * __m.height) + " units | TEXTURE * size: " + v_str(__m.texture.units.size()) + " units");
                for ( int i = 0; i < __m.height; i++ ) {
                    for ( int j = 0; j < __m.width; j++ ) {
                        int vpos = (i*__m.width)+j; unitmap.push_back(__m.texture.units[vpos]);
                        if (Gmeng::global.dont_hold_back) gm_log("job_render *draw_model (_vlinear_draw=true) -> vp_compileUnit (from<texture>->units to<unitmap>->vector) : unit * vpos = " + v_str(vpos+1) + " / " + v_str(__m.texture.units.size()));
                    };
                };

            if (Gmeng::global.dont_hold_back) gm_log("job_render (draw_model -> vp_compileUnit v_success) v_static_cast<std::vector<units>> total_size : " + v_str(unitmap.size()));
            return unitmap;
        };

        /// returns size of a drawpoint in total amount of units (NOT FOR X,Y POSITIONS)
        /// for coordinate sizes, use trace_1dp(msize, mapwidth)
        /// this method asumes that the given drawpoint is not a position, but
        /// the total size of a canvas.
        /// For example:
        /// drawpoint { 5,5 } represents an object with the width of 5 and height of 5,
        /// so Gmeng::Renderer::getsize returns 25 for this object.
        /// this means that this drawpoint does not represent the coordinate 5,5;
        /// however an object with the size of 5,5.
        std::size_t getsize(drawpoint c) {
            //__functree_call__(Gmeng::Renderer::getsize);
            return (
                        c.x < 1 ?
                        ( c.y ) :
                        ( c.x *  c.y )
                   );
        };

        /// returns drawpoint object as log string
        std::string conv_dp(drawpoint p) {
            //__functree_call__(Gmeng::Renderer::conv_dp);
            return "x=" + v_str(p.x) + ",y=" + v_str(p.y);
        };

        // returns placement coordinates for each coordinate of object at __p with size of __s in mapsize of __ws
        vector<Renderer::drawpoint> get_placement(drawpoint __p, drawpoint __s, drawpoint __ws, bool _vlinear_render = false) {
            //__functree_call__(Gmeng::Renderer::get_placement);
            gm_log("gm::v_renderer -> get_placement : pvalues = 1: " + conv_dp(__p) + " 2: " + conv_dp(__s) + " 3: " + conv_dp(__ws));
            if (getsize(__p) > getsize(__ws) || getsize(__s) > getsize(__ws)) throw std::invalid_argument("placement parameters invalid");
            std::vector<Gmeng::Renderer::drawpoint> vec;
            if (_vlinear_render) {
                if ((__p.x + __s.x <= __ws.x) && (__p.y + __s.y <= __ws.y)) {
                    for (int i = 0; i < __s.x; ++i) {
                        for (int j = 0; j < __s.y; ++j) {
                            Gmeng::Renderer::drawpoint point = {.x=__p.x + i, .y=__p.y + j};
                            vec.push_back(point);
                        };
                    };
                } else { std::cerr << "Gmeng::Renderer::get_placement: e_obj: out of boundaries: __getsize(__p, __s, __ws);" << std::endl; };
            } else {
                if ((__p.x + __s.x <= __ws.x) && (__p.y + __s.y <= __ws.y)) { /// horizontal render
                    for (int j = 0; j < __s.y; ++j) {
                        for (int i = 0; i < __s.x; ++i) {
                            Gmeng::Renderer::drawpoint point = {.x=__p.x + i, .y=__p.y + j};
                            vec.push_back(point);
                        }
                    }
                } else { std::cerr << "Gmeng::Renderer::get_placement: e_obj: out of boundaries: __getsize(__p, __s, __ws);" << std::endl; };
            };
            ASSERT("pref.log", p_no);
            return vec;
        };

        /// returns placement coordinates for a viewpoint within a map sizeof drawpointxy
        vector<drawpoint> get_displacement(viewpoint& view, const drawpoint& map) {
            __functree_call__(Gmeng::Renderer::get_displacement);
            std::vector<drawpoint> result;

            // Ensure the viewpoint is within the map boundaries
            view.start.x = std::max(view.start.x, 0); view.end.x = std::min(view.end.x, map.x - 1);
            view.start.y = std::max(view.start.y, 0); view.end.y = std::min(view.end.y, map.y - 1);
            for (int y = view.start.y; y <= view.end.y; ++y) {
                for (int x = view.start.x; x <= view.end.x; ++x) { result.push_back({x, y}); };
            };
            return result;
        };

        viewpoint c_npos = viewpoint {
            .start = drawpoint { .x=-1, .y=-1 },
            .end   = drawpoint { .x=-1, .y=-1 },
        };

        // type decl
        class Display {
            private:
            public:
                Camera<0, 0> camera; viewpoint viewpoint;
                std::size_t width; std::size_t height; vector<Unit> rendered_units;

                inline void set_cursor_visibility(bool state) {
                    if (!state) std::cout << "\e[?25l"; // hide
                    else std::cout << "\e[?25h"; // show
                };

                inline void set_resolution(std::size_t width, std::size_t height) {
                    __functree_call__(Gmeng::Renderer::Display::set_resolution);
                    this->width = width; this->height = height;
                    this->camera.SetResolution(this->width, this->height);
                };
                inline void refresh() {
                    __functree_call__(Gmeng::Renderer::Display::refresh);
                    // refresh resolution
                    this->camera.SetResolution(this->width, this->height);
                };
                inline void plunit(Objects::G_Player pl, Objects::coord plcoords) {
                    __functree_call__(Gmeng::Renderer::Display::plunit);
                    auto v_playerunit = this->rendered_units[(plcoords.x*this->camera.w)+plcoords.y];
                    std::cout << this->camera.draw_unit(v_playerunit) << " " << Gmeng::colors[pl.colorId] << pl.colorId << endl;
                    this->rendered_units[(plcoords.x*this->camera.w)+plcoords.y] = Gmeng::Unit {
                        .color=v_playerunit.color,
                        .collidable=v_playerunit.collidable,
                        .is_player=true,
                        .player = pl,
                        .special=true,
                        .special_clr=pl.colorId
                    };
                };
                inline void nplunit(Objects::coord coords) {
                    __functree_call__(Gmeng::Renderer::Display::nplunit);
                    this->rendered_units[coords.y*this->camera.w + coords.x].is_player = false;
                };
                inline void draw(Objects::G_Player pl, Objects::coord plcoords) {
                    __functree_call__(Gmeng::Renderer::Display::draw);
                    this->refresh();
                    if (!this->camera.player_init) this->camera.SetPlayer(0, pl, plcoords.y, plcoords.x, true);
                    else this->camera.MovePlayer(0, plcoords.y, plcoords.x);
                    gm_log("Gmeng::Renderer::Display job_render *draw -> total compiled units: " + v_str(this->rendered_units.size()));
                    for (int i = 0; i < this->rendered_units.size(); i++) {
                        if (this->camera.display_map.unitmap[i].is_player) {
                            this->camera.playerunit.color = this->rendered_units[i].color; /// fix for __gtransparent_unit__ background
                            continue;
                        };
                        this->camera.display_map.unitmap[i] = this->rendered_units[i];
                    };
                    this->camera.update();
                    this->camera.clear_screen();
                    std::cout << this->camera.draw() << endl;
                };
                inline void move_to(Gmeng::Renderer::viewpoint __vp) {
                    __functree_call__(Gmeng::Renderer::Display::move_to);
                    this->viewpoint = __vp;
                };
        };
    };
    // chunk of a level
    // similar to how minecraft handles chunks
    // in Gmeng::Level::wm_render() a std::vector<Gmeng::r_chunk> chunks is loaded
    // these chunks contain { Gmeng::Renderer::viewpoint vp; } metadata
    // which in Gmeng::Level::Display::draw() are located.
    // The chunk's std::vector<Gmeng::Renderer::Model> models object is recieved
    // and compiled into a Gmeng::Camera::unitmap;
    struct chunk {
        Renderer::viewpoint vp;
        vector<Renderer::Model> models;
    };
    // levelinfo (parsed into Gmeng::Level::load_level after Gmeng::parse_glvl())
    struct LevelInfo {
        Renderer::LevelBase base; std::string name; vector<int> display_res;
        vector<chunk> chunks; std::string description;
    };
    struct VectorView {
        std::map<int, std::vector<Gmeng::Unit>> vectors; std::size_t width = 1; std::size_t height = 1;
    };
    struct RenderBuffer {
        std::vector<Gmeng::Unit> units; Renderer::drawpoint pos; bool linear = false;
    };
    enum LinearRenderBufferPositionController {
        SIDE_RIGHT  = 0, SIDE_LEFT = 1
    };
    enum HorizontalRenderBufferPositionController {
        SIDE_BOTTOM = 0, SIDE_TOP  = 1
    };
    /// methods to join RenderBuffer output together
    /// only linear and horizontal since we dont have to account for a Camera viewpoint
    /// being bigger than the size of current_chunk in r_chunk->chunks(0) - it is limited.
    /// std::vector<string> as &new_delegate since a single character may have color defining escape codes in it
    inline std::string _ujoin_unit_linear(std::string& current, std::vector<std::string>& new_delegate, LinearRenderBufferPositionController pos) {
        __functree_call__(Gmeng::_ujoin_unit_linear);
        int _vpos = 0;
        std::vector<std::string> c_lines = g_splitStr(current, "\n");
        switch (pos) {
            case SIDE_RIGHT:
                for (const auto& line : c_lines) {
                    if (_vpos > new_delegate.size()-1) break;
                    c_lines[_vpos] = line + new_delegate[_vpos];
                   _vpos++;
                };
                return g_joinStr(c_lines, "\n");
                break;
            case SIDE_LEFT:
                for (const auto& line : c_lines) {
                    if (_vpos > new_delegate.size()-1) break;
                    c_lines[_vpos] = new_delegate[_vpos] + line;
                   _vpos++;
                };
                return g_joinStr(c_lines, "\n");
                break;
        };
    };
    inline std::string _ujoin_unit_horizontal(std::string& current, std::vector<std::string>& new_delegate, HorizontalRenderBufferPositionController pos) {
        __functree_call__(Gmeng::_ujoin_unit_horizontal);
        switch (pos) {
            case SIDE_TOP:
                return g_joinStr(new_delegate, "") + "\n" + current;
                break;
            case SIDE_BOTTOM:
                return current + "\n" + g_joinStr(new_delegate, "");
                break;
        };
    };

    inline Objects::coord trace_1dp(int xy, int wmp_x) {
        __functree_call__(Gmeng::trace_1dp);
        int width  = xy % wmp_x; int height = xy / wmp_x;
        return Objects::coord { .x = width, .y = height };
    };
    /// logs information about units within vp_units as vector, po_size vp_units wsizeX
    inline void log_vpu(std::vector<Gmeng::Unit>& vp_units, int wsizeX) {
        __functree_call__(Gmeng::log_vpu);
        int jndx = 0;
        for (const auto& _v_unit : vp_units) { Objects::coord gp = Gmeng::trace_1dp(jndx, wsizeX); gm_nlog("vp_gtx.units @ pos "+ v_str(jndx) + " ("+v_str(gp.y)+","+v_str(gp.x)+"):\n\tv_color -> " + v_str(_v_unit.color) + "\n\tv_collision -> " + (_v_unit.collidable ? "true" : "false") + "\n\tv_special -> " + (_v_unit.special ? "true" : "false") + "\n\tv_spcolor -> " + v_str(_v_unit.special_clr) + "\n"); jndx++; };
        gm_nlog("\n");
    };
    /// logs information about a viewpoint chunk
    inline void log_vpc(Gmeng::chunk vp_chunk) {
        __functree_call__(Gmeng::log_vpc);
        gm_nlog("vp_chunk->vpc_info : r_chunk & Gmeng::r_chunk & Gmeng::Renderer::Model & Gmeng::Texture & gm_vpcontrol\n");
        gm_nlog("metadata:\n");
        gm_nlog("\tv_viewpoint -> start = " + Gmeng::Renderer::conv_dp(vp_chunk.vp.start) + " - end = " + Gmeng::Renderer::conv_dp(vp_chunk.vp.end) + "\n");
        gm_nlog("\tv_models:\n");
        for (const auto& v_mdl : vp_chunk.models) {
            gm_nlog("\t\tm_name -> " + v_mdl.name + "\n");
            gm_nlog("\t\tm_id -> " + v_str(v_mdl.id) + "\n");
            gm_nlog("\t\tm_width -> " + v_str(v_mdl.width) + "\n");
            gm_nlog("\t\tm_height -> " + v_str(v_mdl.height) + "\n");
            gm_nlog("\t\tm_posX -> " + v_str(v_mdl.position.x) + "\n");
            gm_nlog("\t\tm_posY -> " + v_str(v_mdl.position.y) + "\n");
            gm_nlog("\t\tm_size -> " + v_str(v_mdl.size) + "\n");
            gm_nlog("\t\tm_texture -> " + v_mdl.texture.name + "\n");
            gm_nlog("\t\tpreview:");
            Gmeng::Camera<10, 10>* preview_cam = new Gmeng::Camera<10, 10>();
            std::vector<Gmeng::Unit> v_preview = Gmeng::Renderer::draw_model(v_mdl);
            for (int v_lndx = 0; v_lndx < v_preview.size(); v_lndx++) {
                if (v_lndx % v_mdl.width == 0) gm_nlog("\n\t\t");
                gm_nlog(preview_cam->draw_unit(v_preview[v_lndx]));
            };
            gm_nlog("\n");
        };
    };

    inline const Renderer::Model nomdl = {
        .id=CONSTANTS::vl_nomdl_id
    };
    inline const texture notxtr = {
        .name=v_str(CONSTANTS::vl_notxtr_id)
    };

    template<typename v_type>
    struct vd_item {
        v_intl id; v_type data;
    };

    template<typename v_type>
    struct v_dictl {
        public:
            std::vector<Gmeng::vd_item<v_type>> values;
            /// returns size of the dictionary
            inline int size(bool diff = 0) { return this->values.size()-(diff ? 1 : 0); };
            /// returns value at the specified index of the dictionary
            inline v_type indx(v_intl pos) { return (this->values[pos]).data; };
            /// emplaces a value at the end of the dictionary
            inline void addb_l(v_type val) { this->values.push_back(vd_item<v_type> {.id=g_mkid(),.data=val}); };
            /// returns the vector of the dictionary
            inline std::vector<Gmeng::vd_item<v_type>> v_getrelative() { return this->values; };
    };

    inline Renderer::Model vd_find_model(v_dictl<Renderer::Model> dict, v_title name) {
        __functree_call__(Gmeng::vd_find_model);
        for (const auto& val : dict.v_getrelative()) {
            if (val.data.name == name) return val.data;
        };
        return Gmeng::nomdl;
    };
    inline texture vd_find_texture(v_dictl<texture> dict, v_title name) {
        __functree_call__(Gmeng::vd_find_texture);
        int ccount = 0;
        for (const auto& val : dict.v_getrelative()) {
            if (Gmeng::global.dont_hold_back) gm_log("gm::vd_find_texture() -> v_searchAddr: v_dictl<gm::texture> vgm_defaults asumed | addr: " + _uconv_1ihx(ccount) + " -> relative_value = " + val.data.name + " , search_value = " + name + " , match = " + std::string(val.data.name == name ? "yes" : "no"));
            if (val.data.name == name) return val.data;
            ccount++;
        };
        return Gmeng::notxtr;
    };
    namespace vgm_defaults {
        v_dictl<Renderer::Model> vg_rdmodels;
        v_dictl<texture>         vg_textures;
    };
    /// reads a folder's .gmdl and .gtx files into Gmeng::vgm_defaults::vg_rdmodels & vg_textures;
    inline void _uread_into_vgm(const v_title& folder) {
        __functree_call__(Gmeng::_uread_into_vgm);
        // we wait until all textures are in vgm_defaults before we load any models
        // afterwards we loop through the vector and load them into Gmeng::vgm_defaults::vg_rdmodels
        std::vector<std::string> q_modelindx;
        if (Gmeng::global.dont_hold_back) gm_log("_uread_into_vgm() : job_readfile *loop -> ffolder:name = " + folder);
        try {
            for (const auto& entry : fs::directory_iterator(folder)) {
                if (Gmeng::global.dont_hold_back) gm_log("_uread_into_vgm() : reading file " + entry.path().string() + " - size: " + v_str(entry.file_size()) + "B | " + v_str(entry.file_size() / 1024) + "KB | " + v_str(entry.file_size() / 1024 / 1000) + " MB");
                if (entry.is_regular_file()) {
                    v_title fname = (entry.path().string());
                    if (Gmeng::global.dont_hold_back) gm_log("_uread_into_vgm() : status_update -> entryAt = " + folder = " | filename: " + entry.path().filename().string());
                    if (endsWith(fname, ".gmdl")) { std::string fcontent = g_readFile(fname); q_modelindx.push_back(fcontent); continue; };
                    vgm_defaults::vg_textures.addb_l(Gmeng::LoadTexture(fname));
                };
            }
            for (const auto& q_item : q_modelindx) {
                Gmeng::Renderer::gnmdl vmdl = Renderer::parse_gmdl(q_item);
                vmdl.m.attach_texture(vd_find_texture(vgm_defaults::vg_textures, vmdl.txname));
                Gmeng::vgm_defaults::vg_rdmodels.addb_l(vmdl.m);
            };
        } catch (const fs::filesystem_error& ex) {
            gm_err(1, "g,gm,gmeng,Gmeng -> _uread_into_vgm(const v_title& folder) : status v_error -> vgm data in folder " + std::string(ex.what()) + " is corrupt/faulty.");
        }
    };

    /// writes values in Gmeng::vgm_defaults::vg_rdmodels & vg_textures into vectors v_mdls and v_txtrs
    inline void _uwritevgm_to_maps(std::map<v_title, Gmeng::Renderer::Model>& v_mdls, std::map<v_title, Gmeng::texture>& v_txtrs) {
        __functree_call__(Gmeng::_uwritevgm_to_maps);
        for (const auto& mdl : vgm_defaults::vg_rdmodels.v_getrelative()) { v_mdls[mdl.data.name] = mdl.data; };
        for (const auto& txtr : vgm_defaults::vg_textures.v_getrelative()) { v_txtrs[txtr.data.name] = txtr.data; };
    };

    inline LevelInfo parse_glvl(std::string __fn) {
        __functree_call__(Gmeng::parse_glvl);
        /// syntax:
        /// ln 1-4 are headers
        /// ln 0 | name=level_name
        /// ln 1 | desc=level_description [ **n equals newline ]
        /// ln 2 | base_texture=file_name
        /// ln 3 | chunk_size=x,y
        /// ------------------------------------------------------
        /// rest | DEFINING TEXTURES
        /// rest | #texture name path_to_texture.gt
        /// rest | DEFINING MODELS
        /// rest | #model name px=0 py=0 w=5 h=3 tx=texture_name
        /// rest | DEFINING CHUNKS
        /// rest | #chunk p1x=0 p1y=0 p2x=4 p2y=4 model_name1,model_name2
        /// ------------------------------------------------------
        /// conf | all texture and model names will include vgm
        /// conf | vgm (vgm_defaults nspace) includes default textures
        /// conf | if we are checking for a texture name to be
        /// conf | specified, we will check for the name in vgm_defaults
        /// conf | before loading any files. Also, any filefolder can
        /// conf | be included in vgm_defaults (coming in 5.1)
        /// conf | with the method vgm_include(const v_title& ffolder) -> auto;
        std::string f_content = g_readFile(__fn); Gmeng::LevelInfo info;
        std::vector<std::string> lines = g_splitStr(f_content, "\n");
        gm_log("parsing glvl file: " + __fn + " -> Gmeng::LevelInfo");
        std::map<std::string, Gmeng::texture> textures;
        std::map<std::string, Gmeng::Renderer::Model> models;
        Gmeng::_uread_into_vgm("./envs/models");
        Gmeng::_uwritevgm_to_maps(models, textures);
        std::vector<int> resolution = {5, 5};
        gm_log("glvl_v vector_init -> success");
        int indx = 0;
        gm_log("glvl_v inl loadF load_file start");
        for ( const auto& ln : lines ) {
            /// indx equ ln_num
            if (startsWith(ln, ";")) continue;
            std::string ln_vcomments = g_splitStr(ln, ";")[0];
            std::vector<std::string> params = g_splitStr(ln_vcomments, " ");
            gm_log(__fn + ": gl_v->line @ " + v_str(indx) + ": " + ln);
                 if ( indx == 0 ) { info.name = params[0].substr(5); gm_log("glvl->name : SETNAME = " + params[0] + " -> success\n"); }
            else if ( indx == 1 ) { info.description = params[0].substr(5); gm_log("glvl->desc : SETDESC = " + params[0] + " -> success\n"); }
            else if ( indx == 2 ) {
                gm_log("glvl->base.template : SETTEXTURE = " + params[0] + " -> proc.init() -> success");
                std::string vp_name = params[0].substr(13);
                Gmeng::texture vp_gtx;
                if (textures.find(vp_name) != textures.end()) vp_gtx = textures[vp_name];
                else vp_gtx = Gmeng::LoadTexture(vp_name);
                gm_log("glvl->base.tx = " + vp_name + " -> LOADTEXTURE -> success");
                gm_log("glvl->base.template >> vp_info:");
                Gmeng::log_vpu(vp_gtx.units, vp_gtx.width);
                info.base.width = vp_gtx.width;
                info.base.height = vp_gtx.height;
                info.base.lvl_template = vp_gtx;
            }
            else if ( indx == 3 ) { gm_log("glvl->p_xy : initl() -> start"); std::vector<std::string> p_xy = g_splitStr(params[0].substr(11), ","); gm_nlog("v_chunksize -> " + params[0].substr(11) + "\nv_chX -> " + p_xy[0] + "\nv_chY -> " + p_xy[1]); resolution[0] = std::stoi(p_xy[0]); resolution[1] = std::stoi(p_xy[1]); };
            if ( indx >  3 && ln.length() > 0) {
                std::string keyword = params[0];
                if ( keyword == "#model" ) {
                    if (params.size() < 7) { std::cerr << "gm:0/c_err: Gmeng::parse_glvl -> kw_model : ERR_PARAMS: at(line="+ v_str(indx) +",ch="+v_str(ln.length())+") -> expected 7 params, got " + v_str(params.size()) + "." << endl; indx++; continue; };
                    std::string _m_name       = params[1];
                    int _m_posX               = std::stoi(params[2].substr(3));
                    int _m_posY               = std::stoi(params[3].substr(3));
                    int _m_width              = std::stoi(params[4].substr(2));
                    int _m_height             = std::stoi(params[5].substr(2));
                    Gmeng::texture _m_texture = textures [params[6].substr(3)];
                    models[_m_name] = Gmeng::Renderer::Model {
                        .width=static_cast<std::size_t>(_m_width),
                        .height=static_cast<std::size_t>(_m_height),
                        .size=static_cast<std::size_t>(_m_posX*_m_posY),
                        .position = { .x=_m_posX,.y=_m_posY },
                        .name=_m_name,
                        .texture=_m_texture,
                        .id=g_mkid()
                    };
                    gm_log("glvl->models : push_back() -> v_static_cast<std::size_t> : *m_texture:load *m_metadata:load v_status -> success");
                }
                else if ( keyword == "#texture" ) {
                    if (params.size() < 2) { std::cerr << "gm:0/c_err: Gmeng::parse_glvl -> kw_texture : ERR_PARAMS: at(line="+ v_str(indx) +",ch="+v_str(ln.length())+") -> expected 2 params, got " + v_str(params.size()) + "." << endl; indx++; continue; };
                    if (textures.find(params[2]) != textures.end()) textures[params[1]] = textures[params[2]];
                    else textures[params[1]] = Gmeng::LoadTexture(params[2]);
                    gm_log("glvl->textures : push_back() -> v_static_cast<std::size_t> : *m_texture:load *m_metadata:load v_status -> success");
                }
                else if ( keyword == "#chunk" ) {
                    /// rest | #chunk p1x=0 p1y=0 p2x=4 p2y=4 model_name1,model_name2
                    if (params.size() < 6) { std::cerr << "gm:0/c_err: Gmeng::parse_glvl -> kw_chunk : ERR_PARAMS: at(line="+ v_str(indx) +",ch="+ v_str(ln.length()) +") -> expected 6 params, got " + v_str(params.size()) + "." << endl; indx++; continue; };
                    std::vector<Gmeng::Renderer::Model> ls_models;
                    std::vector<std::string> vp_models = g_splitStr(params[5], ",");
                    for (const auto& vp_model : vp_models ) ls_models.push_back( models[vp_model] );
                    info.chunks.push_back(Gmeng::chunk {
                        .vp = {
                            .start = { .x=std::stoi(params[1].substr(4)), .y=std::stoi(params[2].substr(4)) },
                            .end   = { .x=std::stoi(params[3].substr(4)), .y=std::stoi(params[4].substr(4)) }
                        },
                        .models = ls_models
                    });
                    gm_log("glvl->chunks : push_back() -> v_static @ this->trace_1dp(v_static_cast<std::size_t>) : v_static @ this->trace_1dp(v_static_cast<Gmeng::r_chunk>) -> gm_conv_1dc() & r_chunk # v_status -> success");
                }
                else {
                    if (params.size() > 0) std::cerr << "gm:0/c_err: Gmeng::parse_glvl() -> ERR_INVALID_KEYWORD: at(line=" + v_str(indx) +",ch=0)" << endl;
                }
            };
            indx++;
        };
        info.display_res.push_back(resolution[0]);
        info.display_res.push_back(resolution[1]);
        return info;
    };

    static const Objects::G_Player v_base_player = (Objects::G_Player {
        .entityId=0,
        .colorId = 0,
        .colored=true,
        .c_ent_tag = "o",
    });

    using std::vector, std::string;

    /// started work on implementing rendering
    /// options for SDL2 screens.
    /// @since 10.2.0-d
    enum Renderer_Type { CONSOLE, EXTERNAL };
    /// Type decl
    class EntityBase;

    struct Entity_Interaction {
      public:
        enum Interaction_Type {
            LMB_CLICK, RMB_CLICK,
            MMB_CLICK, KEYPRESS,
            PROXIMITY, SCRIPT,
            UNKNOWN_INTERACTION
        };

        Interaction_Type type;
        std::unique_ptr<EntityBase>* interacted_by;
    };

    /// type decl
    class Level;
    class EventLoop;

    class EntityBase {
      protected:
        // Entity derivatory class id assignment
        // this is increasingly added each time
        // a new class derives from 'Entity'.
        // with the Entity class.
        static int id_counter;


        template <typename Derived>
        static void register_derived_class(int id) {
            get_derived_factory()[id] = []() { return std::make_unique<Derived>(); };
        };
      public:
        static std::unordered_map<int, std::function<std::unique_ptr<EntityBase>()>>& get_derived_factory() {
            static std::unordered_map<int, std::function<std::unique_ptr<EntityBase>()>> factory_map;
            return factory_map;
        }

        // well, brave calling this a "sprite".
        texture sprite;
        // position of the Entity.
        // The position is not CENTRAL.
        // The 'position' value returns the position of the 1st unit of the
        // entity.
        // For example:
        //
        // 'o' for map, 'x' for entity units
        //
        // oooo
        // ooXx
        // ooxx
        //
        // the capitalised 'X' is the position value.
        // So in this case it would be (x=2,y=1)
        Renderer::drawpoint position;
        // random id, to differenciate entities
        unsigned int entity_id = g_mkid();
        // interaction proximity,
        // the amount of units a player should be
        // in proximity to the entity for the
        // entity->interact() function to be called.
        //
        // Defaults to 5
        unsigned int interaction_proximity = 5;

        /// Entity creation
        EntityBase() = default;

        /// Entity from a texture and position.
        EntityBase(texture model_, Renderer::drawpoint pos_) :
            sprite(model_), position(pos_) {};

        /// Entity from a model, but with a set position
        /// instead of the value found in Model::position.
        EntityBase(Renderer::Model model_, Renderer::drawpoint pos_) :
            sprite(model_.texture), position(pos_) {};

        /// Entity from a model,
        /// converts from a Model object to an Entity.
        EntityBase(Renderer::Model model_) :
            sprite(model_.texture), position(model_.position) {};

        /// Default deconstructor
        virtual ~EntityBase() = default;

        /// Returns the serialization id of the
        /// derived Entity class.
        virtual int get_serialization_id() const = 0;
        /// Default serialization function.
        /// Classes can override this function too.
        virtual void serialize(std::ostream& out) const {
            out.write(reinterpret_cast<const char*>(&entity_id), sizeof(entity_id));
            out.write(reinterpret_cast<const char*>(&interaction_proximity), sizeof(interaction_proximity) );

            // cant read-write drawpoints yet, so done manually
            out.write(reinterpret_cast<const char*>(&position.x), sizeof(position.x));
            out.write(reinterpret_cast<const char*>(&position.y), sizeof(position.y));

            serialize_texture(sprite, out);
        };
        /// Default deserialization function.
        /// Classes can override this function too.
        virtual void deserialize(std::istream& in) {
            in.read(reinterpret_cast<char*>(&entity_id), sizeof(entity_id));
            in.read(reinterpret_cast<char*>(&interaction_proximity), sizeof(interaction_proximity) );

            // cant read-write drawpoints yet, so done manually
            in.read(reinterpret_cast<char*>(&position.x), sizeof(position.x));
            in.read(reinterpret_cast<char*>(&position.y), sizeof(position.y));

            deserialize_texture(sprite, in);
        };

        /// Method to be called when requesting a texture
        /// for the entity.
        ///
        /// This is used because the Entity::sprite texture
        /// may not be finalized.
        ///
        /// The draw function allows for changes in the sprite
        /// with the ability to take into account other variables
        /// like the EventLoop states.
        ///
        /// e.g. zombies eyes turn red when the player is
        /// within 5 units of proximity
        virtual texture draw( Renderer_Type type = EXTERNAL ) {
            // default behaviour for drawing the entity.
            // can be extended with subclasses
            return this->sprite;
        };

        /// Method to be called when any interaction with
        /// an entity takes place.
        ///
        /// Interactions have proximity, using the
        /// Entity::interaction_proximity value.
        virtual void interact( Entity_Interaction, Level* ) { /* no default behaviour */ };

        /// Method to be called periodically at every FIXED_UPDATE event
        /// overriding this method allows you to give functionality & AI to entities.
        ///
        /// Examples of this can be following the player,
        /// checking for proximity with other entities, etc.
        virtual void periodic( EventLoop* ) { /* no default behaviour */ };

        /// This method is called periodically every 250ms for all entities.
        /// It is to provide animation capabilities with a stable interval.
        ///
        /// This way, you do not have to rely on the periodic() method to animate
        /// the texture of an entity.
        ///
        /// NOT REQUIRED! Animated sprites are optional.
        virtual void animate( EventLoop* ) { /* no default behaviour */ };
    };
    /// sets off the derivatory id count
    int EntityBase::id_counter = 0;

    /// Virtual class for Extendable Entity
    /// classes.
    ///
    /// Derive this class with:
    /// ```
    /// class MyEntityType : public Entity<MyEntityType> {
    ///
    /// };
    /// ```
    /// and add behaviour to interact(), draw(), animate()
    /// and etc to your new entity type.
    template <typename Derived>
    class Entity : public EntityBase {
      public:
        /// DERIVED CLASS ID
        static const int id;

        Entity() {
            // register the new class (that extends this) to the derived class factory
            Entity::register_derived_class<Derived>(id);
        };
        /// returns the serialization id (so the derived class id)
        /// of the derivatory class that extends Entity : EntityBase.
        int get_serialization_id() const override {
            return id;
        };
    };

    /// Set derivatory class ID.
    template <typename Derived>
    const int Entity<Derived>::id = EntityBase::id_counter++;

    template<typename Derived>
    class DerivedEntityRegistrar {
      public:
        DerivedEntityRegistrar() {
            Derived eclass;
        };
    };
/// Registers an entity type.
/// Must be called for all new Derived Entity types.
///
/// If you do not register entity types, save files will seg-fault
/// when loading due to no initial registry to the factory having been
/// made. This ensures the factory has an overload for the Derived Class.
#define REGISTER_ENTITY_TYPE( eclass ) static DerivedEntityRegistrar<eclass> derivatory__##eclass


    /// Player Entity object.
    /// Stable for every gmeng instance.
    ///
    /// Extend this class to add different functionality.
    class Player : public Entity<Player> {
      public:
        int interaction_proximity = 10;
        Renderer::drawpoint position = { 0, 0 };

    }; REGISTER_ENTITY_TYPE( Player );






    class Level {
        private:
            // compiles a chunk into a vector<Unit> unitmap for a Camera instance to render.
            // this method can also be used externally to render any chunk, it does not require
            // for the chunk to  be a member of the Level class.
            //
            // currently used in get_renderscale, get_lvl_view & emplace_lvl_camera.
            //
            // Does not render entities!
            inline vector<Unit> render_chunk(chunk chunk) {
                __functree_call__(Gmeng::Level::__private__::render_chunk);
                // write base_template skybox image to chunk (level 0 of canvas)
                // base_template's viewpoint relative to the chunks viewpoint will be drawn as a base 'skybox' like image
                texture* base_map = &this->base.lvl_template; vector<Unit> units;

                    gm_log("init OK" );
                    gm_log("base_map -> WIDTH: " + v_str(base_map->width) + " - HEIGHT: " + v_str(base_map->height) );

                std::vector<Gmeng::Renderer::drawpoint> wrapper = Gmeng::Renderer::get_displacement(chunk.vp, Renderer::drawpoint {.x=static_cast<int>(base_map->width),.y=static_cast<int>(base_map->height)});

                    gm_log("get_displacement OK" );
                    gm_log("chunk viewpoint -> start -> x: " + v_str(chunk.vp.start.x) + " - y: " + v_str(chunk.vp.start.y) + " | end -> x: " + v_str(chunk.vp.end.x) + " - y: " + v_str(chunk.vp.end.y) );
                    gm_log("wrapper viewpoints -> " + v_str(wrapper.size()) );
                    gm_log("get_displacement -> " + v_str(wrapper.size()) + " drawpoints" );
                    gm_log("BASE_MAP_LENGTH: " + v_str(base_map->units.size()));

                ASSERT("pref.log", Assertions::vd_assert::OFF);

                // compiles the units from every drawpoint within
                // the chunk's wrapper and finds them from the base_map.
                for ( const auto& dp : wrapper ) {
                    int vpos = (dp.y * base_map->width)+dp.x;
                    // units.at(x) provides error logging and throws exceptions.
                    // otherwise if the base_map is empty units[x] would
                    // cause a segmentation fault and exit the program.
                    Gmeng::Unit v_unit = base_map->units.at(vpos);

                    units.push_back(v_unit);
                    if (Gmeng::global.dont_hold_back)
                        gm_log("job_render *render_chunk -> v_chunkwrapper vpos at("+v_str(vpos)+") out of vp_chunk::constl::base_template");
                };

                // log some information about the chunkwrapper
                if (global.dont_hold_back)
                    gm_log("job_render *render_chunk -> v_chunkwrapper compileUnits: v_success\ncl_preview:"),
                    gm_log("write_drawpoint -> base_map.units OK" );

                /* there was a for loop for a preview, but since it's unused it has been removed since 10.4.0-b. */

                // compile models into unitmaps and write them to x,y coordinates
                // within the r_chunk by getting placement positions with get_placement
                int debug_render_mode = this->display.camera.modifiers.get_value("debug_render");
                for ( auto _model : chunk.models ) {
                    auto model = _model;
                    if (debug_render_mode > 0) {
                        for (int i = 0; i < model.width*model.height; i++) {
                            int y = i / model.width, x = i % model.width;
                            if (y == 0 || x == 0 ||
                                y == model.height-1 || x == model.width-1 ) model.texture.units.at(i) = {
                                .color = (debug_render_mode == 2 ? YELLOW : RED), .collidable = true, .transparent = false, .special = false
                            };
                        };
                    };
                    Renderer::drawpoint dp = model.position;

                    /// 10.4.0 - this log isn't really needed
                    /// so it has been moved to full-log only
                    /// mode.
                    if (global.dont_hold_back)
                        gm_log("dp_loadmodel " + v_str(model.id) + " OK" ),
                        gm_log("dp -> x: " + v_str(dp.x) + " - y: " + v_str(dp.y) ),
                        gm_log("p2 -> x: " + v_str(model.width) + " - y: " + v_str(model.height) );

                    vector<Renderer::drawpoint> displacement = Renderer::get_placement(dp, {.x=static_cast<int>(model.width),.y=static_cast<int>(model.height)}, {.x=(chunk.vp.end.x - chunk.vp.start.x),.y=(chunk.vp.end.y - chunk.vp.start.y)});


                    gm_log("total_drawpoints: " + v_str(displacement.size()) );
                    gm_log("get_placement OK" );

                    vector<Unit> unitmap = Renderer::draw_model(model);

                    gm_log("draw_model OK: unitmap.size(): " + v_str(unitmap.size()) );

                    int lndx = 0, i2 = 0;
                    for ( auto unit : unitmap ) {

                        if (unit.transparent) {
                            if (this->display.camera.modifiers.get_value("debug_render") == 2) {
                                unit.transparent = false; unit.color = GREEN;
                            } else { lndx++; continue; };
                        } else {
                            if (this->display.camera.modifiers.get_value("debug_render") == 2) {
                                unit.color = unit.collidable ? WHITE : BLACK;
                            };
                        };

                        if (global.dont_hold_back) // 10.4.0 - this log isn't really needed either, full log only mode
                            gm_log(v_str(lndx) +" <- pos_vdp: rendering_model_unit PREVIEW: " + this->display.camera.draw_unit(unit) );

                        int _vdp_pos = ((displacement[lndx].y)*(chunk.vp.end.x - chunk.vp.start.x + 1))+displacement[lndx].x;

                        // 10.4.0 - this log was also moved to full log only mode
                        if (Gmeng::global.dont_hold_back) {
                            if (_vdp_pos < (displacement).size()) {
                                gm_log("_vdp_pos find: " + v_str(_vdp_pos) +" OK" );
                                gm_log("_vdp_current -> x: " + v_str(displacement[_vdp_pos].x) + " - y: " + v_str(displacement[_vdp_pos].y) );
                                gm_log("_vdp_current_addr -> " + this->display.camera.draw_unit(units[_vdp_pos]) );
                                gm_log("swap_unit: at(" + v_str(_vdp_pos) +") -> PREVIEW: " + this->display.camera.draw_unit(units[_vdp_pos]) + " TO unit() -> PREVIEW: " + this->display.camera.draw_unit(unit) );
                            } else {
                                gm_log(Gmeng::colors[YELLOW] + "WARN!" + Gmeng::colors[WHITE] + " possible invalid _vdp_pos, clarification methods disabled");
                                gm_log(Gmeng::colors[CYAN] + "TRACE to pointer of " + Gmeng::colors[GREEN] + "_vdp_pos" + Gmeng::colors[CYAN] + ":");
                                gm_log("@== " + Gmeng::colors[PINK] + "_vdp_pos" + Gmeng::colors[CYAN] + " ==@");
                                gm_log(colors[WHITE] + "\t#0 " + Gmeng::colors[WHITE] + _uconv_1ihx(_uget_addr(displacement)) + " " + Gmeng::colors[GREEN] + "displacement(" + v_str(displacement.size()) + ") [ lndx ]");
                                gm_log(colors[WHITE] + "\t#1 " + Gmeng::colors[WHITE] + _uconv_1ihx(_uget_addr(displacement[lndx].x)) + "\t" + Gmeng::colors[CYAN] + "*(self) || coord.y_pointer [" + v_str(displacement[lndx].y)+ "] * this->display.width [ " + v_str(this->display.width) + " ] " + colors[PINK] + "== " + colors[WHITE] + v_str(displacement[lndx].y * this->display.width));
                                gm_log(colors[WHITE] + "\t#2 " + Gmeng::colors[WHITE] + _uconv_1ihx(_uget_addr(displacement[lndx].y)) + "\t" + Gmeng::colors[CYAN] + "*(self) || coord.x_pointer [" + v_str(displacement[lndx].x) +"]");
                                gm_log(colors[CYAN] + "@== formulae -> " + Gmeng::colors[GREEN] + "(displacement[lndx].y*this->display.width)+displacement[lndx].x" + Gmeng::colors[CYAN] + " ==@");
                                gm_log(colors[YELLOW] + "WARN!" + colors[WHITE] + " consider investigation of this ccode, since it may occur when _vdp_pos is outbound from sizeof(displacement)");
                            };
                            gm_log(v_str(unitmap.size()) + " ACCESS_TO_INDEX: " + v_str(lndx) + " APPLIED_FROM_SIZED: " + v_str(_vdp_pos) + " TO_SIZED_VECTOR_OF: " + v_str(units.size()));
                            gm_log("set_unit_at(id: " + v_str(_vdp_pos) + ") OK" );
                        };

                        units.at(_vdp_pos) = (
                            this->display.camera.modifiers.get_value("debug_render") == 2 ?
                            unit : unitmap[lndx]
                        );
                        lndx++;
                    };
                };

                gm_log("job_render *render_chunk completed, status: v_success");
                gm_log("total units: " + v_str(units.size()));
                return units;
            };

            // retrieves the chunk at the given id from
            // level->chunks.at(id) and returns it.
            //
            // @since 4.1-glvl
            inline chunk& get_chunk(int id) {
                __functree_call__(Gmeng::Level::__private__::get_chunk);
                return this->chunks.at(id); // .at(x) is segfault-proof
            };

            // Replaces the chunk at level->chunks.at(id)
            // to the given chunk.
            //
            // @since 4.1-glvl
            inline void set_chunk(int id, chunk& chunk) {
                __functree_call__(Gmeng::Level::__private__::set_chunk);
                this->chunks[id] = chunk; // vector[x] can be used here because we're setting it
            };

            /// @deprecated We don't use Objects::G_Player as the player anymore
            ///
            /// calculates the viewpoint of the camera depending on the player object,
            /// for 4.1-glvl framework applications only. Kept for backwards compatibility
            inline Renderer::viewpoint calculate_camera_viewpoint() {
                return { -1, -1 };
                /*
                __functree_call__(Gmeng::Level::__private__::calculate_camera_viewpoint);
                Renderer::drawpoint start; Renderer::drawpoint end;
                start.x = this->plcoords.x - ((this->display.width -1)/2);
                start.y = this->plcoords.y - ((this->display.height-1)/2);
                end.x   = this->plcoords.x + ((this->display.width -1)/2);
                end.y   = this->plcoords.y + ((this->display.height-1)/2);
                while (start.x > this->base.lvl_template.width && start.x != 0)  start.x--;
                while (start.y > this->base.lvl_template.height&& start.y != 0)  start.y--;
                while (end.x   > this->base.lvl_template.width &&   end.x != 0)    end.x--;
                while (end.y   > this->base.lvl_template.height &&  end.y != 0)    end.x--;
                return { start, end };
            */};
        public:
            /// Base texture for the Level.
            /// This object is treated as a skybox.
            ///
            /// It covers all chunks and is the first layer
            /// (so the backmost layer) in every chunk as well.
            Renderer::LevelBase base;
            /// Display for the level. Contains the Camera
            /// and the display properties like the viewpoint,
            /// camera width, height and resolution as well
            /// as the camera ModifierList.
            Renderer::Display display;
            /// Chunk vector of the level. Contains every
            /// chunk registered to the level.
            ///
            /// Chunks contain models and
            /// are spliced with get_renderscale when
            /// drawing the level display.
            vector<chunk> chunks;
            /// Entities in the level. Contains
            /// every entity registered t4o the level.
            ///
            /// Entities are not bound with chunks.
            vector<std::unique_ptr<EntityBase>> entities;
            /// Description of the level.
            ///
            /// can be set with 4.1 text-based GLVL files
            /// with desc="<description>" on the file headers.
            std::string desc;
            /// Name of the level.
            ///
            /// can be set by 4.1 text-based GLVL files
            /// with name="<name>" on the file headers.
            std::string name;

            /// Loads a chunk from the given param,
            /// and returns the id that the chunk is assigned to.
            ///
            /// can be called with get_chunk(id) or level->chunks.at(id).
            /// can be cast to void, (void)load_chunk(...). The id value
            /// is not crucial to keep as long as no further changes will
            /// be made to the chunk after its addition.
            ///
            /// @since 4.1-glvl
            inline int load_chunk(chunk chunk) {
                __functree_call__(Gmeng::Level::load_chunk);
                this->chunks.push_back(chunk);
                return (this->chunks.size()-1);
            };

            /// Loads a level from a LevelInfo header.
            /// Only used for 4.1-glvl framework applications.
            ///
            /// If you're on higher than 10.2.0-d, this method
            /// is not required (not that it will work either).
            /// 10.2.0-d was when gmeng switched to binary level
            /// files from text-based parsed level files.
            ///
            /// Instead, import the `utils/serialization.cpp`
            /// header and use the `read_level_data` method
            /// to read from a Level binary instead.
            ///
            /// @since 4.1-glvl
            /// @deprecated +10.2.0-d, use read_level_data().
            inline void load_level(Gmeng::LevelInfo __glvl) {
                __functree_call__(Gmeng::Level::load_level);
                int i,j = 0;
                gm_log("vp_loadlevel: __glvl chunks v_size: " + v_str(__glvl.chunks.size()));
                /// load the chunks from the LevelInfo to the Level (*this)
                for (auto& chunk : __glvl.chunks) {
                    Gmeng::log_vpc(chunk);
                    if (i >= this->chunks.size()) { this->load_chunk(chunk); continue; };
                    this->chunks[i] = chunk;
                    i++;
                };
                /// set all values in the base texture.
                this->base.height = __glvl.base.height;
                this->base.width  = __glvl.base.width;
                this->display.set_resolution(__glvl.display_res[0], __glvl.display_res[1]);
                this->base.lvl_template.collidable = __glvl.base.lvl_template.collidable;
                this->base.lvl_template.height = __glvl.base.height;
                this->base.lvl_template.width  = __glvl.base.width;
                gm_log("class Model { ... } : load_level(...) -> base_template @ h+w = " + v_str(__glvl.base.height) + "," + v_str(__glvl.base.width));
                this->base.lvl_template.name   = __glvl.base.lvl_template.name;
                /// TODO: inefficient to for loop for this, we can just set the texture
                /// with LoadTexture. however it'll be kept like this for now due to logging.
                for ( const auto& _v_unit : __glvl.base.lvl_template.units ) {
                    if (j >= this->base.lvl_template.units.size()) { this->base.lvl_template.units.push_back(_v_unit); j++; continue; };
                    this->base.lvl_template.units[j] = _v_unit;
                    gm_log("class Gmeng::Level { ... } : load_level(...) -> status v_success : job_push_back, cntrl.display.template_units LOAD at("+v_str(j)+")");
                    j++;
                };
                this->desc = __glvl.description;
                this->name = __glvl.name;
                this->set_player(Objects::G_Player {
                    .entityId=0,
                    .colorId=3,
                    .colored=true,
                    .c_ent_tag="o"
                }, 0, 0);
            };

            /// @deprecated
            ///
            /// Sets the current image for 1 frame.
            /// Also places the units to the unitmap (once again, for 1 frame).
            inline void set_image(vector<Unit> units) {/*
                __functree_call__(Gmeng::Level::set_image);
                std::copy(units.begin(), units.end(), this->display.camera.display_map.unitmap);
                this->display.draw(this->player, this->plcoords);
            */};

            /// For 4.1-glvl. Draws the selected chunk to the camera.
            /// This method focuses on chunk-based movement
            /// and does not contain any code for spliced
            /// chunks. This means that the camera can not
            /// move between chunks, and nor can any entities/players.
            ///
            /// If you are on a higher version than 7.0.0, do not use
            /// this method if you want spliced chunks, whole-level
            /// rendering and cubic render (1x1 square units instead
            /// of 2x1 rectangle units).
            ///
            /// Instead, use the `get_renderscale`, `get_lvl_view`
            /// and `emplace_lvl_camera` methods  found in the
            /// `renderer.cpp` utility (automatically imported by gmeng).
            ///
            /// For more information on renderscale rendering, visit
            /// https://gmeng.org/?doc=Getting%20Started.
            inline void draw_camera(int chunk_id) {/*
                __functree_call__(Gmeng::Level::draw_camera);
                ASSERT("pref.log", DISABLE());
                if (chunk_id < 0 || chunk_id > chunks.size()) throw std::invalid_argument("chunk_id is invalid");
                Gmeng::chunk chunk = this->chunks[chunk_id];
                gm_log("render_chunk start" );
                std::vector<Gmeng::Unit> vr_unit = this->render_chunk(chunk);
                for (const auto& cv_unit : vr_unit) { gm_log("render_chunk -> result:success v_success -> preview_units: " + this->display.camera.draw_unit(cv_unit)); };
                gm_log("render_chunk end" );
                int v_camview = 0;
                for (const auto& v_unit : vr_unit) {
                    gm_log("class Gmeng::Level { ... } : load_level(...) -> status v_success : job_push_back, cntrl.display.rendered_units LOAD at("+v_str(v_camview)+")");
                    if (v_camview >= this->display.rendered_units.size()) { this->display.rendered_units.push_back(v_unit); v_camview++; continue; };
                    this->display.rendered_units[v_camview] = v_unit;
                    v_camview++;
                };
                this->display.set_resolution(this->display.width, this->display.height);
                this->display.draw(this->player, this->plcoords);
            */};

            /// Returns the rendered output of a chunk
            /// at the given id.
            ///
            /// @since 4.1-glvl
            inline vector<Unit> get_rendered_chunk(int id) {
                __functree_call__(Gmeng::Level::get_rendered_chunk);
                return this->render_chunk(this->chunks[id]);
            };

            /// Renders a given chunk with the internal chunk
            /// renderer.
            ///
            /// @since 7.0.0-d
            inline vector<Unit> v_render_chunk(chunk chunk) {
                __functree_call__(Gmeng::Level::v_render_chunk);
                return this->render_chunk(chunk);
            };

            /// refreshes the current chunk display,
            /// resetting the resolution and draws.
            ///
            /// @deprecated, use get_lvl_view instead.
            /// this method is only intended for 4.1-glvl.
            ///
            /// Any version higher than 7.0.0 should use
            /// the renderscale functions instead.
            ///
            /// @since 4.1-glvl
            inline void refresh() {/*
                this->display.set_resolution(this->display.width, this->display.height);
                this->display.draw(this->player, this->plcoords);
            */};

            /// @deprecated, gmeng does not use G_Player unit objects
            /// as players & entities are being reworked for 11.0.0.
            ///
            /// @since 4.1-glvl
            inline void set_player(Objects::G_Player p, int x, int y) {/*
                __functree_call__(Gmeng::Level::set_player);
                if (this->display.camera.player_init) this->display.nplunit(this->plcoords);
                this->player = p; this->plcoords = { .x=x, .y=y };
                gm_log("r_level::set_player *inline,static -> v_success ; r_level::player (Objects::G_Player).coords = @pos(" + v_str(this->player.coords.x)+","+v_str(this->player.coords.y)+")");
                this->move_player(x, y);
            */};

            /// @deprecated, gmeng does not use G_Player unit objects
            /// as players & entities are being reworked for 11.0.0.
            ///
            /// @since 4.1-glvl
            inline void move_player(int x, int y) {/*
                __functree_call__(Gmeng::Level::move_player);
                this->plcoords.x=x; this->plcoords.y=y;
                this->display.viewpoint = this->calculate_camera_viewpoint();
            */};
    };
    /// checks if a viewpoint contains a sub-viewpoint
    /// this code is horrifying to edit, we should just hope that it will never cause issues
    inline std::vector<Renderer::drawpoint> viewpoint_includes(const Renderer::viewpoint &v_control, const Renderer::viewpoint &v_check) {
        __functree_call__(Gmeng::viewpoint_includes);
        std::vector<Renderer::drawpoint> __shared__;
        // Check if v_checks start point is within control
        if (v_check.start.x >= v_control.start.x && v_check.start.y >= v_control.start.y &&
            v_check.start.x <= v_control.end.x && v_check.start.y <= v_control.end.y) {
            __shared__.push_back(v_check.start);
        }

        // Check if v_checks end point is within control
        if (v_check.end.x >= v_control.start.x && v_check.end.y >= v_control.start.y &&
            v_check.end.x <= v_control.end.x && v_check.end.y <= v_control.end.y) {
            __shared__.push_back(v_check.end);
        }

        // Check if v_checks start and end points overlap control
        if (v_check.start.x <= v_control.end.x && v_check.end.x >= v_control.start.x &&
            v_check.start.y <= v_control.end.y && v_check.end.y >= v_control.start.y) {
            // Determine shared drawpoints within the overlapping region
            int sharedStartX = std::max(v_check.start.x, v_control.start.x);
            int sharedStartY = std::max(v_check.start.y, v_control.start.y);
            int sharedEndX = std::min(v_check.end.x, v_control.end.x);
            int sharedEndY = std::min(v_check.end.y, v_control.end.y);

            __iterator_outer__: for (int x = sharedStartX; x <= sharedEndX; ++x) {
                __iterator_inner__: for (int y = sharedStartY; y <= sharedEndY; ++y) {
                    __shared__.push_back({x, y});
                }
            };
        };

        return __shared__;
    };

    /// checks if a viewpoint includes a drawpoint.
    inline bool viewpoint_includes_dp(Renderer::viewpoint vp, Renderer::drawpoint dp) {
        //__functree_call__(Gmeng::viewpoint_includes_dp);
        if (dp.x >= vp.start.x && dp.x <= vp.end.x &&
            dp.y >= vp.start.y && dp.y <= vp.end.y) return true;
        return false;
    };

    /// PROXIMITY UTILS

    /// Proximity Info (used for calculate_proximity() calls)
    struct Proximity_Info {
        unsigned int proximity;
        Renderer::viewpoint path;
    };

    /// Calculates the proximity between two objects
    /// given their position and size values.
    ///
    /// The calculated value is the gap between
    /// the first object and the second object.
    ///
    /// posA, sizeA for first object
    Proximity_Info calculate_proximity(
        const Renderer::drawpoint& posA, const Renderer::drawpoint& sizeA,
        const Renderer::drawpoint& posB, const Renderer::drawpoint& sizeB
    ) {
        using namespace Renderer;

        // Compute borders of A
        int leftA   = posA.x;
        int rightA  = posA.x + sizeA.x;
        int topA    = posA.y;
        int bottomA = posA.y + sizeA.y;

        // Compute borders of B
        int leftB   = posB.x;
        int rightB  = posB.x + sizeB.x;
        int topB    = posB.y;
        int bottomB = posB.y + sizeB.y;

        // Calculate horizontal and vertical proximity (border-to-border)
        int dx = 0;
        if (rightA < leftB) {
            dx = leftB - rightA;
        } else if (rightB < leftA) {
            dx = leftA - rightB;
        }

        int dy = 0;
        if (bottomA < topB) {
            dy = topB - bottomA;
        } else if (bottomB < topA) {
            dy = topA - bottomB;
        }

        unsigned int proximity = std::sqrt(dx * dx + dy * dy);

        // Determine the closest border points
        drawpoint start{
            std::clamp(posA.x + (dx == 0 ? sizeA.x / 2 : (dx > 0 ? sizeA.x : 0)),
                       leftA, rightA),
            std::clamp(posA.y + (dy == 0 ? sizeA.y / 2 : (dy > 0 ? sizeA.y : 0)),
                       topA, bottomA)
        };

        drawpoint end{
            std::clamp(posB.x + (dx == 0 ? sizeB.x / 2 : (dx < 0 ? sizeB.x : 0)),
                       leftB, rightB),
            std::clamp(posB.y + (dy == 0 ? sizeB.y / 2 : (dy < 0 ? sizeB.y : 0)),
                       topB, bottomB)
        };

        return Proximity_Info{ proximity, viewpoint{ start, end } };
    };

    /// Returns the shortest grid path with every immediate step needed
    /// to be taken to reach a viewpoint's end point from the start point.
    ///
    /// Based on Bresenam's line algorithm.
    vector<Renderer::drawpoint> shortest_path(const Renderer::viewpoint& view) {
        using namespace Gmeng::Renderer;
        vector<drawpoint> path;

        int x0 = view.start.x; int y0 = view.start.y;

        int x1 = view.end.x; int y1 = view.end.y;

        int dx = std::abs(x1 - x0); int dy = std::abs(y1 - y0);

        int sx = (x0 < x1) ? 1 : -1; int sy = (y0 < y1) ? 1 : -1;

        int err = dx - dy;

        while (true) {
            path.push_back({x0, y0});
            if (x0 == x1 && y0 == y1) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; x0 += sx; }
            if (e2 < dx)  { err += dx; y0 += sy; }
        };

        return path;
    };




    /// @deprecated not used internally | DOES NOT RETURN CORRECT VALUES
    /// @deprecated use the renderscale utility
    inline vector<Unit> trace_render_partial(Level& level_t, int id, vector<Renderer::drawpoint> drawpoints) {
        __functree_call__(Gmeng::__deprecated_do_not_use__::trace_render_partial);
        vector<Unit> raw_chunk = level_t.get_rendered_chunk(id);
        vector<Unit> __partial__;
        for (const auto& dp : drawpoints) {
            int index = dp.y * level_t.base.width + dp.x;
            if (index < static_cast<int>(raw_chunk.size())) {
                /// FIXME: THIS WOULD NOT WORK
                /// index points to a value INSIDE the camera's viewpoint.
                /// If the camera's viewpoint is bigger than the size of the chunk's size, it would be wrong since
                /// index would be pointing to the drawpoint in a level-scope where the raw_chunk is a vector and
                /// its values should be treated in the chunk-scope. TODO: add _vscale_accordingly_to_viewpoint(&int, &viewpoint_chunk, &viewpoint_lvl)
                __partial__.push_back(raw_chunk[index]);
            };
        };
        return __partial__;
    };

    /// traces a chunk's display position in a vector
    /// @deprecated use the renderscale utility
    inline vector<chunk> trace_chunk_vector(Level& level_t) {
        __functree_call__(Gmeng::__deprecated_do_not_use__::trace_chunk_vector);
        vector<chunk> displays; Camera<0, 0> *pCamera = &(level_t.display.camera);
        Renderer::Display *pDisplay = &(level_t.display);
        int __iterator_count__ = 0;
        __iterate_through__: for (const auto& chunk : level_t.chunks) {
            vector<Renderer::drawpoint> shared_delegates = viewpoint_includes(pDisplay->viewpoint, chunk.vp);
            if (shared_delegates.size() < 1) continue;
            // chunk exists in camera viewpoint, add it to vectorview
            displays.push_back(chunk);
            __iterator_count__++;
        };
        return displays;
    };

    /// combines render buffers into a list
    inline vector<Unit> splice_render_buffers(vector<chunk> chunks, Level &level_t) {
        __functree_call__(Gmeng::__deprecated_do_not_use__::splice_render_buffers);
        vector<Unit> units;
        for (const auto& chunk : chunks) {
            vector<Renderer::drawpoint> shared_delegates = viewpoint_includes(level_t.display.viewpoint, chunk.vp);
            if (shared_delegates.size() < 1) continue;
            vector<Unit> v_units = level_t.v_render_chunk(chunk);
            units.insert(units.end(), v_units.begin(), v_units.end());
        };
        return units;
    };

    inline vector<Renderer::drawpoint> chromatize_viewpoint(Renderer::viewpoint vp) {
        __functree_call__(Gmeng::chromatize_viewpoint);
        vector<Renderer::drawpoint> _vpos;
        /// x, y < |MATH_DELTA( VP_END, VP_START )| (viewpoint_width)
        /// transcend_drawpoint_to_viewpoint(); __viewpoint_controller__();
        for (int y = 0; y < vp.end.y - vp.start.y; y++) {
            for (int x = 0; x < vp.end.x - vp.start.x; x++) {
                _vpos.push_back({
                    .x = x + (vp.end.x - vp.start.x),
                    .y = y + (vp.end.y - vp.start.y)
                });
            };
        };
        return _vpos;
    };

    /// traces the position of a drawpoint within a viewpoint.
    inline Renderer::drawpoint trace_drawpoint_in_viewpoint(Renderer::viewpoint vp, Renderer::drawpoint dp) {
        __functree_call__(Gmeng::trace_drawpoint_in_viewpoint);
        vector<Renderer::drawpoint> vp_pos = chromatize_viewpoint(vp);
        return {
            .x = vp_pos[dp.y * (vp.end.x - vp.start.x) + dp.x].x,
            .y = vp_pos[dp.y * (vp.end.x - vp.start.x) + dp.x].y
        };
    };

    /// @deprecated not used internally | NOT IMPLEMENTED
    /// trace_chunk_vector implements optimization itself.
    ///
    /// @deprecated use the renderscale utility instead
    inline void _voptimize_chunk_vector(Level& level_t) {
        __functree_call__(Gmeng::__no_impl__::_voptimize_chunk_vector);
    };

    /// sorts a chunk vector according to its levels' viewpoints ( correct_formed_list )
    /// @deprecated not used internally, but returns values as expected
    ///
    /// @deprecated use the renderscale utility instead
    inline vector<chunk> _vsort_chunk_vector(Level& level_t) {
        __functree_call__(Gmeng::__deprecated_do_not_use__::_vsort_chunk_vector);
        vector<chunk> chunks;
        int __base_width__ = level_t.base.width;
        int __iterator_value__ = 0;
        int __next_viewpoint_value__ = 0;
        __iterate_through__: for (const auto& chunk : level_t.chunks) {
            if (__next_viewpoint_value__ != 0 && __next_viewpoint_value__ != chunk.vp.start.y * level_t.base.width + chunk.vp.start.x) continue;
            chunks.push_back(chunk);
            __next_viewpoint_value__ = chunk.vp.start.y * level_t.base.width + chunk.vp.start.x;
            __iterator_value__++;
        };
        return chunks;
    };

    /// compiles a level's display viewpoint into a vector of units
    /// (splicing different parts of r_chunk renderbuffers into one Camera instance)
    ///
    /// @deprecated use the renderscale utility instead
    inline vector<Unit> _vgen_camv_fv2cv(Level &level_t) {
        __functree_call__(Gmeng::__deprecated_do_not_use__::_vgen_camv_fv2cv);
        Renderer::Display *pDisplay = &level_t.display; Camera<0,0> *pCamera = &level_t.display.camera;
        vector<chunk> chunks = Gmeng::trace_chunk_vector(level_t);
        vector<Unit> units = Gmeng::splice_render_buffers(chunks, level_t);
        vector<Unit> v_units_final;
        int x = 0, y = 0;
        for (const auto& unit : units) {
            int unit_dpi_in_level = y * level_t.base.width + x;
            if (viewpoint_includes_dp(pDisplay->viewpoint, { x, y })) v_units_final.push_back(unit);
            x++;
            if (x % level_t.base.width == 0) { x = 0; y++; };
        };
        return v_units_final;
    };

    /// returns delta of a viewpoint's X coordinate
    template <typename number_type = int>
    inline number_type _vcreate_vp2d_deltax(Renderer::viewpoint vp) {
        return v_static_cast<number_type>( (vp.end.x - vp.start.x) );
    };

    /// returns delta of a viewpoint's Y coordinate
    template <typename number_type = int>
    inline number_type _vcreate_vp2d_deltay(Renderer::viewpoint vp) {
        return v_static_cast<number_type>( (vp.end.y - vp.start.y) );
    };

    template <typename... Args>
    auto vp_width(Args&&... args) -> decltype(_vcreate_vp2d_deltax(std::forward<Args>(args)...)) {
        return _vcreate_vp2d_deltax(std::forward<Args>(args)...);
    };

    template <typename... Args>
    auto vp_height(Args&&... args) -> decltype(_vcreate_vp2d_deltay(std::forward<Args>(args)...)) {
        return _vcreate_vp2d_deltay(std::forward<Args>(args)...);
    };

    /// returns position of a 2d pointer in a vector as a 1d index to the vector
    /// example:
    /// { 0, 1, 2,
    ///   3, 4, 5 }
    /// width = 3,
    /// pointer = (1, 1)
    /// y (1) * width (3) + x (1) => pos(4)
    /// vector[4] = 4
    template <typename number_type = int>
    inline number_type _vcreate_vu2d_delta_xy(int x, int y, int width) {
        __functree_call__(_vcreate_vu2d_delta_xy);
        return v_static_cast<number_type>( ( y * width ) + x );
    };

    /// @deprecated USE get_lvl_view(&level) INSTEAD
    /// returns a frame generated by a level_t object
    /// chronological execution:
    ///     ->  _vcamv_gen_frame( LEVEL(1) )
    ///         -> _vgen_camv_fv2cv( &LEVEL(1) )
    ///             -> trace_chunk_vector( &LEVEL(1) ) => vector< CHUNK(0) > chunks
    ///             -> splice_render_buffers( chunks ) => vector< UNIT(0) > units
    ///         -> _vcreate_vp2d_deltay, _vcreate_vp2d_deltax, _vcreate_vu2d_delta_xy
    inline std::string _vcamv_gen_frame(Level level_t) {
        ASSERT("pref.log", DISABLE());
        __functree_call__(Gmeng::__deprecated_do_not_use__::_vcamv_gen_frame);
        gm_log("_vcamv_gen_frame() -> registering job_render to process queue [ __gmeng_invoke_expr__, __gmeng_vcamv_gen_frame__, __gmeng_renderer__ ]");
        std::string __final__ = "";
        gm_log("_vcamv_gen_frame() job_render -> process has been recognized by the thread");
        gm_log("_vcamv_gen_frame() job_render *calc_vunits (static_parameter __ATTEMPT__ ) -> args: &__this_function__::forward(params) : attempting method _vgen_camv_fv2cv");
        std::vector<Unit> units = Gmeng::_vgen_camv_fv2cv(level_t);
        gm_log("_vcamv_gen_frame() job_render *calc_vunits (static_parameter __ATTEMPT__) -> static parameter returned v_success ; vector unit compiled");
        Gmeng::Camera<0, 0>* pCameraRenderer = &level_t.display.camera;
        gm_log("_vcamv_gen_frame() job_render *draw_vunits -> looping through DELTA( *(level)->display.vpoint.forward(x,y) ");
        for (int y = 0; y < Gmeng::_vcreate_vp2d_deltay(level_t.display.viewpoint); y++) {
            gm_log("_vcam_gen_frame() job_render *draw_vunits -> __ROW__ display row #" + v_str(y));
            for (int x = 0; x < Gmeng::_vcreate_vp2d_deltax(level_t.display.viewpoint); x++) {
                gm_log("_vcam_gen_frame() job_render *draw_vunits -> __COLUMN__ display column #" + v_str(x));
                __final__ += pCameraRenderer->draw_unit(
                                units[ _vcreate_vu2d_delta_xy(x, y,
                                       _vcreate_vp2d_deltax(level_t.display.viewpoint)) ]
                             );
            };
            __final__ += "\n";
        };
        return __final__;
    };

    /// chromatized chunks are rendered chunks of a level
    /// that share the same viewpoint.start.y coordinates.
    /// bound by level's base_template header width & height.
    /// example:
    /// level's base_template header: { width=9, height=2 }
    /// level->chunks: [render chunk (Gmeng::r_chunk)]
    ///   { "1", "2", "3",
    ///     "7", "8", "9",
    ///     "d", "e", "f", },
    ///
    ///   { "4", "5", "6",
    ///     "a", "b", "c",
    ///     "g", "h", "i", }
    /// parsed into _vp2d_scalar_size and _vconcatenate_lvl_chunks(&p)
    /// chromatized_chunks: [chormatized chunk (__CHROMATIZED_CHUNK_CONTROLLER_VIEWPOINT__)]
    ///   { "1", "2", "3", "4", "5", "6", "7", "8", "9" },
    ///   { "a", "b", "c", "d", "e", "f", "g", "h", "i" }
    /// width = 9units, height: 2rows
    struct __CHROMATIZED_CHUNK_CONTROLLER_VIEWPOINT__ {
        vector<Unit> data;
        Renderer::viewpoint vp;
    };

    /// returns the width of a viewpoint
    inline int _cc1d_scalar_size(Renderer::viewpoint vp) {
        __functree_call__(Gmeng::_cc1d_scalar_size);
        return ( _vcreate_vp2d_deltax(vp) * 0x1 );
    };

    /// mirror: reverses a vector
    template<typename __vtype__>
    inline void mirror(std::vector<__vtype__>& __v, int cc = 0) {
        __functree_call__(Gmeng::mirror);
        cc % 2 == 0 ?
            std::reverse(__v.begin(), __v.end()) :
            std::reverse(__v.end(), __v.begin()) ;
    };

    /// mirror: returns a reversed version of a vector
    template<typename __vtype__>
    inline vector<__vtype__> mirror(std::vector<__vtype__> __v, bool returntyped = true) {
        __functree_call__(Gmeng::mirror);
        std::reverse(__v.begin(), __v.end());
        return __v;
    };

    /// concatenates all chunks within a level, according for a 2d space
    /// of which boundaries are set by the level's header base_template.
    /// returned as a vector<string>, which consist of rendered Gmeng::Unit rows
    /// example:
    /// { "xxxxxxx",
    ///   "xxxxxxx",
    ///   "xxxoxxx", } -> VECTOR(VECTOR<Unit>, 3);
    inline vector<Unit> _vconcatenate_lvl_chunks(Level& lvl, bool cubic_render = true) {
        __functree_call__(Gmeng::_vconcatenate_lvl_chunks);
        gm_log("Gmeng::_vconcatenate_lvl_chunks *debugger, *0.0, p0,gm:0 :: breakpoint 1");
        std::vector<__CHROMATIZED_CHUNK_CONTROLLER_VIEWPOINT__> v_chunks;
        const int __level_base_width__ = v_static_cast<int>(lvl.base.lvl_template.width);
        unsigned int p = 0; unsigned int rowc = 0;
        std::string t_chunk_partial = "";
        vector<vector<Unit>> v_rows;
        std::string unit_seperator = v_str((char)0x1F); // hex code of 'UNIT SEPERATOR' (1-byte long)

        for (int __rc = 0; __rc < lvl.base.lvl_template.height; __rc++) {
            std::vector<Gmeng::Unit> current_row;
            for (int __lc = 0; __lc < lvl.base.lvl_template.width; __lc++) current_row.push_back(Gmeng::Unit{.color=PINK,.special=true,.special_clr=WHITE,.special_c_unit="?"});
            v_rows.push_back(current_row);
        }; // v8.2.0 / swapped to Units instead of rendered_units
        for (const chunk chunk : lvl.chunks) {
            /// Y location from the start of the deltaY position of the viewpoint
            int vY = chunk.vp.start.y;
            /// X location from the start of the deltaX position of the viewpoint
            int __intlc = 0;
            bool vb_start = true;
            /// v8.2.0 / cubic_render parameter is unused in this function: moved to get_lvl_view
            for (const auto unit : lvl.v_render_chunk(chunk)) {
                if (__intlc-1 == _vcreate_vp2d_deltax(chunk.vp)) { vY++; __intlc = 0; vb_start = false; };
                /// inserts to the ROW of units (y = vY calculation) to the:
                ///     beggining + chunk coverage boundary start X location + current drawpoint X location
                /// \x0F is formatter to split units to prepare for trimming at get_lvl_view
                v_rows[vY].insert(v_rows[vY].begin() + chunk.vp.start.x + __intlc, unit);
                if (global.debugger) {
                    if (!vb_start && __intlc == 0) std::cout << std::endl;
                    std::cout << (lvl.display.camera.draw_unit(unit)) + colors[RED];
                };
                __intlc++;
            };
            if (global.debugger) std::cout << std::endl;
        };


        unsigned int rc = 0;
        for (vector<Unit>& row : v_rows) {
            v_chunks.push_back({
                row,
                Renderer::viewpoint { { 0, 0 }, { __level_base_width__, 0 } }
            });
        };

        unsigned int p2 = 0;
        std::vector<Unit> reinterpereted_data;
        for (const auto partial : v_chunks) {
            if (p2 != 0 && p2 % __level_base_width__ == 0) reinterpereted_data.push_back(Unit{.is_entity=true});
            if (_vcreate_vp2d_deltax(partial.vp) == lvl.base.lvl_template.width)
                for (int ik = 0; ik < partial.data.size(); ik++)
                    lvl.display.camera.display_map.unitmap[p2] = partial.data[ik],
                    reinterpereted_data.push_back(partial.data[ik]);
            else { reinterpereted_data.push_back(repeatThing<Unit>((Unit{
                    .color = color_t::PINK, .collidable = false, .special = true,
                    .special_clr = color_t::RED, .special_c_unit = "X"
                 }), _vcreate_vp2d_deltax(lvl.display.viewpoint))[0]); gm_slog(YELLOW, "DEBUGGER", colors[RED] + "p2 CHUNK WRITE ERROR ( UNMATCH p2 [" + v_str(_vcreate_vp2d_deltax(partial.vp)) + "] WITH lvl_base_with [" + v_str(lvl.base.lvl_template.width) + "] )" + colors[WHITE] + " at p2_of ~(" + v_str(p2) + ") [ partial_of: chunk_" + _uconv_1ihx(_uget_addr(partial)) + "_00" + v_str(p2 / _vcreate_vp2d_deltax(lvl.display.viewpoint)) + " ]"); };
            p2 += _vcreate_vp2d_deltax(partial.vp);
        };

        if (global.debugger) {
            gm_slog(YELLOW, "DEBUGGER", "^^ above is reinterpereted_data from _vconcatenate_lvl_chunks");
        };

        ASSERT("pref.log", p_no);
        return reinterpereted_data;
    };

    /// returns all drawpoints that are included in a viewpoint as a vector<Gmeng::Renderer::drawpoint>.
    inline vector<Renderer::drawpoint> _vexpand_viewpoint(Renderer::viewpoint &vp) {
        __functree_call__(Gmeng::_vexpand_viewpoint);
        std::vector<Renderer::drawpoint> v_drawpoints;
        for (int unchromatized_y = 0; unchromatized_y < _vcreate_vp2d_deltay(vp); unchromatized_y++) {
            for (int unchromatized_x = 0; unchromatized_x < _vcreate_vp2d_deltax(vp); unchromatized_x++) {
                v_drawpoints.push_back({ .x = unchromatized_x + vp.start.x, .y = unchromatized_y + vp.start.y });
            };
        };
        return v_drawpoints;
    };

    /// Returns the renderscale of a level.
    ///
    /// @before {
    ///     This method concatenates all chunks in the level according to their scalar viewpoints
    ///     to prevent cumilative-index drawing which may result in a wrong image.
    ///     PERFORMANCE-HEAVY especially in bigger levels.
    ///     DO NOT! use this method in loops. It will cause lag and stutterring. This is not due to a
    ///     poorly optimized method. It is intended to be called after loading a gm4_0 framework glvl.
    ///     It will trace_chunk_vector(&lvl) and render it completely. It should be used as a base image.
    ///     Only use this when a chunk's models, textures are updated. Player and entity movement are handled
    ///     automatically, so there is no need to put this method in any loops.
    /// }
    ///
    /// @after {
    ///     Since 10.0.0, this method is sufficiently optimized. Draw times will not be affected,
    ///     and it is safe to use inside event loops. to draw the screen.
    /// }
    inline vector<Unit> _vget_renderscale2dpartial_scalar(Level& level_t, bool cubic_render = true) {
        __functree_call__(Gmeng::_vget_renderscale2dpartial_scalar);
        vector<Unit> v_concat_chunks = _vconcatenate_lvl_chunks(level_t, cubic_render);
        return v_concat_chunks;
    };

    /// Returns the renderscale of a level.
    ///
    /// @before {
    ///     This method concatenates all chunks in the level according to their scalar viewpoints
    ///     to prevent cumilative-index drawing which may result in a wrong image.
    ///     PERFORMANCE-HEAVY especially in bigger levels.
    ///     DO NOT! use this method in loops. It will cause lag and stutterring. This is not due to a
    ///     poorly optimized method. It is intended to be called after loading a gm4_0 framework glvl.
    ///     It will trace_chunk_vector(&lvl) and render it completely. It should be used as a base image.
    ///     Only use this when a chunk's models, textures are updated. Player and entity movement are handled
    ///     automatically, so there is no need to put this method in any loops.
    /// }
    ///
    /// @after {
    ///     Since 10.0.0, this method is sufficiently optimized. Draw times will not be affected,
    ///     and it is safe to use inside event loops. to draw the screen.
    /// }
    template <typename... Args>
    auto get_renderscale(Args&&... args) -> decltype(_vget_renderscale2dpartial_scalar(std::forward<Args>(args)...)) {
        return _vget_renderscale2dpartial_scalar(std::forward<Args>(args)...);
    };

    /// Draws a line of units
    /// vector<Unit> -> "[][][][]"
    inline std::string draw_line_units(vector<Unit> line) {
        Camera<1,1> camera;
        /// ensure resolution - fix v8.2.0-d
        camera.SetResolution(1,1);
        std::string __final__ = "";
        for (const auto unit : line )
            __final__ += camera.draw_unit(unit);
        return __final__;
    };

    /// returns the camera of the current level, with drawpoints included in level_t->display.viewpoint
    /// This utility is a replacement for cam_view_generate, chunk_view and spliced_chunk systems.
    ///
    /// @since 7.2.0 @continuous 8.1.0 @since {cubic_render} 8.2.0 @since {debug_render} 10.4.0
    ///
    /// use `get_renderscale` to receive a concatenated vector<Unit> object to be parsed by this method.
    /// Give the output of this method to `emplace_lvl_camera` and it will set up your camera.
    ///
    /// This utility supports chunk splicing, cubic rendering (with 1x1 square, smaller units
    /// instead of 2x1 rectangle units) for much more size and way better visuals, debug rendering,
    /// continuous and seemless chunk movement, viewpoints.
    ///
    /// @coming_soon { entities & players - for now we use models for both. }
    inline std::string get_lvl_view(Level& level_t, vector<Unit> concat_chunks, bool cubic_render = true) {
        __functree_call__(Gmeng::get_lvl_view);
        gm_log("get_lvl_view -> tracing viewpoint from level->display.vp");
        /// splits each unit including its colorcode ascii characters, using the formatter \x0F defined
        /// in _vget_renderscale2dpartial_scalar() ~ _vconcatenate_lvl_chunks().
        vector<vector<Unit>> trimmed_units = splitThing<Unit>(concat_chunks, [&](Unit u) -> bool {
            return u.is_entity == true; // (x == true) operator is required because u.is_entity can be NULL
        }); /// TRIMMED_UNITS == line of units. trimmed_units[0] = vector<Unit>(LINE_0)

        for ( int entity_id = 0; entity_id < level_t.entities.size(); entity_id++) {
            auto entity = level_t.entities.at(entity_id).get();

            for ( int loops = 0; loops < entity->sprite.width*entity->sprite.height; loops++ ) {
                bool debug = level_t.display.camera.modifiers.get_value("debug_render") >= 1;
                if ( entity->sprite.units.at(loops).transparent && !debug ) continue;
                int _y = loops / entity->sprite.width;
                int y = entity->position.y + _y;
                int _x = (loops % entity->sprite.width);
                int x = entity->position.x + _x;

                Unit unit = entity->sprite.units.at(loops);

                if ( debug ) {
                    if ( _y == 0 || _y == entity->sprite.height-1 ||
                         _x == 0 || _x == entity->sprite.width-1     ) unit = { YELLOW };
                    if ( entity->sprite.units.at(loops).transparent && unit.color != YELLOW ) continue;
                };

                trimmed_units.at(y).at(x) = unit;
            };

            /// entity proximity renderer, for debugger showing tracers to
            /// entities the current entity is in proximity to.
            if ( level_t.display.camera.modifiers.get_value("debug_render") == 3 ) {
                for ( int entity_id2 = 0; entity_id2 < level_t.entities.size(); entity_id2++ ) {
                    if (entity_id == entity_id2) continue; // skip checking between the same entity twice
                    auto entity2 = level_t.entities.at(entity_id2).get();

                        auto prox = calculate_proximity(
                            entity->position,  { (int)entity->sprite.width,   (int)entity->sprite.height },
                            entity2->position, { (int)entity2->sprite.width,  (int)entity2->sprite.height });
                    if ( prox.proximity < entity->interaction_proximity && prox.proximity < entity2->interaction_proximity ) {
                        auto path = shortest_path( prox.path );
                        for ( auto dp : path ) {
                            trimmed_units.at(dp.y).at(dp.x) = { .color = PINK };
                        };
                    };
                };
            };
        };

        /// debugger-special logs
        /// trimmed_unit inspection
        if (global.debugger) {
            gm_slog(GREEN, "DEBUGGER_RENDERENGINE", "inspection of trimmed_units:");
            /*unsigned int cc = 0;
            for ( const auto unit : trimmed_units ) {
                if (cc % v_static_cast<int>(level_t.base.lvl_template.width) == 0) std::cout << std::endl;
                std::cout << level_t.display.camera.draw_unit(unit);
                cc++;
            };*/ // v8.2.0 -> swap to vector<Unit> disables this functionality for the time being
            gm_slog(GREEN, "DEBUGGER_RENDERENGINE", colors[RED] + "< currently disabled >");
        };
        /// TODO: create arbitrary return type for get_lvl_view
        /// returning this response as a string is inefficient
        std::string __final__ = "";
        unsigned int ptr = 0;
        gm_log("get_lvl_view -> expanding viewpoint of level_t's camera");
        /// expand the viewpoint to a vector of drawpoints
        auto time_start = GET_TIME();
        auto resource = _vexpand_viewpoint(level_t.display.viewpoint);
        // for cubic_render, skips the Y position for already-drawn lines
        int skip_y_position = -1;
        bool notified = false;
        /// loop through each drawpoint in the camera.
        Unit no_un = {
                        .color = BLACK,
                        .special = true, .special_clr = RED,
                        .special_c_unit = "?"
                     };
        for (const auto dp : resource) {
            if (dp.y == skip_y_position) {
                if (!notified && global.dont_hold_back) gm_log( " { cubic_render } advancing to next Y position / seek_drawn( ++dp.Y )");
                ptr++; // still count the unit number
                continue;
            }; notified = false;
            if (global.dont_hold_back) gm_log(" :::: get_lvl_view -> CURR_DP: " + Renderer::conv_dp(dp) + " *(p): " + v_str(ptr));
            if (global.debugger) {
                gm_slog(YELLOW, "DEBUGGER", "current drawpoint: " + v_str(dp.x) + "," + v_str(dp.y));
                gm_slog(YELLOW, "DEBUGGER", "curr_dp potential vexers:");
                gm_slog(YELLOW, "DEBUGGER", "ptr == " + v_str(ptr) + " | deltaX ~ ^vp2d == " + v_str(_vcreate_vp2d_deltax(level_t.display.viewpoint)));
                gm_slog(YELLOW, "DEBUGGER", "unit_depot size()" + v_str(trimmed_units.size()) + "concat_chunks size() == " + v_str(concat_chunks.size()) + " | _vcreate_vu2d_delta_xy ~ dp.x, dp.y, ^level_t.display.viewpoint == " + v_str(_vcreate_vu2d_delta_xy(dp.x, dp.y, _vcreate_vp2d_deltax(level_t.display.viewpoint))));
            };
            if (ptr % _vcreate_vp2d_deltax(level_t.display.viewpoint) == 0 && ptr != 0) __final__ += "\n";
            /// INVALID, retrieves from vp not level_t.display
            int _vpos_fallback_avoid_01 = ( ((int)(ptr / _vcreate_vp2d_deltax(level_t.display.viewpoint)) * _vcreate_vp2d_deltax(level_t.display.viewpoint)) + ( ptr % _vcreate_vp2d_deltax(level_t.display.viewpoint) ) );
            /// valid _vpos
            int _vpos = ( dp.y * v_static_cast<int>(level_t.base.lvl_template.width) + dp.x );
            /// final unit to be placed to the _vpos specified 1d vector id for the 2d unit dp pointer, deltaX
            /// \x0F is a formatter to trim in emplace_lvl_camera
            ///------------------------------------------------------------------------------------------------
            /// fallback in case the viewpoint is out of bounds or extends out of the base level wrapper,
            /// otherwase the application would crash due to heap-extreme-overflow-access-unallocated-mem SEGV.
            #define out_of_bounds(dp)  (dp.x < 0 || dp.y < 0 || dp.x >= level_t.base.lvl_template.width || dp.y >= level_t.base.lvl_template.height)
            #define out_of_cbounds(dp) (dp.x < 0 || dp.y < 0 || dp.x >= level_t.base.lvl_template.width || dp.y >= (level_t.base.lvl_template.height/2))
            if ( out_of_bounds(dp) ) {
                if (global.debugger) gm_slog(RED, "DEBUGGER", colors[YELLOW] + "E_NO_UNIT" + colors[WHITE] + ": drawpoint out-of-bound, replace with placeholder");
                int the_depths = (ptr % 2 == 0) == 0 ? ((dp.y % 2 == 0) ? BLACK : WHITE) : ((dp.y % 2 == 0) ? WHITE : BLACK);
                __final__ += level_t.display.camera.draw_unit(no_un) + "\x0F"; ptr++; // v7.1.0 - filler unit for out-of-bounds drawpoint, to prevent crashes
                continue;
            } else {
                if (global.debugger) gm_slog(RED, "DEBUGGER", "dp.x: " + v_str(dp.x) + " lvl.width: " + v_str(level_t.base.lvl_template.width) + " dp.y: " + v_str(dp.y) + " lvl.height: " + v_str(level_t.base.lvl_template.height));
                if (dp.y == trimmed_units.size()) { gm_log("!! ** ooga-booga problem: trimmed_size(" + v_str(trimmed_units.size()) + ") = dp.y(" + v_str(dp.y) + ") "); continue; };
                Unit __unit;
                /// v8.2.0 / vpos scales with each unit, % operator will chromatize to the current X position
                int true_position = _vpos % level_t.base.lvl_template.width;
                try { __unit = trimmed_units.at(dp.y).at(true_position); } catch(const std::out_of_range& e) {
                    gm_log( "FAULTY_POS: _vpos -> " + v_str(true_position) + " | dp.y -> " + v_str(dp.y) + " | units_size -> " + v_str(trimmed_units.size()));
#if __GMENG_LOG_TO_COUT__
                    gm_log( "NONFATAL_STOP: " + v_str(_vpos) + "," + v_str(true_position));
#endif
                    continue;
                };
                /// v8.2.0 / cubic_render -> real pixel-scale 1x1 unit rendering
                if (cubic_render) {
                    /// fallback if fetching the lower-unit fails
                    /// should not happen though, bounds check is performed before this operation
                    Unit __fallback__ = Unit {.color=PINK,.special=true,.special_clr=WHITE,.special_c_unit="?"};
                    Unit __next_unit = __fallback__;
                    #define _draw level_t.display.camera.draw_unit
                    try { __next_unit = trimmed_units.at(dp.y+1).at(true_position); } catch (const std::out_of_range& e) {
                        __final__ += _draw(__fallback__) + "\x0F";
                        continue;
                    };
                    /// double-render in one character size
                    /*if (__unit.color == __next_unit.color) {
                        /// same color, does not require different pixels
                    };*/ // v8.1.1 / draw_unit() automatically handles same colors
                    __final__ += _draw(__unit, __next_unit);
                } else {
                    /// non-cubic render
                    __final__ += level_t.display.camera.draw_unit(__unit);
                };
                __final__ += "\x0F"; // v8.2.1-d / delimeter fix lol
            };
            ptr++;
            if (cubic_render) skip_y_position = dp.y+1; // v8.2.1-d / skip repeating already-drawn line
        };

        auto time_fin = GET_TIME() - time_start;
        level_t.display.camera.frame_time = time_fin;
        gm_log("get_lvl_view -> level view rendered");
        if (global.debugger) {
              gm_slog(GREEN, "RENDERER OUTPUT FOR get_lvl_view:", "deltaX: " + v_str(_vcreate_vp2d_deltax(level_t.display.viewpoint)) +"\n" + __final__);
        };

        /// remove \n newlines and return base, with formatters to split units
        ASSERT("pref.log", DISABLE());
        return g_joinStr(g_splitStr(__final__, "\n"), "");
    };

    /// emplaces unit data to the camera.
    /// This method requires a Gmeng::Level& and a viable rendered_viewpoint as parameters.
    /// to create a rendered_viewpoint use Gmeng::_vget_renderscale2dpartial_scalar(level) and put its return value to get_lvl_view(level, value)
    inline void emplace_lvl_camera(Level& level_t, std::string cam_data) {
        __functree_call__(Gmeng::emplace_lvl_camera);
        std::string cam_data_raw = cam_data;
        gm_log("emplace_lvl_camera -> retrieved viable camera data from rendered_viewpoint");
        int p = 0;
        auto deltas_x = _vcreate_vp2d_deltax(level_t.display.viewpoint);
        auto deltas_y = _vcreate_vp2d_deltay(level_t.display.viewpoint);
        if (level_t.display.width  != deltas_x
        ||  level_t.display.height != deltas_y) level_t.display.set_resolution(deltas_x, deltas_y);
        for (const auto raw_unit : g_splitStr(cam_data_raw, "\x0F")) {
            if (Gmeng::global.debugger) gm_log("emplace_lvl_camera -> overriding renderunit @ " + v_str(p) + " with cam_viewpoint[" + v_str(p) +"]");
            level_t.display.camera.raw_unit_map[p] = raw_unit + Gmeng::resetcolor;
            p++;
        };
        gm_log("buffer override complete, the rendered_viewpoint is transfered to the camera");
        ASSERT("pref.log", DISABLE());
    };


    /// Text and ascii character handling
    /// TODO: implement textarea to fw4.0_glvl framework's world file format

    // textblob_colors, color impl for text and blob
struct textblob_colors_t {
    Gmeng::color_t text;
    Gmeng::color_t blob;
};
    // textblob, blob of text, controller methods in struct
    // still no source
struct textblob_t {
    private:
        inline bool checkspace(std::size_t width, std::size_t height) {
            if (this->position.x < width              ||
                this->position.y < height             ||
                this->position.x+1 >= width           ||
                this->position.y+1 >= height          ||
                this->position.x+4 + contents.length() >= width ||
                this->position.y+2+(contents.length() / this->max_line_width) >= height ||
                this->position.x+4 + title.length() >= height)
                return false;
            return true;
        };

        Gmeng::Camera<0,0> cam_hide;
        bool placed = false;
        bool hidestate = false;
    public:
        std::size_t max_line_width; Renderer::drawpoint position;
        std::string contents; std::string title; textblob_colors_t colors;
        std::map<int, Gmeng::Unit> affected_units; std::vector<int> affected_unit_ids;

        // Constructor
        textblob_t(const std::string& text, const std::string& title, std::size_t max_line_width, Renderer::drawpoint pos, color_t text_color, color_t blob_color)
            : contents(text), title(title), colors({text_color, blob_color}), max_line_width(max_line_width), position(pos) {}

        // Emplace function
        inline void emplace(Gmeng::Camera<0,0>& cam) {
            this->cam_hide = cam;
            this->placed = true;
            if (!this->checkspace(cam.w, cam.h)) {
                if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "textblob_t could not be placed to " + v_str(this->position.x) + "," + v_str(this->position.y) + " because there is not enough space.");
                return;
            };
        };

        // Show function
        template <typename T, typename = std::enable_if_t<std::is_same_v<T, int> || std::is_same_v<T, bool>>>
        inline void hide(T state) {
            if (state) {
                this->emplace(this->cam_hide);
            } else {
                this->remove(this->cam_hide);
            };
            this->hidestate = (bool) state;
        };

        // Remove function
        inline void remove(Gmeng::Camera<0, 0>& cam) {
            this->cam_hide = cam;
            if (!this->placed || this->hidestate) { if (global.debugger) gm_slog(YELLOW, "DEBUGGER", "ignored textblob_t.remove() call  because the text is already hidden or is not placed."); return; };
            /// TODO: implement
        };
};

    // textarea, creates a blob to emplace text
    // turns units surrounding text in the specified position to special,
    // and surrounds it with outer_c_unit characters
inline textblob_t textarea(Gmeng::Camera<0, 0> cam, Renderer::drawpoint pos, std::string text, std::string title = "$__NO_TITLE", std::size_t max_line_width = 10, color_t text_color = WHITE, color_t blob_color = BLACK) {
    textblob_t blob(text, title, max_line_width, pos, text_color, blob_color);
    blob.emplace(cam);
    return blob;
};
};




#define __GMENG_MODELRENDERER__INIT__ true
