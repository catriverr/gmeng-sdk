#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <map>
#include <filesystem>
#include "../src/textures.cpp"

#include "../types/model.h"
#include "../types/entity.h"
#include "../types/display.h"
#include "../types/level.h"

#include "gmeng.cpp"

#include "../utils/viewpoint.h"

#include "objects.cpp"
#ifndef _WIN32
#include <termios.h>
#include <sys/select.h>
#endif
#include <unistd.h>
#include <functional>


namespace fs = std::filesystem;


namespace Gmeng {
    /// Ansi escape code for HOME (0, 0 cursor position)
    constexpr const char* HOME = "\033[H";

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


    /// All Renderer utilities, structs, objects
    /// controllers, handlers and more.
    namespace Renderer {
        int dp_distance(const drawpoint& a, const drawpoint& b) {
            int dx = b.x - a.x;
            int dy = b.y - a.y;
            return dx * dx + dy * dy;  // precise squared distance
        }





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
        vector<Unit> draw_model(const Model& __m) {
            return __m.texture.units;
        };

    }; /// namespace Renderer

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
        for (const auto& _v_unit : vp_units) { Objects::coord gp = Gmeng::trace_1dp(jndx, wsizeX); gm_nlog("vp_gtx.units @ pos "+ v_str(jndx) + " ("+v_str(gp.y)+","+v_str(gp.x)+"):\n\tv_color -> " + v_str((int)_v_unit.color) + "\n\tv_collision -> " + (_v_unit.collidable ? "true" : "false") + "\n\tv_special -> " + (_v_unit.special ? "true" : "false") + "\n\tv_spcolor -> " + v_str(_v_unit.special_clr) + "\n"); jndx++; };
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
    inline texture notxtr = {
        Blob( 0, 0, {} ), false, v_str(CONSTANTS::vl_notxtr_id)
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
            inline std::vector<Gmeng::vd_item<v_type>>& v_getrelative() { return this->values; };
    };

    inline Renderer::Model vd_find_model(v_dictl<Renderer::Model> dict, v_title name) {
        __functree_call__(Gmeng::vd_find_model);
        for (const auto& val : dict.v_getrelative()) {
            if (val.data.name == name) return val.data;
        };
        return Gmeng::nomdl;
    };
    inline texture& vd_find_texture(v_dictl<texture>& dict, v_title name) {
        __functree_call__(Gmeng::vd_find_texture);
        int ccount = 0;
        for (auto& val : dict.v_getrelative()) {
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
    /// reads a folder's VGM texture & model files into Gmeng::vgm_defaults::vg_rdmodels & vg_textures
    /// for ease-of-access to textures without having to read them from the disc.
    inline int _uread_into_vgm(const v_title& folder) {
        __functree_call__(Gmeng::_uread_into_vgm);
        // we wait until all textures are in vgm_defaults before we load any models
        // afterwards we loop through the vector and load them into Gmeng::vgm_defaults::vg_rdmodels
        std::vector<std::string> q_modelindx;
        gm_log("Gmeng VisualCache Graphics Manager: reading visual cache models from directory: " + folder);
        try {
            for (const auto& entry : fs::directory_iterator(folder)) {
                /// write information about the file
                /// to the log if developer mode is
                /// enabled by the user/developer
                if (Gmeng::global.dev_mode) gm_log("_uread_into_vgm() : reading file " + entry.path().string() + " - size: " + v_str(entry.file_size()) + "B | " + v_str(entry.file_size() / 1024) + "KB | " + v_str(entry.file_size() / 1024 / 1000) + " MB");
                /// check if the entry is a regular file, not a directory/else
                if (entry.is_regular_file()) {
                    /// get the name of the file, with the
                    /// path to it so it can be read without
                    /// the process working directory.
                    v_title fname = (entry.path().string());
                    /// log information about the file being
                    /// read if developer mode is enabled
                    if (Gmeng::global.dev_mode) gm_log("_uread_into_vgm() : status_update -> entryAt = " + folder = " | filename: " + entry.path().filename().string());
                    /// check if file is a model file,
                    /// skip it if it is.
                    if (endsWith(fname, ".gmdl")) {
                        /// file is a model file, read it and store
                        /// to parse after texture loading is complete.
                        std::string fcontent = g_readFile(fname);
                        /// add the entry to the model index.
                        q_modelindx.push_back(fcontent);
                        /// skip the file
                        continue;
                    };
                    /// file is assumed to be a texture file, regardless
                    /// of file extension / name unless it's a .gmdl file.
                    /// attempt to load the texture
                    auto m_item = LoadTexture( fname );

                    /// iterate through the entire dictionary to check if a value already exists
                    /// this is so we can replace it instead of using it
                    auto it = std::find_if(
                        vgm_defaults::vg_textures.v_getrelative().begin(),
                        vgm_defaults::vg_textures.v_getrelative().end(),
                        [&](const vd_item< texture >& item) {
                        /// item entry holds the same name
                        return item.data.name == m_item.name;
                    });

                    /// if the iterator has found a corresponding item,
                    /// access it to reference the value and write to it
                    /// instead of creating a new entry for the same
                    /// texture object. \since gmeng 12.0.0
                    if (it != vgm_defaults::vg_textures.v_getrelative().end()) {
                        /// dereference it to acces value
                        vd_item< texture >& item = *it;
                        /// log information about the item to the gmeng log stream.
                        gm_log(std::string("source texture (") + (fname) + ") '"
                                    + m_item.name + "' is vgm-cached, updating data only");
                        /// re-set the texture object. this is done
                        /// because the texture might be different
                        /// in the current source to the cached
                        /// texture inside the vgm dictionary.
                        item.data = m_item;
                    } else {
                        /// there was no such texture, add a new entry.
                        vgm_defaults::vg_textures.addb_l( m_item );
                    };
                };
            }
            for (const auto& q_item : q_modelindx) {
                Gmeng::Renderer::gnmdl vmdl = Renderer::parse_gmdl(q_item);
                vmdl.m.attach_texture(vd_find_texture(vgm_defaults::vg_textures, vmdl.txname));
                Gmeng::vgm_defaults::vg_rdmodels.addb_l(vmdl.m);
            };
            gm_log("Gmeng VisualCache Graphics Manager: compiled visual cache from " + folder);
        } catch (const fs::filesystem_error& ex) {
            gm_err(1, "_uread_into_vgm(const v_title& folder) : status v_error -> vgm data in folder threw " + std::string(ex.what()) + ". Data is corrupt/faulty.");
        };
        return 0;
    };

    /// writes values in Gmeng::vgm_defaults::vg_rdmodels & vg_textures into vectors v_mdls and v_txtrs
    inline void _uwritevgm_to_maps(std::map<v_title, Gmeng::Renderer::Model>& v_mdls, std::map<v_title, Gmeng::texture>& v_txtrs) {
        __functree_call__(Gmeng::_uwritevgm_to_maps);
        for (const auto& mdl : vgm_defaults::vg_rdmodels.v_getrelative()) { v_mdls[mdl.data.name] = mdl.data; };
        for (const auto& txtr : vgm_defaults::vg_textures.v_getrelative()) { v_txtrs[txtr.data.name] = txtr.data; };
    };

    [[deprecated]] inline LevelInfo parse_glvl(std::string __fn) {
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


    /// Returns the displacement vector of a circle with a radius of 'r'.
    /// center being the center position of the circle.
    ///
    /// This ignores negative values, values starting from 0,0 will be returned only.
std::vector<Gmeng::Renderer::drawpoint> get_radius_displacement(int radius, const Gmeng::Renderer::drawpoint& center) {
    std::vector<Gmeng::Renderer::drawpoint> points;
    int r2 = radius * radius;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            if (dx * dx + dy * dy > r2)
                continue;

            int x = center.x + dx;
            int y = center.y + dy;

            if (x >= 0 && y >= 0)
                points.push_back({x, y});
        }
    }

    return points;
}




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
        /// potentially dangerous but ehhh
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
        for (auto& chunk : chunks) {
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

    /// calculates the distance between 2 viewpoints
    inline int calc_distance( Renderer::drawpoint p1, Renderer::drawpoint p2 ) {
        return std::min(
                std::abs(p1.x - p2.x),
                std::abs(p1.y - p2.y)
        );
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
    /// returned as a vector<vector<Unit>>, which consist of Gmeng::Unit rows
    /// example:
    /// { "xxxxxxx",
    ///   "xxxxxxx",
    ///   "xxxoxxx", } -> VECTOR(VECTOR<Unit>, 3);
    inline vector<vector<Unit>> _vconcatenate_lvl_chunks(Level& lvl, bool _cubic_render = true, TRACEFUNC) {
        /// log to gmeng.log about the renderscale request
        DEBUGGER gm_log("Gmeng::_vconcatenate_lvl_chunks renderscale requested by " + TRACEFUNC_STR);

        /// chunk controllers, in vector.
        std::vector<__CHROMATIZED_CHUNK_CONTROLLER_VIEWPOINT__> v_chunks;
        /// the base width of the level, for easy access.
        const int __level_base_width__ = v_static_cast<int>(lvl.base.lvl_template.width);

        /// the real height of the base lvl template
        const int height_size = lvl.base.lvl_template.height;

        /// rows
        if ( !lvl.base_init ) {
            lvl.base_init = true;
            lvl.CACHED_BASE = vector(height_size, vector<Unit>(__level_base_width__));

            for ( int y = 0; y < height_size; y++ ) {
                for ( int x = 0; x < __level_base_width__; x++ ) {
                    int vdpos = y * __level_base_width__ + x;
                    lvl.CACHED_BASE[y][x] = lvl.base.lvl_template.units[vdpos];
                };
            };
        };

        vector<vector<Unit>> v_rows = lvl.CACHED_BASE;

        int vp_required_size = std::max(vp_width( lvl.display.viewpoint )*vp_height( lvl.display.viewpoint ), __level_base_width__*height_size);

        if ( lvl.display.camera.display_map.unitmap.size() < vp_required_size ) {
            lvl.display.camera.display_map.unitmap.resize( vp_required_size );
            gm_log("resized display_camera_unitmap: viewpoint changed or the unitmap is not large enough for the viewpoint");
        };


        /// real cubic render value
        bool cubic_render = _cubic_render && lvl.display.camera.modifiers.get_value("cubic_render") == 1;

        /// fill the template's width with boilerplate units
        for (chunk& chunk : lvl.chunks) {
            /// Y location since the start of the deltaY position of the viewpoint
            int vY = chunk.vp.start.y;
            /// X location since the start of the deltaX position of the viewpoint
            int __intlc = 0;
            /// for debug render of the renderscale
            bool vb_start = true;
            /// cache chunk.vp's delta X once here instead of recomputing it for every unit below
            const int __chunk_delta_x__ = _vcreate_vp2d_deltax(chunk.vp);
            /// v8.2.0 / cubic_render parameter is unused in this function: moved to get_lvl_view
            for (const auto& unit : lvl.v_render_chunk(chunk)) {
                if (__intlc-1 == __chunk_delta_x__) { vY++; __intlc = 0; vb_start = false; };
                if ( unit.transparent ) { __intlc++; continue; };
                /// inserts to the ROW of units (y = vY calculation) to the:
                ///     beggining + chunk coverage boundary start X location + current drawpoint X location
                /// \x0F is formatter to split units to prepare for trimming at get_lvl_view
                ///
                /// FIX 12.0.0:
                ///     this was previously v_rows[vY].insert( ... ) which caused
                ///     the vector to ADD the new unit, not emplace it, causing the
                ///     renderscale to be double the size. This wasn't a visible
                ///     issue since get_lvl_view uses the viewpoint which can not
                ///     be bigger than the actual viewpoint size, causing the
                ///     overflow to be hidden.
                ///
                ///     This is now x2 as efficient, with emplacing,
                ///     and also will not cause problems with SDL windows, or
                ///     any excess function that relies on the get_renderscale
                ///     function.
                ///
                int insert_pos_x = (chunk.vp.start.x + __intlc);
                 v_rows[vY][insert_pos_x] = (unit);
                __intlc++;
            };
        };

        /// initialize row-based data as renderscale chunks
        //v_chunks.reserve(v_rows.size());
        //for (vector<Unit>& row : v_rows) {
        //    v_chunks.push_back({
        //            std::move(row), // v_rows isn't read again after this loop, so move each row instead of copying it
        //        Renderer::viewpoint { { 0, 0 }, { __level_base_width__, 0 } }
        //    });
        //};

        /// reinterpret the data to match the renderscale
        //unsigned int p2 = 0;
        //std::vector<Unit> reinterpreted_data(height_size*__level_base_width__ + height_size);
        //for (const auto& partial : v_chunks) {
        //    /// cache the delta X of this partial's viewpoint once, it's reused a few times below
        //    const int __partial_delta_x__ = _vcreate_vp2d_deltax(partial.vp);
        //    /// width-splitter object, appends as an 'entity_unit'
        //    if (p2 != 0) reinterpreted_data[p2++] = (Unit{.is_entity=true});
        //    /// debug info
        //    DEBUGGER gm_log("p2: "$(p2)"partial_vp: "$(__partial_delta_x__)
        //                    ". lvl_base_width: "$(lvl.base.lvl_template.width)".");
        //    /// if the delta width of the partial's viewpoint matches the base template's width
        //    /// (it should)
        //    if (__partial_delta_x__ == __level_base_width__) {
        //        /// since we know that the width of the row is always
        //        /// going to be equal to the level's base width, no
        //        /// need to call partial.data.size() every iteration
        //        for (int ik = 0; ik < ( __level_base_width__ ); ik++)
        //            reinterpreted_data[p2++] = (partial.data[ik]);
        //    } else {
        //        /// SINCE v13.5.0: THIS CODE IS INVALID BECAUSE WE USE RESIZE()
        //        /// ON THE VECTOR. PUSH_BACK WILL BE ADDING TO AFTER THE SIZE
        //        /// WE WANT. MUST BE CHANGED IF THIS EDGE-CASE STARTS BEING
        //        /// POSSIBLE. ALTHOUGH, AS OF 13.5.0, THIS NEVER EXECUTES.
        //        /// chunk mismatch
        //        /// pushes the fallback unit directly instead of building (and immediately
        //        /// discarding) an N-unit vector via repeatThing(...) just to read back [0]
        //        reinterpreted_data.push_back(Unit{
        //            .color = color_t::PINK, .collidable = false, .special = true,
        //            .special_clr = color_t::RED, .special_c_unit = 'X'
        //         });

        //        gm_slog(RED, "ERROR", colors[WHITE] +
        //                "p2 CHUNK WRITE ERROR ( UNMATCH p2 [" + v_str(__partial_delta_x__) +
        //                "] WITH lvl_base_with [" + v_str(lvl.base.lvl_template.width) + "] )" + colors[WHITE] +
        //                " at p2_of ~(" + v_str(p2) + ") [ partial_of: chunk_" + _uconv_1ihx(_uget_addr(partial)) +
        //                "_00" + v_str(p2 / _vcreate_vp2d_deltax(lvl.display.viewpoint)) + " ]");
        //    };
        //    //p2 += __partial_delta_x__;
        //};

        /// this used to be initialized as a copy of reinterpreted_data, but it's cleared
        /// and fully rebuilt from trimmed_units further below before it's ever read, so the
        /// copy was pure wasted work; start empty instead.
        /// Disabled 13.5.0 vector<Unit> _reinterpreted_data( reinterpreted_data.size() );


        /// re-split the units for entities.
        vector<vector<Unit>> trimmed_units = std::move(v_rows);

        /// lighting modifier from the camera's modifier list
        auto lighting_enabled = lvl.display.camera.modifiers.get_value( "lighting" ) == 1;

        /// efficient_render modifier from the camera's modifier list
        int efficient_render = lvl.display.camera.modifiers.get_value("efficient_render");

        /// debug_render modifier from the camera's modifier list, hoisted here so it isn't
        /// looked up again for every pixel of every entity sprite further down
        int debug_render = lvl.display.camera.modifiers.get_value("debug_render");

        /// lighting check for the cache, skipped unless lighting is enabled
        if ( lighting_enabled && lvl.lighting_cache.empty() && lvl.light_sources.size() > 0 )
            /// check if the light_cache is empty, sets initial render to true.
            /// can be set to true to refresh the render of lighting cache.
            gm_log("recalculating lighting cache as one or more environments have changed");

        /// Reset the entity count. Inefficient... but not so much.
        lvl.display.camera.entity_count = 0;
        /// Loop through all entitites
        for ( int entity_id = 0; entity_id < lvl.entities.size(); entity_id++) {
            const auto& _entity = lvl.entities.at(entity_id); // bind by reference: avoids bumping the shared_ptr's atomic refcount per entity, per frame
            auto entity = _entity.get();

            /// entity is not valid
            if ( entity == nullptr ) continue;

            /// skip incative entities
            if ( !entity->active ) continue;

            /// debugger mode only message for comparing entity ids to LightSource::id
            DEBUGGER gm_log("entity id "$(entity->get_serialization_id()) " vs "$(Entity<LightSource>::id)".");
            /// detect LightSource entities and add to lightsource render cache ( if lighting is enabled )
            if ( entity->get_serialization_id() == Entity<LightSource>::id && lighting_enabled ) {
                /// recreate LightSource* object from EntityBase
                std::shared_ptr<LightSource> light_source_ptr = std::dynamic_pointer_cast<LightSource>( _entity );
                /// check if cache contains the LightSource already, add if not.
                if ( !lvl.light_sources.contains( entity->entity_id ) ) {
                    gm_log("caching new LightSource at " +
                        Renderer::conv_dp(entity->position)
                        + " with id " + v_str(entity->entity_id)
                        + "."
                    );
                };

                /// add/assign the light source to the light source cache.
                /// We assign the pointer to the light source here as we want
                /// to be able to modify the object within the entity map.
                lvl.light_sources[ entity->entity_id ] = light_source_ptr;
                /// increment the camera's entity count
                lvl.display.camera.entity_count++;
                /// skip rendering of the light source as it's a non-rendering entity.
                continue;
            };

            if ( efficient_render == 1 ) {
                /// since 13.4.0: efficient render for Entities.
                /// Increases draw times a lot see modifiers("efficient_render").
                Renderer::viewpoint vp_e = {
                    entity->position,
                    { entity->position.x + (int)entity->sprite.width,
                           entity->position.y + (int)entity->sprite.height  }
                };

                if ( !viewpoint_includes( lvl.display.viewpoint, vp_e, true ) ) continue;
            };

            /// Increment the camera's entity count
            lvl.display.camera.entity_count++;

            /// Draw the sprite and emplace it to the displacement for the entity.
            texture& drawn_sprite = entity->draw(
#ifdef GMENG_SDL
                Renderer_Type::EXTERNAL
#else
                Renderer_Type::CONSOLE
#endif
            );

            /// assert if the drawn sprite's vector size is not equal to its width*height.
            assert(drawn_sprite.units.size() == drawn_sprite.width*drawn_sprite.height);

            /// Draw each sprite
            for ( int loops = 0; loops < drawn_sprite.width*drawn_sprite.height; loops++ ) {

                /// for debug render
                bool debug = debug_render >= 1;
                /// if the unit is transpert and debug mode is off, skip the unit.
                if ( drawn_sprite.units[loops].transparent && !debug ) continue;

                /// positioning
                int _y = loops  / drawn_sprite.width;  int y = entity->position.y + _y;
                int _x = (loops % drawn_sprite.width); int x = entity->position.x + _x;

                /// get the unit at the position specified in the loop.
                Unit unit = drawn_sprite.units[loops];
                /// captured before the possible debug-border override just below, avoids a second lookup
                bool unit_was_transparent = unit.transparent;

                /// debug mode rendering.
                if ( debug ) {
                    if ( _y == 0 || _y == drawn_sprite.height-1 ||
                         _x == 0 || _x == drawn_sprite.width -1    ) unit = { YELLOW };
                    if ( unit_was_transparent && unit.color != YELLOW ) continue;
                };
                /// place the unit.
                if( y > -1 && x > -1 &&
                    y < trimmed_units.size() &&
                    x < trimmed_units[y].size()) trimmed_units[y][x] = unit;
            };

            /// entity proximity renderer, for debugger showing tracers to
            /// entities the current entity is in proximity to.
            if ( debug_render == 3 ) {
                for ( int entity_id2 = 0; entity_id2 < lvl.entities.size(); entity_id2++ ) {
                    if (entity_id == entity_id2) continue; // skip checking between the same entity twice
                    auto entity2 = lvl.entities.at(entity_id2).get();
                    /// calculate the proximity between two entities using their sprite width/height.
                    /// right now, this does not respect entity->draw(). So entity->draw() output size
                    /// must match entity->sprite.width * entity->sprite.height.
                        auto prox = calculate_proximity(
                            entity->position,  { (int)entity->sprite.width,   (int)entity->sprite.height },
                            entity2->position, { (int)entity2->sprite.width,  (int)entity2->sprite.height });
                    /// if the proximity data is good, add a tracer between the entities.
                    if ( prox.proximity < entity->interaction_proximity && prox.proximity < entity2->interaction_proximity ) {
                        auto path = shortest_path( prox.path );
                        for ( auto dp : path ) {
                            trimmed_units.at(dp.y).at(dp.x) = { .color = PINK };
                        };
                    };
                };
            };
        };

        /// lighting and illumination, cached as this is intensive
        /// calculate the closest light source to each drawpoint
        /// and re-evaluates the color based on the data.

        /// minimum light intensity of the camera
        auto cam_light_floor = lvl.display.camera.modifiers.get_value( "lighting_floor" );

        /// loop over light sources in the level
        for ( const auto& light_source : lvl.light_sources ) {
            /// stop loop if lighting isn't enabled
            if ( !lighting_enabled ) break;
            /// for easy access to the variables, 'light_source.second.xx' is too long
            /// and i am the laziest human being on the planet.

            /// light source entity object id
            const auto& obj_id = light_source.first;
            /// light source object
            const auto& object = light_source.second; // bind by reference, avoids a shared_ptr refcount bump per light source

            DEBUGGER gm_log("LightSource id " + v_str(object->entity_id)
                + " position: " + Renderer::conv_dp(object->position)
                + " cached_position: " + Renderer::conv_dp(object->cached_position)
                + " cached: " + v_str(object->cached));

            /// skip inactive light sources
            /// TODO: FIXME: currently this does not invalidate
            /// level.lighting_cache! even if the light source is inactive
            /// the lighting data from it will be still drawn to the screen.
            if ( !object->active ) continue;
            /// validate the light source's cache. handled internally by
            /// the LightSource entity type. will set object->cached to
            /// false if the cache is invalid, so it can be recalculated.
            object->validate_cache( cam_light_floor );
            /// only apply lighting data if the light source was not cached,
            /// for performance. this allows us to apply lighting data to
            /// moving entities, models and other screen contents with lighting
            /// without having to re-do the lighting every fucking scene.
            /// (unless debug mode is enabled, then do it.)
            if ( object->cached ) continue;
            gm_log("redoing lighting for obj"$(object->entity_id)": cache invalidated");
            /// get the affected drawpoints of the light source
            auto light_displacement = get_radius_displacement(
                    object->intensity,
                    object->position
                );
            /// loop over the drawpoints and cache the effect for each point
            for ( auto dp_val : light_displacement ) {
                /// skip areas that are over the level's max size.
                /// we don't have to account for positions that are < 0
                /// because get_radius_displacement() already normalizes it.
                if ( dp_val.x >= lvl.base.width || dp_val.y/2 >= lvl.base.height ) continue;
                /// ??: fix: divide Y value by 2 for true_position
                auto& unit_at_pos = trimmed_units.at( dp_val.y/2 ).at( dp_val.x );
                /// get lighting effect output and modulation factor
                auto color = color32_t( unit_at_pos.color );
                /// calculate the effect
                auto effect = lvl.display.camera.calculate_illumination(
                        color, object->color,
                        object->intensity,
                        std::sqrt(Renderer::dp_distance( dp_val, object->position )),
                        object->max_brightness,
                        cam_light_floor
                );
                /// store the result unless the brightness value is smaller/equal to
                /// the camera's lighting floor.
                if (effect.brightness > cam_light_floor)
                    lvl.lighting_cache[ dp_val.y*__level_base_width__ + dp_val.x ] = effect;
                // delete the cached value if it is equal or smaller than the lighting_floor
                else lvl.lighting_cache.erase( dp_val.y*__level_base_width__ + dp_val.x );
            };
            /// cache the light source so we don't have to re-render it
            /// every frame, if the light is modified the user should set
            /// lightsource->cached = false, otherwise it won't be updated
            object->cached = true;
            /// additional cache properties for validation and automation

            /// cached minimum brightness, so if the lighting floor
            /// changes the light's previously > light_floor units
            /// don't stay shaded below the lighting floor.
            object->cached_min_brightness = cam_light_floor;
            /// cached position, so if the position of the source
            /// changes the lighting information follows.
            object->cached_position = object->position;
            /// cached intensity, so if the data of the lightsource
            /// changes it reflects immediately without manual cache
            /// invalidation from the level inspector/commandline.
            object->cached_intensity = object->intensity;
        };

        /// save the data and return the renderscale output

        int y = 0, i = 0;
        DEBUGGER gm_log(" lighting cache size: "$( lvl.lighting_cache.size() )".");

        for ( auto& vec : trimmed_units ) {
            int x = 0;
            for ( auto& new_unit : vec ) {
                Renderer::drawpoint _dp = { x, y };

                /// dont touch shit if lighting isn't enabled
                if ( lighting_enabled ) {
                    /// get the RGB value of the unit, so we can blend with the brightness value of the lightsource
                    auto rgb_col = color32_t( new_unit.color );

                    auto state_iterator = lvl.lighting_cache.find( _dp.y*__level_base_width__ + _dp.x );
                    // check if the position of the unit is already in the lightcache_posmap.
                    // if it is, the value is changed instead of being created again
                    if ( state_iterator != lvl.lighting_cache.end() ) {
                        // get the cached lighting data, for its brightness value.
                        // if the cached data exists ( != lvl.lighting_cache.end() ) of the lighting cache, apply the value
                        LightingState cached_lighting = state_iterator->second;
                        // change the surface color to the default color of the unit
                        cached_lighting.surface_color = color32_t( new_unit.color );
                        // apply the lighting data
                        new_unit.color = uint32_from_color32(
                            apply_lighting( cached_lighting )
                        );
                    } else {
                        /// no previously saved lighting data, create it
                        /// right now we do this every frame because it's hard to
                        /// replace cached light data, i'll change it when i have
                        /// a good robust system in place
                        auto effect = lvl.display.camera.calculate_illumination(
                            rgb_col, { 0, 0, 0 },
                            0, 0, 0, cam_light_floor
                        );

                        lvl.lighting_cache[ _dp.y*__level_base_width__ + _dp.x ] = effect;
                        new_unit.color = uint32_from_color32(
                            apply_lighting( effect )
                        );
                    };
                };
                x++;
            };
            y++;
        };

        //lvl.renderscale = trimmed_units;

        return trimmed_units;
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
    inline vector<vector<Unit>> _vget_renderscale2dpartial_scalar(Level& level_t, bool cubic_render = true) {
        return _vconcatenate_lvl_chunks( level_t, cubic_render );
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
    vector<vector<Unit>> get_renderscale(Level& level_t, bool cubic_render = true)  {
        return _vget_renderscale2dpartial_scalar( level_t, cubic_render );
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
    inline std::string get_lvl_view(Level& level_t, vector<vector<Unit>>& trimmed_units, bool cubic_render = true) {
        Renderer::viewpoint& display_vp = level_t.display.viewpoint;
        const int base_width  = level_t.base.lvl_template.width;
        const int base_height = level_t.base.lvl_template.height;

        auto vp_dx = _vcreate_vp2d_deltax( display_vp );
        auto vp_dy = _vcreate_vp2d_deltay( display_vp );

        bool efficient_render = level_t.display.camera.has_modifier("efficient_render");
        bool kitty_graphics = level_t.display.camera.has_modifier("kitty_graphics");

        /// The frame has extra Blobs that
        /// needs to be drawn into the frame.
        if ( !level_t.display.camera.frame_draw_list.empty() ) {
            for ( auto& _blob : level_t.display.camera.frame_draw_list ) {
                /// get the position of the RenderBlob
                int x_start = std::get<0>( _blob ).x,
                    y_start = std::get<0>( _blob ).y;

                /// get the blob itself
                auto blob = std::get<1>( _blob );
                /// don't draw the blob if efficient_render is enabled and the blob
                /// is outside the boundaries of the display's viepwoint. (13.4.0)
                if ( efficient_render ) {
                    if ( !viewpoint_includes(display_vp,
                        {
                            { x_start, y_start },
                            { x_start + (int)blob.width,
                                   y_start + (int)blob.height }
                        }, true) ) continue;
                };

                for ( int i = 0; i < (int)blob.height; i++ ) {
                    for ( int j = 0; j < (int)blob.width; j++ ) {
                        /// unit_at_ij, which is basically the unit
                        /// that is at y = i, x = j, y * width + x
                        const Gmeng::Unit& unit_at_ij = blob.units[ i * blob.width + j ];
                        /// If the unit is transparent, don't draw anything.
                        if (unit_at_ij.transparent) continue;
                        /// Get the X and Y coordinates according to the size of the blob.
                        int x = x_start + j, y = y_start + i;
                        /// Set the unit at(y,x) to the unit_at_ij.
                        trimmed_units[y][x] = unit_at_ij;
                    };
                };
            };
            /// Clear the frame (since blobs are temporary, they should be
            /// updated each frame because they are not part of the actual
            /// draw_list of the entire frame and camera viewpoint.)
            level_t.display.camera.frame_draw_list.clear();
        };

        /// TODO: comments
        if ( efficient_render && kitty_graphics ) {
            if ( level_t.display.camera.display_map.framebuffer == nullptr ||
                 level_t.display.camera.renderer.width  != vp_dx ||
                 level_t.display.camera.renderer.height != vp_dy) {
                level_t.display.camera.display_map.framebuffer =
                    (uint8_t*)realloc( level_t.display.camera.display_map.framebuffer,
                                       vp_dx * vp_dy * 4); /// * 4 because uint8_t is 0.25 * uint32_t
            };

            const uint32_t map_size = (uint32_t)base_width*(uint32_t)base_height;

            #pragma omp parallel for collapse(2) schedule(static)
            for ( int e_y = 0; e_y < vp_dy; e_y++ ) {
                for ( int e_x = 0; e_x < vp_dx; e_x++ ) {
                    int actual_y = display_vp.start.y + e_y,
                        actual_x = display_vp.start.x + e_x;

                    int idx = (e_y * vp_dx + e_x) * 4;

                    if ( actual_y * base_width + actual_x >= map_size ) {
                            level_t.display.camera.display_map.framebuffer[ idx + 0 ] = 0;
                            level_t.display.camera.display_map.framebuffer[ idx + 1 ] = 0;
                            level_t.display.camera.display_map.framebuffer[ idx + 2 ] = 0;
                            level_t.display.camera.display_map.framebuffer[ idx + 3 ] = 255;
                        continue; /// auto-handled by compiler for openmp, im not sure
                    };

                    const Unit& unit_at_yx = trimmed_units[ actual_y ][ actual_x ];

                    const uint32_t& clr = GMENG_COL32( unit_at_yx.color );

                    level_t.display.camera.display_map.framebuffer[ idx + 0 ] = ( clr >> 16 ) & 0xFF;
                    level_t.display.camera.display_map.framebuffer[ idx + 1 ] = ( clr >>  8 ) & 0xFF;
                    level_t.display.camera.display_map.framebuffer[ idx + 2 ] = ( clr       ) & 0xFF;
                    level_t.display.camera.display_map.framebuffer[ idx + 3 ] = 255;
                };
            };
            return "";
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
        const int delta_x = _vcreate_vp2d_deltax(level_t.display.viewpoint);
        const int delta_y = _vcreate_vp2d_deltay(level_t.display.viewpoint);
        const int vp_start_x = level_t.display.viewpoint.start.x;
        const int vp_start_y = level_t.display.viewpoint.start.y;
        const int total_points = delta_x * delta_y;
        // for cubic_render, skips the Y position for already-drawn lines
        int skip_y_position = -1;
        bool notified = false;
        Unit no_un = {
                        .color = BLACK,
                        .special = true, .special_clr = RED,
                        .special_c_unit = '?'
                     };
        auto no_un_drawn = level_t.display.camera.draw_unit( no_un );
        Unit __fallback__ = Unit {.color=PINK,.special=true,.special_clr=WHITE,.special_c_unit='?'};

        auto invalid_unit_rgba_R = ( __fallback__.color >> 16 ) & 0xFF;
        auto invalid_unit_rgba_G = ( __fallback__.color >> 8  ) & 0xFF;
        auto invalid_unit_rgba_B = ( __fallback__.color       ) & 0xFF;

        __final__.reserve( total_points * no_un_drawn.size() );
        /// loop through each drawpoint in the camera.
        for (int dp_i = 0; dp_i < total_points; dp_i++) {
            Renderer::drawpoint dp = {
                .x = vp_start_x + (dp_i % delta_x),
                .y = vp_start_y + (dp_i / delta_x)
            };

            if (dp.y == skip_y_position) {
                ptr++; // still count the unit number
                continue;
            }; notified = false;
            /// append newline if ptr is divisible by the delta X of the viewpoint
            if ((ptr % vp_dx) == 0 && ptr != 0) __final__ += "\n";
            /// valid _vpos
            int _vpos = ( dp.y * (int)(level_t.base.lvl_template.width) + dp.x );

            /// final unit to be placed to the _vpos specified 1d vector id for the 2d unit dp pointer, deltaX
            /// \x0F is a formatter to trim in emplace_lvl_camera
            ///------------------------------------------------------------------------------------------------
            /// fallback in case the viewpoint is out of bounds or extends out of the base level wrapper,
            /// otherwase the application would crash due to heap-extreme-overflow-access-unallocated-mem SEGV.
            #define out_of_bounds(dp)  (dp.x < 0 || dp.y < 0 || dp.x >= level_t.base.lvl_template.width || dp.y >= level_t.base.lvl_template.height)
            #define out_of_cbounds(dp) (dp.x < 0 || dp.y < 0 || dp.x >= level_t.base.lvl_template.width || dp.y >= (level_t.base.lvl_template.height/2))
            if ( out_of_bounds(dp) ) {
                __final__ += no_un_drawn + "\x0f"; ptr++; // v7.1.0 - filler unit for out-of-bounds drawpoint, to prevent crashes
                continue;
            } else {
                /// if the drawpoint's Y coordinate is over trimmed_units.size(), there was a big calculation error.
                if (dp.y == trimmed_units.size()) { gm_log("!! ** ooga-booga problem: trimmed_size(" + v_str(trimmed_units.size()) + ") = dp.y(" + v_str(dp.y) + ") "); continue; };
                Unit __unit;
                /// v8.2.0 / vpos scales with each unit, % operator will chromatize to the current X position
                int true_position = _vpos % level_t.base.lvl_template.width;
                try { __unit = trimmed_units.at(dp.y).at(true_position); } catch(const std::out_of_range& e) {
                    gm_log( "FAULTY_POS: _vpos -> " + v_str(true_position) + " | dp.y -> " + v_str(dp.y) + " | units_size -> " + v_str(trimmed_units.size()));
                    continue;
                };
                /// v8.2.0 / cubic_render -> real pixel-scale 1x1 unit rendering
                if (cubic_render) {
                    /// fallback if fetching the lower-unit fails
                    /// should not happen though, bounds check is performed before this operation
                    Unit& __next_unit = __fallback__;
                    #define _draw level_t.display.camera.draw_unit
                    try { __next_unit = trimmed_units.at(dp.y+1).at(true_position); } catch (const std::out_of_range& e) {
                        __final__ += _draw(__fallback__);
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
                /// 13.4.0-d: Kitty Graphics
                __final__ += "\x0F"; // v8.2.1-d / delimeter fix lol
            };
            ptr++;
            if (cubic_render) skip_y_position = dp.y+1; // v8.2.1-d / skip repeating already-drawn line
        };

        gm_log("get_lvl_view -> level view rendered");
        if (global.debugger) {
              gm_slog(GREEN, "RENDERER OUTPUT FOR get_lvl_view:", "deltaX: " + v_str(_vcreate_vp2d_deltax(level_t.display.viewpoint)) +"\n" + __final__);
        };

        /// remove \n newlines and return base, with formatters to split units
        ASSERT("pref.log", DISABLE());
        replace_all(__final__, "\n", "");
        return __final__;
    };

    /// emplaces unit data to the camera.
    /// This method requires a Gmeng::Level& and a viable rendered_viewpoint as parameters.
    /// to create a rendered_viewpoint use Gmeng::_vget_renderscale2dpartial_scalar(level) and put its return value to get_lvl_view(level, value)
    inline void emplace_lvl_camera(Level& level_t, std::string& cam_data) {
        gm_log("emplace_lvl_camera -> retrieved viable camera data from rendered_viewpoint");
        int p = 0;
        auto deltas_x = _vcreate_vp2d_deltax(level_t.display.viewpoint);
        auto deltas_y = _vcreate_vp2d_deltay(level_t.display.viewpoint);
        if (level_t.display.width  != deltas_x
        ||  level_t.display.height != deltas_y) level_t.display.set_resolution(deltas_x, deltas_y);
        for (const auto& raw_unit : g_splitStr(cam_data, "\x0F")) {
            if (Gmeng::global.debugger) gm_log("emplace_lvl_camera -> overriding renderunit @ " + v_str(p) + " with cam_viewpoint[" + v_str(p) +"]");
            level_t.display.camera.raw_unit_map[p++] = raw_unit + Gmeng::resetcolor;
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


void Gmeng::Renderer::Display::DisplayController::backdrop(const Gmeng::Renderer::drawpoint& offset, std::function<void(DisplayController*)> controller, bool extend_both_sides = true ) const {
    Renderer::viewpoint vp = {
        { 
            std::max(box.start.x - ( extend_both_sides ? offset.x : 0 ), -offset.x),
            std::max(box.start.y - ( extend_both_sides ? offset.y : 0 ), -offset.y)
        },
        {
            std::min(box.end.x + offset.x,
                     vp_width (main_event_loop->level->display.viewpoint)),
            std::min(box.end.y + ( extend_both_sides ? 0 : offset.y ),
                     vp_height(main_event_loop->level->display.viewpoint))
        }
    };
    main_event_loop->level->display.box( vp, controller );
};

#define GMENG_RENDERER_INIT true
#define __GMENG_MODELRENDERER__INIT__ true
