#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <map>
#include <random>
#include <filesystem>
#include <sstream>

#include "../utils/textures.cpp"

#ifdef __GMENG_INIT__
#ifndef __GMENG_MODELRENDERER__INIT__

namespace fs = std::filesystem;

namespace Gmeng {
    namespace Renderer {
        struct drawpoint { int x; int y; };
        struct viewpoint { drawpoint start; drawpoint end; };
        class Model {
            private:
              inline Objects::coord get_pointXY(int pos) {
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
              inline void attach_texture(Gmeng::texture __t) { this->texture = __t; this->width = __t.width; this->height = __t.height; };
              inline void load_texture(std::string __tf) { this->texture = Gmeng::LoadTexture(__tf); }; //! FIXME: width,height values remain unchanged
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
            return returnvalue;
        };
        // compiles Renderer::Model objects into a Gmeng::Unit vector
        std::vector<Gmeng::Unit> draw_model(Gmeng::Renderer::Model __m) {
            std::vector<Gmeng::Unit> unitmap;
            gm_log("job_render *draw_model -> MODEL * size: " + v_str(__m.width * __m.height) + " units | TEXTURE * size: " + v_str(__m.texture.units.size()) + " units");
                for ( int i = 0; i < __m.height; i++ ) {
                    for ( int j = 0; j < __m.width; j++ ) {
                        int vpos = (i*__m.width)+j; unitmap.push_back(__m.texture.units[vpos]);
                        gm_log("job_render *draw_model (_vlinear_draw=true) -> vp_compileUnit (from<texture>->units to<unitmap>->vector) : unit * vpos = " + v_str(vpos+1) + " / " + v_str(__m.texture.units.size()));
                    };
                };

            gm_log("job_render (draw_model -> vp_compileUnit v_success) v_static_cast<std::vector<units>> total_size : " + v_str(unitmap.size()));
            return unitmap;
        };
        /// returns size of a drawpoint (NOT FOR X,Y POSITIONS)
        /// for coordinate sizes, use trace_1dp(msize, mapwidth)
        std::size_t getsize(drawpoint c) {
            return (c.x * c.y);
        };
        /// returns drawpoint object as log string
        std::string conv_dp(drawpoint p) {
            return "x=" + v_str(p.x) + ",y=" + v_str(p.y);
        };
        // returns placement coordinates for each coordinate of object at __p with size of __s in mapsize of __ws
        std::vector<Gmeng::Renderer::drawpoint> get_placement(drawpoint __p, drawpoint __s, drawpoint __ws, bool _vlinear_render = false) {
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
            return vec;
        };
        /// returns placement coordinates for a viewpoint within a map sizeof drawpointxy
        std::vector<drawpoint> get_displacement(viewpoint& view, const drawpoint& map) {
            std::vector<drawpoint> result;

            // Ensure the viewpoint is within the map boundaries
            view.start.x = std::max(view.start.x, 0); view.end.x = std::min(view.end.x, map.x - 1);
            view.start.y = std::max(view.start.y, 0); view.end.y = std::min(view.end.y, map.y - 1);
            for (int x = view.start.x; x <= view.end.x; ++x) {
                for (int y = view.start.y; y <= view.end.y; ++y) { result.push_back({x, y}); };
            };
            return result;
        };

        Gmeng::Renderer::viewpoint c_npos = viewpoint {
            .start = drawpoint { .x=-1, .y=-1 },
            .end   = drawpoint { .x=-1, .y=-1 },
        };
        class Display {
            private:
            public:
                Gmeng::CameraView<0, 0> camera; Gmeng::Renderer::viewpoint viewpoint;
                std::size_t width; std::size_t height; std::vector<Gmeng::Unit> rendered_units;
                inline void set_resolution(std::size_t width, std::size_t height) {
                    this->width = width; this->height = height;
                    this->camera.SetResolution(this->width, this->height);
                };
                inline void refresh() {
                    // refresh resolution
                    this->camera.SetResolution(this->width, this->height);
                };
                inline void plunit(Objects::G_Player pl, Objects::coord plcoords) {
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
                    this->rendered_units[coords.y*this->camera.w + coords.x].is_player = false;
                };
                inline void draw(Objects::G_Player pl, Objects::coord plcoords) {
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
    // and compiled into a Gmeng::CameraView::unitmap;
    struct r_chunk {
        Gmeng::Renderer::viewpoint vp;
        std::vector<Gmeng::Renderer::Model> models;
    };
    // levelinfo (parsed into Gmeng::Level::load_level after Gmeng::parse_glvl())
    struct LevelInfo {
        Gmeng::Renderer::LevelBase base; std::string name; std::vector<int> display_res;
        std::vector<Gmeng::r_chunk> chunks; std::string description;
    };
    inline Objects::coord trace_1dp(int xy, int wmp_x) {
        int width  = xy % wmp_x; int height = xy / wmp_x;
        return Objects::coord { .x = width, .y = height };
    };
    /// logs information about units within vp_units as vector, po_size vp_units wsizeX
    inline void log_vpu(std::vector<Gmeng::Unit>& vp_units, int wsizeX) {
        int jndx = 0;
        for (const auto& _v_unit : vp_units) { Objects::coord gp = Gmeng::trace_1dp(jndx, wsizeX); gm_nlog("vp_gtx.units @ pos "+ v_str(jndx) + " ("+v_str(gp.y)+","+v_str(gp.x)+"):\n\tv_color -> " + v_str(_v_unit.color) + "\n\tv_collision -> " + (_v_unit.collidable ? "true" : "false") + "\n\tv_special -> " + (_v_unit.special ? "true" : "false") + "\n\tv_spcolor -> " + v_str(_v_unit.special_clr) + "\n"); jndx++; };
        gm_nlog("\n");
    };
    /// logs information about a viewpoint chunk
    inline void log_vpc(Gmeng::r_chunk vp_chunk) {
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
            Gmeng::CameraView<10, 10>* preview_cam = new Gmeng::CameraView<10, 10>();
            std::vector<Gmeng::Unit> v_preview = Gmeng::Renderer::draw_model(v_mdl);
            for (int v_lndx = 0; v_lndx < v_preview.size(); v_lndx++) {
                if (v_lndx % v_mdl.width == 0) gm_nlog("\n\t\t");
                gm_nlog(preview_cam->draw_unit(v_preview[v_lndx]));
            };
            gm_nlog("\n");
        };
    };
        inline const Gmeng::Renderer::Model nomdl = {
        .id=Gmeng::CONSTANTS::vl_nomdl_id
    };
    inline const Gmeng::texture notxtr = {
        .name=v_str(Gmeng::CONSTANTS::vl_notxtr_id)
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
    inline Gmeng::Renderer::Model vd_find_model(v_dictl<Gmeng::Renderer::Model> dict, v_title name) {
        for (const auto& val : dict.v_getrelative()) {
            if (val.data.name == name) return val.data;
        };
        return Gmeng::nomdl;
    };
    inline Gmeng::texture vd_find_texture(v_dictl<Gmeng::texture> dict, v_title name) {
        int ccount = 0;
        for (const auto& val : dict.v_getrelative()) {
            gm_log("gm::vd_find_texture() -> v_searchAddr: v_dictl<gm::texture> vgm_defaults asumed | addr: " + _uconv_1ihx(ccount) + " -> relative_value = " + val.data.name + " , search_value = " + name + " , match = " + std::string(val.data.name == name ? "yes" : "no"));
            if (val.data.name == name) return val.data;
            ccount++;
        };
        return Gmeng::notxtr;
    };
    namespace vgm_defaults {
        v_dictl<Gmeng::Renderer::Model> vg_rdmodels;
        v_dictl<Gmeng::texture>         vg_textures;
    };
    /// reads a folder's .gmdl and .gtx files into Gmeng::vgm_defaults::vg_rdmodels & vg_textures;
    inline void _uread_into_vgm(const v_title& folder) {
        // we wait until all textures are in vgm_defaults before we load any models
        // afterwards we loop through the vector and load them into Gmeng::vgm_defaults::vg_rdmodels
        std::vector<std::string> q_modelindx;
        gm_log("_uread_into_vgm() : job_readfile *loop -> ffolder:name = " + folder);
        try {
            for (const auto& entry : fs::directory_iterator(folder)) {
                gm_log("_uread_into_vgm() : reading file " + entry.path().string() + " - size: " + v_str(entry.file_size()) + "B | " + v_str(entry.file_size() / 1024) + "KB | " + v_str(entry.file_size() / 1024 / 1000) + " MB");
                if (entry.is_regular_file()) {
                    v_title fname = (entry.path().string());
                    gm_log("_uread_into_vgm() : status_update -> entryAt = " + folder = " | filename: " + entry.path().filename().string());
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
        for (const auto& mdl : vgm_defaults::vg_rdmodels.v_getrelative()) { v_mdls[mdl.data.name] = mdl.data; };
        for (const auto& txtr : vgm_defaults::vg_textures.v_getrelative()) { v_txtrs[txtr.data.name] = txtr.data; };
    };
    inline LevelInfo parse_glvl(std::string __fn) {
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
                    Gmeng::texture _m_texture = textures[params[6].substr(3)];
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
                    info.chunks.push_back(Gmeng::r_chunk {
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
    class Level {
        private:
            // compiles a chunk into a std::vector<Gmeng::Unit> unitmap for a CameraView instance to render
            inline std::vector<Gmeng::Unit> render_chunk(Gmeng::r_chunk chunk) {
                // write base_template skybox image to chunk (level 0 of canvas)
                // base_template's viewpoint relative to the chunks viewpoint will be drawn as a base 'skybox' like image
                Gmeng::texture* base_map = &this->base.lvl_template; std::vector<Gmeng::Unit> units;
                gm_log("init OK" );
                gm_log("base_map -> WIDTH: " + v_str(base_map->width) + " - HEIGHT: " + v_str(base_map->height) );
                std::vector<Gmeng::Renderer::drawpoint> wrapper = Gmeng::Renderer::get_displacement(chunk.vp, Renderer::drawpoint {.x=static_cast<int>(base_map->width),.y=static_cast<int>(base_map->height)});
                gm_log("get_displacement OK" );
                gm_log("chunk viewpoint -> start -> x: " + v_str(chunk.vp.start.x) + " - y: " + v_str(chunk.vp.start.y) + " | end -> x: " + v_str(chunk.vp.end.x) + " - y: " + v_str(chunk.vp.end.y) );
                gm_log("wrapper viewpoints -> " + v_str(wrapper.size()) );
                gm_log("get_displacement -> " + v_str(wrapper.size()) + " drawpoints" );
                for ( const auto& dp : wrapper ) {
                    int vpos = (dp.y * base_map->width)+dp.x;
                    Gmeng::Unit v_unit = base_map->units[vpos];
                    units.push_back(v_unit);
                    gm_log("job_render *render_chunk -> v_chunkwrapper vpos at("+v_str(vpos)+") out of vp_chunk::constl::base_template");
                };
                gm_nlog("job_render *render_chunk -> v_chunkwrapper compileUnits: v_success\ncl_preview:");
                int v_compl_t = 0;
                gm_nlog("\n\t");
                for ( const auto& un : units ) {
                    gm_nlog(this->display.camera.draw_unit(un) + (v_compl_t != 0 && v_compl_t % base_map->height == 0 ? "\n\t" : ""));
                    v_compl_t++;
                };
                gm_log("write_drawpoint -> base_map.units OK" );
                // compile models into std::vector<Gmeng::Unit> unitmaps and write them to x,y coordinates
                // within the r_chunk by getting placement positions with Gmeng::Renderer::get_placement
                for ( const auto& model : chunk.models ) {
                    Gmeng::Renderer::drawpoint dp = model.position;
                    gm_log("dp_loadmodel " + v_str(model.id) + " OK" );
                    gm_log("dp -> x: " + v_str(dp.x) + " - y: " + v_str(dp.y) );
                    gm_log("p2 -> x: " + v_str(model.width) + " - y: " + v_str(model.height) );
                    std::vector<Gmeng::Renderer::drawpoint> displacement = Gmeng::Renderer::get_placement(dp, {.x=static_cast<int>(model.width),.y=static_cast<int>(model.height)}, {.x=static_cast<int>(this->display.width),.y=static_cast<int>(this->display.height)});
                    for ( auto dp : displacement ) {
                        gm_log("displacement_log = x: " + v_str(dp.x) + " - y: " + v_str(dp.y) );
                    };
                    gm_log("total_drawpoints: " + v_str(displacement.size()) );
                    gm_log("get_placement OK" );
                    std::vector<Gmeng::Unit> unitmap = Gmeng::Renderer::draw_model(model);
                    gm_log("draw_model OK: unitmap.size(): " + v_str(unitmap.size()) );
                    int lndx = 0;
                    int i2 = 0;
                    for ( const auto& unit : unitmap ) {
                        if (unit.transparent) { lndx++; continue; };
                        gm_log(v_str(lndx) +" <- pos_vdp: rendering_model_unit PREVIEW: " + this->display.camera.draw_unit(unit) );
                        int _vdp_pos = (displacement[lndx].y*this->display.width)+displacement[lndx].x;
                        gm_log("_vdp_pos find: " + v_str(_vdp_pos) +" OK" );
                        gm_log("_vdp_current -> x: " + v_str(displacement[_vdp_pos].x) + " - y: " + v_str(displacement[_vdp_pos].y) );
                        gm_log("_vdp_current_addr -> " + this->display.camera.draw_unit(units[_vdp_pos]) );
                        gm_log("swap_unit: at(" + v_str(_vdp_pos) +") -> PREVIEW: " + this->display.camera.draw_unit(units[_vdp_pos]) + " TO unit() -> PREVIEW: " + this->display.camera.draw_unit(unit) );
                        units[_vdp_pos] = unitmap[lndx];
                        gm_log("set_unit_at(id: " + v_str(_vdp_pos) + ") OK" );
                        lndx++;
                    };
                };
                gm_log("job_render *render_chunk completed, status: v_success");
                gm_log("logs:");
                gm_log("v_units -> size() : " + v_str(units.size()));
                return units;
            };
            inline Gmeng::r_chunk get_chunk(int id) {
                return this->chunks[id];
            };
            inline void set_chunk(int id, Gmeng::r_chunk chunk) {
                this->chunks[id] = chunk;
            };
        public:
            Gmeng::Renderer::LevelBase base; Gmeng::Renderer::Display display; Objects::G_Player player = Gmeng::v_base_player; Objects::coord plcoords = { .x=0, .y=0 };
            std::vector<Gmeng::r_chunk> chunks; std::string desc; std::string name;
            inline int load_chunk(Gmeng::r_chunk chunk) {
                this->chunks.push_back(chunk);
                return (this->chunks.size()-1);
            };
            inline void load_level(Gmeng::LevelInfo __glvl) {
                int i,j = 0;
                gm_log("vp_loadlevel: __glvl chunks v_size: " + v_str(__glvl.chunks.size()));
                for (const auto& chunk : __glvl.chunks) {
                    Gmeng::log_vpc(chunk);
                    if (i >= this->chunks.size()) { this->load_chunk(chunk); continue; };
                    this->chunks[i] = chunk;
                    i++;
                };
                this->base.height = __glvl.base.height;
                this->base.width  = __glvl.base.width;
                this->display.set_resolution(__glvl.display_res[0], __glvl.display_res[1]);
                this->base.lvl_template.collidable = __glvl.base.lvl_template.collidable;
                this->base.lvl_template.height = __glvl.base.height;
                this->base.lvl_template.width  = __glvl.base.width;
                gm_log("class Model { ... } : load_level(...) -> base_template @ h+w = " + v_str(__glvl.base.height) + "," + v_str(__glvl.base.width));
                this->base.lvl_template.name   = __glvl.base.lvl_template.name;
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
            // draws chunk in Gmeng::Level::(std::vector<Gmeng::r_chunk>)chunks on position: chunk_id
            // to the Display::CameraView controller
            inline void draw_camera(int chunk_id) {
                if (chunk_id < 0 || chunk_id > chunks.size()) throw std::invalid_argument("chunk_id is invalid");
                Gmeng::r_chunk chunk = this->chunks[chunk_id];
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
            };
            /// refreshes the current chunk display
            inline void refresh() {
                this->display.set_resolution(this->display.width, this->display.height);
                this->display.draw(this->player, this->plcoords);
            };
            inline void set_player(Objects::G_Player p, int x, int y) {
                if (this->display.camera.player_init) this->display.nplunit(this->plcoords);
                this->player = p; this->plcoords = { .x=x, .y=y };
                gm_log("r_level::set_player *inline,static -> v_success ; r_level::player (Objects::G_Player).coords = @pos(" + v_str(this->player.coords.x)+","+v_str(this->player.coords.y)+")");
                this->move_player(x, y);
            };
            inline void move_player(int x, int y) {
                this->plcoords.x=x; this->plcoords.y=y;
            };
    };
};

#define __GMENG_MODELRENDERER__INIT__ true
#endif
#endif
