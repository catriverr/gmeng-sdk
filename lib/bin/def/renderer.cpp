#include <iostream>
#include <string>
#include <array>
#include <cstring>
#include <vector>
#include <algorithm>
#include <fstream>
#include <map>
#include <random>

#include "../utils/textures.cpp"

#ifdef __GMENG_INIT__
#ifndef __GMENG_MODELRENDERER__INIT__

static void gm_nlog(std::string msg) {
    #ifndef __GMENG_ALLOW_LOG__
        return;
    #endif
    #if __GMENG_ALLOW_LOG__ == true
        std::cerr << msg;
    #endif
};

static void gm_log(std::string msg, bool use_endl = true) {
    #ifndef __GMENG_ALLOW_LOG__
        return;
    #endif
    #if __GMENG_ALLOW_LOG__ == true
        std::cerr << "gm:0 *logger >> " + msg + (use_endl ? "\n" : "");
    #endif
};

static int g_mkid() {
    std::random_device rd; // Random device to seed the generator
    std::mt19937 gen(rd()); // Mersenne Twister 19937 generator
    std::uniform_int_distribution<int> distribution(1000000, 9999999); // 7-digit range

    return distribution(gen);
}



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
              inline void attach_texture(std::string __tf) {
                  this->texture = Gmeng::LoadTexture(__tf);
              };
        };
        struct LevelBase {
            Gmeng::texture lvl_template; std::size_t width; std::size_t height;
        };
        // compiles Renderer::Model objects into a Gmeng::Unit vector
        std::vector<Gmeng::Unit> draw_model(Gmeng::Renderer::Model __m) {
            std::vector<Gmeng::Unit> unitmap;
            for ( int i = 0; i < __m.height; i++ ) {
                for ( int j = 0; j < __m.width; j++ ) {
                    int vpos = (i*__m.width)+j;
                    unitmap.push_back(__m.texture.units[vpos]);
                };
            };
            return unitmap;
        };
        std::size_t getsize(drawpoint c) {
            return (c.x * c.y);
        };
        std::string conv_dp(drawpoint p) {
            return "x=" + std::to_string(p.x) + ",y=" + std::to_string(p.y);
        };
        // returns placement coordinates for each coordinate of object at __p with size of __s in mapsize of __ws
        std::vector<Gmeng::Renderer::drawpoint> get_placement(drawpoint __p, drawpoint __s, drawpoint __ws) {
            gm_log("gm::v_renderer -> get_placement : pvalues = 1: " + conv_dp(__p) + " 2: " + conv_dp(__s) + " 3: " + conv_dp(__ws));
            if (getsize(__p) > getsize(__ws) || getsize(__s) > getsize(__ws)) throw std::invalid_argument("placement parameters invalid");
            std::vector<Gmeng::Renderer::drawpoint> vec;
            if ((__p.x + __s.x <= __ws.x) && (__p.y + __s.y <= __ws.y)) {
                for (int i = 0; i < __s.x; ++i) {
                    for (int j = 0; j < __s.y; ++j) {
                        Gmeng::Renderer::drawpoint point = {.x=__p.x + i, .y=__p.y + j};
                        vec.push_back(point);
                    };
                };
            } else { std::cerr << "Gmeng::Renderer::get_placement: e_obj: out of boundaries: __getsize(__p, __s, __ws);" << std::endl; };
            return vec;
        };
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
                inline void draw() {
                    for (int i = 0; i < this->rendered_units.size(); i++) {
                        gm_log("Gmeng::Renderer::Display preview @ v_unit:" + std::to_string(i) + " -> PREVIEW = " + this->camera.draw_unit(this->rendered_units[i]));
                        this->camera.display_map.unitmap[i] = this->rendered_units[i];
                        gm_log("Gmeng::CameraView preview @ v_unit:" + std::to_string(i) + " -> PREVIEW = " + this->camera.draw_unit(this->camera.display_map.unitmap[i]));
                        this->camera.raw_unit_map[i] = this->camera.draw_unit(this->rendered_units[i]);
                    };
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
        int width  = xy % wmp_x;
        int height = xy / wmp_x;
        return Objects::coord { .x = width, .y = height };
    };
    inline void log_vpu(std::vector<Gmeng::Unit>& vp_units, int wsizeX) {
        int jndx = 0;
        for (const auto& _v_unit : vp_units) { Objects::coord gp = Gmeng::trace_1dp(jndx, wsizeX); gm_nlog("vp_gtx.units @ pos "+ std::to_string(jndx) + " ("+std::to_string(gp.y)+","+std::to_string(gp.x)+"):\n\tv_color -> " + std::to_string(_v_unit.color) + "\n\tv_collision -> " + (_v_unit.collidable ? "true" : "false") + "\n\tv_special -> " + (_v_unit.special ? "true" : "false") + "\n\tv_spcolor -> " + std::to_string(_v_unit.special_clr) + "\n"); jndx++; };
        gm_nlog("\n");
    };
    inline void log_vpc(Gmeng::r_chunk vp_chunk) {
        gm_nlog("vp_chunk->vpc_info : r_chunk & Gmeng::r_chunk & Gmeng::Renderer::Model & Gmeng::Texture & gm_vpcontrol\n");
        gm_nlog("metadata:\n");
        gm_nlog("\tv_viewpoint -> start = " + Gmeng::Renderer::conv_dp(vp_chunk.vp.start) + " - end = " + Gmeng::Renderer::conv_dp(vp_chunk.vp.end) + "\n");
        gm_nlog("\tv_models:\n");
        for (const auto& v_mdl : vp_chunk.models) {
            gm_nlog("\t   m_name -> " + v_mdl.name + "\n");
            gm_nlog("\t\tm_id -> " + std::to_string(v_mdl.id) + "\n");
            gm_nlog("\t\tm_width -> " + std::to_string(v_mdl.width) + "\n");
            gm_nlog("\t\tm_height -> " + std::to_string(v_mdl.height) + "\n");
            gm_nlog("\t\tm_posX -> " + std::to_string(v_mdl.position.x) + "\n");
            gm_nlog("\t\tm_posY -> " + std::to_string(v_mdl.position.y) + "\n");
            gm_nlog("\t\tm_size -> " + std::to_string(v_mdl.size) + "\n");
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
        std::string f_content = g_readFile(__fn); Gmeng::LevelInfo info;
        std::vector<std::string> lines = g_splitStr(f_content, "\n");
        gm_log("parsing glvl file: " + __fn + " -> Gmeng::LevelInfo");
        std::map<std::string, Gmeng::texture> textures;
        std::map<std::string, Gmeng::Renderer::Model> models;
        std::vector<int> resolution = {5, 5};
        gm_log("glvl_v vector_init -> success");
        int indx = 0;
        gm_log("glvl_v inl loadF load_file start");
        for ( const auto& ln : lines ) {
            /// indx equ ln_num
            std::vector<std::string> params = g_splitStr(ln, " ");
            gm_log(__fn + ": gl_v->line @ " + std::to_string(indx) + ": " + ln);
                 if ( indx == 0 ) { info.name = params[0].substr(5); gm_log("glvl->name : SETNAME = " + params[0] + " -> success"); }
            else if ( indx == 1 ) { info.description = params[0].substr(5); gm_log("glvl->desc : SETDESC = " + params[0] + " -> success"); }
            else if ( indx == 2 ) {
                gm_log("glvl->base.template : SETTEXTURE = " + params[0] + " -> proc.init() -> success");
                std::string vp_name = params[0].substr(13);
                Gmeng::texture vp_gtx = Gmeng::LoadTexture(vp_name);
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
                    if (params.size() < 7) { std::cerr << "gm:0/c_err: Gmeng::parse_glvl -> kw_model : ERR_PARAMS: at(line="+ std::to_string(indx) +",ch="+std::to_string(ln.length())+") -> expected 7 params, got " + std::to_string(params.size()) + "." << endl; indx++; continue; };
                    std::string _m_name       = params[1];
                    int _m_posX               = std::stoi(params[2].substr(3));
                    int _m_posY               = std::stoi(params[3].substr(3));
                    int _m_width              = std::stoi(params[4].substr(2));
                    int _m_height             = std::stoi(params[5].substr(2));
                    Gmeng::texture _m_texture = textures[params[6].substr(3)];
                    models[_m_name] = Gmeng::Renderer::Model {
                        .name=_m_name,.id=g_mkid(),
                        .width=static_cast<std::size_t>(_m_width),.height=static_cast<std::size_t>(_m_height),
                        .position = { .x=_m_posX,.y=_m_posY },
                        .size=static_cast<std::size_t>(_m_posX*_m_posY), .texture=_m_texture
                    };
                    gm_log("glvl->models : push_back() -> v_static_cast<std::size_t> : *m_texture:load *m_metadata:load v_status -> success");
                }
                else if ( keyword == "#texture" ) {
                    if (params.size() < 2) { std::cerr << "gm:0/c_err: Gmeng::parse_glvl -> kw_texture : ERR_PARAMS: at(line="+ std::to_string(indx) +",ch="+std::to_string(ln.length())+") -> expected 2 params, got " + std::to_string(params.size()) + "." << endl; indx++; continue; };
                    textures[params[1]] = Gmeng::LoadTexture(params[2]);
                    gm_log("glvl->textures : push_back() -> v_static_cast<std::size_t> : *m_texture:load *m_metadata:load v_status -> success");
                }
                else if ( keyword == "#chunk" ) {
                    /// rest | #chunk p1x=0 p1y=0 p2x=4 p2y=4 model_name1,model_name2
                    if (params.size() < 6) { std::cerr << "gm:0/c_err: Gmeng::parse_glvl -> kw_chunk : ERR_PARAMS: at(line="+ std::to_string(indx) +",ch="+ std::to_string(ln.length()) +") -> expected 6 params, got " + std::to_string(params.size()) + "." << endl; indx++; continue; };
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
                    if (params.size() > 0) std::cerr << "gm:0/c_err: Gmeng::parse_glvl() -> ERR_INVALID_KEYWORD: at(line=" + std::to_string(indx) +",ch=0)" << endl;
                }
            };
            indx++;
        };
        info.display_res.push_back(resolution[0]);
        info.display_res.push_back(resolution[1]);
        return info;
    };

    class Level {
        private:
            // compiles a chunk into a std::vector<Gmeng::Unit> unitmap for a CameraView instance to render
            inline std::vector<Gmeng::Unit> render_chunk(Gmeng::r_chunk chunk) {
                // write base_template skybox image to chunk (level 0 of canvas)
                Gmeng::texture* base_map = &this->base.lvl_template; std::vector<Gmeng::Unit> units;
                gm_log("init OK" );
                gm_log("base_map -> WIDTH: " + std::to_string(base_map->width) + " - HEIGHT: " + std::to_string(base_map->height) );
                std::vector<Gmeng::Renderer::drawpoint> wrapper = Gmeng::Renderer::get_displacement(chunk.vp, Renderer::drawpoint {.x=static_cast<int>(base_map->width),.y=static_cast<int>(base_map->height)});
                gm_log("get_displacement OK" );
                gm_log("chunk viewpoint -> start -> x: " + std::to_string(chunk.vp.start.x) + " - y: " + std::to_string(chunk.vp.start.y) + " | end -> x: " + std::to_string(chunk.vp.end.x) + " - y: " + std::to_string(chunk.vp.end.y) );
                gm_log("wrapper viewpoints -> " + std::to_string(wrapper.size()) );
                gm_log("get_displacement -> " + std::to_string(wrapper.size()) + " drawpoints" );
                for ( const auto& dp : wrapper ) {
                    int vpos = (dp.y * base_map->width)+dp.x;
                    Gmeng::Unit v_unit = base_map->units[vpos];
                    units.push_back(v_unit);
                };
                gm_log("write_drawpoint -> base_map.units OK" );
                // compile models into std::vector<Gmeng::Unit> unitmaps and write them to x,y coordinates
                // within the r_chunk by getting placement positions with Gmeng::Renderer::get_placement
                for ( const auto& model : chunk.models ) {
                    Gmeng::Renderer::drawpoint dp = model.position;
                    gm_log("dp_loadmodel " + std::to_string(model.id) + " OK" );
                    gm_log("dp -> x: " + std::to_string(dp.x) + " - y: " + std::to_string(dp.y) );
                    gm_log("p2 -> x: " + std::to_string(model.width) + " - y: " + std::to_string(model.height) );
                    std::vector<Gmeng::Renderer::drawpoint> displacement = Gmeng::Renderer::get_placement(dp, {.x=static_cast<int>(model.width),.y=static_cast<int>(model.height)}, {.x=static_cast<int>(this->display.width),.y=static_cast<int>(this->display.height)});
                    for ( auto dp : displacement ) {
                        gm_log("displacement_log = x: " + std::to_string(dp.x) + " - y: " + std::to_string(dp.y) );
                    };
                    gm_log("total_drawpoints: " + std::to_string(displacement.size()) );
                    gm_log("get_placement OK" );
                    std::vector<Gmeng::Unit> unitmap = Gmeng::Renderer::draw_model(model);
                    gm_log("draw_model OK: unitmap.size(): " + std::to_string(unitmap.size()) );
                    int lndx = 0;
                    for ( const auto& unit : unitmap ) {
                        gm_log(std::to_string(lndx) +" <- pos_vdp: rendering_model_unit PREVIEW: " + this->display.camera.draw_unit(unit) );
                        int _vdp_pos = (displacement[lndx].y*this->display.width)+displacement[lndx].x;
                        gm_log("_vdp_pos find: " + std::to_string(_vdp_pos) +" OK" );
                        gm_log("_vdp_current -> x: " + std::to_string(displacement[_vdp_pos].x) + " - y: " + std::to_string(displacement[_vdp_pos].y) );
                        gm_log("_vdp_current_addr -> " + this->display.camera.draw_unit(units[_vdp_pos]) );
                        gm_log("swap_unit: at(" + std::to_string(_vdp_pos) +") -> PREVIEW: " + this->display.camera.draw_unit(units[_vdp_pos]) + " TO unit() -> PREVIEW: " + this->display.camera.draw_unit(unit) );
                        units[_vdp_pos] = unitmap[lndx];
                        gm_log("set_unit_at(id) OK" );
                        lndx++;
                    };
                };
                return units;
            };
            inline Gmeng::r_chunk get_chunk(int id) {
                return this->chunks[id];
            };
            inline void set_chunk(int id, Gmeng::r_chunk chunk) {
                this->chunks[id] = chunk;
            };
        public:
            Gmeng::Renderer::LevelBase base; Gmeng::Renderer::Display display;
            std::vector<Gmeng::r_chunk> chunks; std::string desc; std::string name;
            inline int load_chunk(Gmeng::r_chunk chunk) {
                this->chunks.push_back(chunk);
                return (this->chunks.size()-1);
            };
            inline void load_level(Gmeng::LevelInfo __glvl) {
                int i,j = 0;
                gm_log("vp_loadlevel: __glvl chunks v_size: " + std::to_string(__glvl.chunks.size()));
                for (const auto& chunk : __glvl.chunks) {
                    Gmeng::log_vpc(chunk);
                    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
                    if (i >= this->chunks.size()) { this->load_chunk(chunk); continue; };
                    this->chunks[i] = chunk;
                    i++;
                };
                this->base.height = __glvl.base.height;
                this->base.width  = __glvl.base.width;
                this->display.width  = __glvl.display_res[0];
                this->display.height = __glvl.display_res[1];
                this->base.lvl_template.collidable = __glvl.base.lvl_template.collidable;
                this->base.lvl_template.height = __glvl.base.height;
                this->base.lvl_template.width  = __glvl.base.width;
                gm_log("class Model { ... } : load_level(...) -> base_template @ h+w = " + std::to_string(__glvl.base.height) + "," + std::to_string(__glvl.base.width));
                this->base.lvl_template.name   = __glvl.base.lvl_template.name;
                for ( const auto& _v_unit : __glvl.base.lvl_template.units ) {
                    if (j >= this->base.lvl_template.units.size()) { this->base.lvl_template.units.push_back(_v_unit); continue; };
                    this->base.lvl_template.units[j] = _v_unit;
                    j++;
                };
                this->desc = __glvl.description;
                this->name = __glvl.name;
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
                    if (v_camview >= this->display.rendered_units.size()) { this->display.rendered_units.push_back(v_unit); continue; };
                    this->display.rendered_units[v_camview] = v_unit;
                    v_camview++;
                };
                this->display.draw();
            };
    };
};
#define __GMENG_MODELRENDERER__INIT__ true
#endif
#endif
