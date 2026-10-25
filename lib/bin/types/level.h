#pragma once

#include "entity.h"
#include "texture.h"
#include "model.h"
#include "display.h"

#include "../src/objects.cpp"

#include "../utils/viewpoint.h"
#include "../utils/gridmap.cc"

namespace Gmeng {

    namespace Renderer {

        /// 'Skybox' implementation in Gmeng. Basically, this 'base'
        /// is the underlying texture of a `Gmeng::Level` instance.
        /// All chunks within a level are drawn on top of this texture
        /// and this also defines the boundaries of a Level.
struct LevelBase {
            /// texture of the level base (skybox)
            Gmeng::texture lvl_template;
            /// Width of the level base as well as the maximum width
            /// of the parent level instance.
            std::size_t width;
/// Height of the level base as well as the maximum height
            /// of the parent level instance.
            std::size_t height;

            void resize( float scale ) {
                lvl_template.resize( scale );
                this->width = width*scale;
                this->height = height*scale;
            };

            void resize( const std::size_t& n_width, const std::size_t& n_height ) {
                lvl_template.resize( n_width, n_height );
                this->width = n_width;
                this->height = n_height;
            };

            GMENG_INIT_TYPE( lvl_template, width, height );
        };

    };
    // chunk of a `Gmeng::Level` instance. similar to how minecraft handles chunks.
    // in `Gmeng::Level::render_chunk()` a `std::vector<Gmeng::r_chunk> chunks` is loaded.
    // these chunks contain { Gmeng::Renderer::viewpoint vp; } metadata. The chunk's
    // `std::vector<Gmeng::Renderer::Model>` models object is recieved and compiled into a unitmap.
    struct chunk {
        /// Viewpoint of the chunk. Defines the part of the Level that this
        /// chunk is responsible for containing Model and Entity data for.
        Renderer::viewpoint vp;
        /// Models within the chunk.
        vector<Renderer::Model> models;
        /// Container for the rendered units of the chunk.
        /// Used by Level::v_render_chunk or Level::render_chunk,
        /// stores default initialization of the vector.
        vector<Unit> rendered_units;

        GMENG_INIT_TYPE( vp, models, rendered_units );
    };
    // levelinfo (parsed into Gmeng::Level::load_level after Gmeng::parse_glvl())
    struct LevelInfo {
        Renderer::LevelBase base;
        std::string name;
        vector<int> display_res;
        vector<chunk> chunks;
        std::string description;
    };


    /// Level type of Gmeng.
    /// This is more a 'level manager' than a level itself,
    /// but nontheless it holds the display and camera that
    /// initialize the cameraview and the level object.
    ///
    /// @since 4.1_glvl
    /// @continuous_build 8.0.0
    class Level {
        private:
            // compiles a chunk into a vector<Unit> unitmap for a Camera instance to render.
            // this method can also be used externally to render any chunk, it does not require
            // for the chunk to  be a member of the Level class.
            //
            // currently used in get_renderscale, get_lvl_view & emplace_lvl_camera.
            //
            // Does not render entities!
            inline vector<Unit> render_chunk(chunk& chunk) {
                int vp_dx = vp_width( chunk.vp );
                int vp_dy = vp_height( chunk.vp );
                // write base_template skybox image to chunk (level 0 of canvas)
                // base_template's viewpoint relative to the chunks viewpoint will be drawn as a base 'skybox' like image
                texture* base_map = &this->base.lvl_template;

                chunk.rendered_units.assign( vp_dx*vp_dy, { .transparent=true } );
                vector<Unit>& units = chunk.rendered_units;

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
//                for ( const auto& dp : wrapper ) {
//                    if ( dp.y > this->base.height || dp.x > this->base.width )
//                        {units.push_back({.transparent=true}); continue; };
//                    /// get the displacement
//                    int vpos = (dp.y * base_map->width)+dp.x;
//                    // units.at(x) provides error logging and throws exceptions.
//                    // otherwise if the base_map is empty units[x] would
//                    // cause a segmentation fault and exit the program.
//                    auto v_unit = base_map->units[(vpos)];
//
//                    units.push_back(v_unit);
//                };
//
                /* there was a for loop for a preview, but since it's unused it has been removed since 10.4.0-b. */

                // compile models into unitmaps and write them to x,y coordinates
                // within the r_chunk by getting placement positions with get_placement
                int debug_render_mode = this->display.camera.modifiers.get_value("debug_render");

                bool debug_render_mode_is_2 = debug_render_mode == 2;
                // efficient_render mode. Various optimizations that make rendering chunks faster,
                // but can sometimes cause issues with external game code. However, since these are
                // almost completely nonexistent, we can get away with enabling efficient_render by
                // default. For any edge-case that requires it to be disabled, this option is
                // dynamic and can be changed with set_value("efficient_render", 0).
                int efficient_render = this->display.camera.modifiers.get_value("efficient_render");
                // for ease we just reset the model count here
                this->display.camera.model_count = 0;

                // loop through the models wthin the chunk
                // (if there are any)
                if ( !chunk.models.empty() )
                for ( auto& _model : chunk.models ) {
                    /// Add to model registry if it doesn't exist yet
                    /// (very inefficient to be setting and resetting
                    ///  a vector of models, needs to be fixed.)
                    if (!this->modelmap.contains( _model.id )) this->modelmap[_model.id] = &( _model );
                    /// if the model is not active, skip rendering it.
                    if (!_model.active) continue;
                    /// Check if the model is within the bounds
                    /// of the viewpoint. This will not render
                    /// models that are outside the screen.
                    /// (only when efficient_render is enabled)
                    if ( efficient_render == 1 ) {
                        Gmeng::Renderer::viewpoint vp_m = {
                            { chunk.vp.start.x + _model.position.x,
                                     chunk.vp.start.y + _model.position.y },
                              { chunk.vp.start.x + _model.position.x + (int)_model.width,
                                     chunk.vp.start.y + _model.position.y + (int)_model.height }
                        };

                        if ( !Gmeng::viewpoint_includes( this->display.viewpoint, vp_m, true ) ) continue;
                    };
                    /// Render model
                    auto& model = _model;
                    /// Update the model count of the camera
                    this->display.camera.model_count++;
                    /// if the model is highlighted or the modifier
                    /// 'debug_render' is enabled at 2 or 1, highlight
                    /// the model's borders (or its body).
                    const texture& sprite = model.get_sprite( debug_render_mode > 0 );

                    /// model position remapping. to prevent errors such as
                    /// segmentation faults & other crashes if the displacement of
                    /// a model is outside the bounds of the level->base.

                    /// coordinate remapping ( > bounds: overbound )
                    if ( model.position.x+model.width >= vp_dx )
                        model.position.x = model.position.x % (vp_dx - model.width);
                    if ( model.position.y+model.height >= vp_dy )
                        model.position.y = model.position.y % (vp_dy - model.height);
                    /// coordinate remapping ( < bounds: underbound )
                    if (model.position.x < 0)
                        model.position.x = vp_dx-model.width;
                    if (model.position.y < 0)
                        model.position.y = vp_dy-model.height;

                    /// dp = beginning of the model's actual draw position.
                    /// We use `position mod chunk.vp` here to make sure
                    /// the model is always within the bounds of the
                    /// chunk's viewpoint, even if the model position
                    /// is in negative coordinates or beyond the viewpoint.
                    Renderer::drawpoint dp = { model.position.x %vp_dx,
                                               model.position.y %vp_dy };

                    vector<Renderer::drawpoint> displacement = Renderer::get_placement(dp,
                            {.x=static_cast<int>(model.width),.y=static_cast<int>(model.height)},
                            {.x=(vp_dx),.y=(vp_dy)});

                    int lndx = 0;
 
                    for ( const auto& unit : sprite.units ) {

                        int _vdp_pos = ((displacement[lndx].y)*(vp_dx + 1))+displacement[lndx].x;


                        if (unit.transparent) {
                            if (!debug_render_mode_is_2) {
                                lndx++; continue;
                            } else {
                                units[_vdp_pos] = { .color=GREEN };
                                lndx++;
                                continue;
                            };
                        }
                        if (debug_render_mode_is_2) {
                            units[_vdp_pos] = {.color = unit.collidable ? WHITE : BLACK};
                            lndx++;
                            continue;
                        };

                        //// 10.4.0 - this log was also moved to full log only mode
                        //if (Gmeng::global.dont_hold_back) {
                        //    if (_vdp_pos < (displacement).size()) {
                        //        gm_log("_vdp_pos find: " + v_str(_vdp_pos) +" OK" );
                        //        gm_log("_vdp_current -> x: " + v_str(displacement[_vdp_pos].x) + " - y: " + v_str(displacement[_vdp_pos].y) );
                        //        gm_log("_vdp_current_addr -> " + this->display.camera.draw_unit(units[_vdp_pos]) );
                        //        gm_log("swap_unit: at(" + v_str(_vdp_pos) +") -> PREVIEW: " + this->display.camera.draw_unit(units[_vdp_pos]) + " TO unit() -> PREVIEW: " + this->display.camera.draw_unit(unit) );
                        //    } else {
                        //        gm_log(Gmeng::colors[YELLOW] + "WARN!" + Gmeng::colors[WHITE] + " possible invalid _vdp_pos, clarification methods disabled");
                        //        gm_log(Gmeng::colors[CYAN] + "TRACE to pointer of " + Gmeng::colors[GREEN] + "_vdp_pos" + Gmeng::colors[CYAN] + ":");
                        //        gm_log("@== " + Gmeng::colors[PINK] + "_vdp_pos" + Gmeng::colors[CYAN] + " ==@");
                        //        gm_log(colors[WHITE] + "\t#0 " + Gmeng::colors[WHITE] + _uconv_1ihx(_uget_addr(displacement)) + " " + Gmeng::colors[GREEN] + "displacement(" + v_str(displacement.size()) + ") [ lndx ]");
                        //        gm_log(colors[WHITE] + "\t#1 " + Gmeng::colors[WHITE] + _uconv_1ihx(_uget_addr(displacement[lndx].x)) + "\t" + Gmeng::colors[CYAN] + "*(self) || coord.y_pointer [" + v_str(displacement[lndx].y)+ "] * this->display.width [ " + v_str(this->display.width) + " ] " + colors[PINK] + "== " + colors[WHITE] + v_str(displacement[lndx].y * this->display.width));
                        //        gm_log(colors[WHITE] + "\t#2 " + Gmeng::colors[WHITE] + _uconv_1ihx(_uget_addr(displacement[lndx].y)) + "\t" + Gmeng::colors[CYAN] + "*(self) || coord.x_pointer [" + v_str(displacement[lndx].x) +"]");
                        //        gm_log(colors[CYAN] + "@== formulae -> " + Gmeng::colors[GREEN] + "(displacement[lndx].y*this->display.width)+displacement[lndx].x" + Gmeng::colors[CYAN] + " ==@");
                        //        gm_log(colors[YELLOW] + "WARN!" + colors[WHITE] + " consider investigation of this ccode, since it may occur when _vdp_pos is outbound from sizeof(displacement)");
                        //    };
                        //    gm_log(v_str(unitmap.size()) + " ACCESS_TO_INDEX: " + v_str(lndx) + " APPLIED_FROM_SIZED: " + v_str(_vdp_pos) + " TO_SIZED_VECTOR_OF: " + v_str(units.size()));
                        //    gm_log("set_unit_at(id: " + v_str(_vdp_pos) + ") OK" );
                        //};

                        units[_vdp_pos] = unit;
                        lndx++;
                    };
                };

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
            /// for displacement_cache
        struct displacement_cache_entry {
                Renderer::Model* model; std::vector<Renderer::drawpoint> displacements;
            };
        public:
            /// stores every displacement of the level's models
            /// mapped by the models' ids. Used for shadows &
            /// ray tracing in Gmeng::get_renderscale()
            std::map< int, displacement_cache_entry > displacement_cache;
            /// Map of light sources for each LightSource entity,
            /// mapped by LightSource->entity_id. cached & generated
            /// by Gmeng::get_renderscale().
            std::map< int, std::shared_ptr<LightSource> > light_sources;
            /// Contains a map of illumination and lighting info
            /// for each drawpoint in the level. The minimum light
            /// intensity is set by the camera (level.display.camera)
            /// or (Gmeng::Camera). The minimum light intensity is a
            /// modifier and can be serialized and saved to level
            /// files or any stdc++ outstream.
            ///
            /// Lighting cache is initialized by Gmeng::get_renderscale and
            /// stored in Gmeng::Level::lighting_cache, but lighting states
            /// for every drawpoint is calculated by Gmeng::Camera. via
            /// Gmeng::Camera::calculate_illumination( color32_t color,
            /// int light_intensity_max, int distance, int min_intensity).
            ///
            /// Lighting can be disabled by setting the modifier on
            /// Gmeng::Camera::modifiers( "lighting", 0 ) with the method
            /// Gmeng::Camera::set_modifier( "lighting", 0 ).
            ///
            /// The minimum lighting level (min_intensity) can also be set
            /// with Gmeng::Camera::set_modifier( "lighting_floor", 0 ).
            std::unordered_map< uint32_t, LightingState > lighting_cache;
            /// Contains a copy of each model included in the level.
            /// Does not contain a copy of each individual model's derivatories,
            /// only models that weren't initialized before.
            std::map<int, Renderer::Model*> modelmap;
            /// Available renderscale for the level.
            /// Not updated internally, is set by get_renderscale(),
            /// _vconcatenate_lvl_chunks() or _vget_renderscale2dpartial_scalar().
            ///
            /// For ease-of-access in preview / debugging utilities without having to
            /// recompile the renderscale from the start by using this shared object.
            vector<vector<Unit>> renderscale;
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
            vector<std::shared_ptr<EntityBase>> entities;
        /// The Level's interaction grid map.
            /// The level is divided into 32x32 cells that
            /// contain buckets of entities, which makes
        /// checking for proximity between entities
            /// easier
            GridMap<Gmeng::EntityBase> entity_grid;
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

            /// Caching modifier for the Level::base object,
            /// the level template is only drawn once (unless
            /// a `redo_levelbase` command is called) so we
            /// cache it in _vconcatenate_lvl_chunks().
            bool base_init = false;
            /// Cached rows of the base object.
            std::vector<std::vector<Unit>> CACHED_BASE;

            GMENG_INIT_TYPE( displacement_cache, light_sources, lighting_cache,
                             modelmap, renderscale, base, display, chunks,
                             entities, entity_grid, desc, name, base_init );

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

            /// Returns the rendered output of a chunk
            /// at the given id.
            ///
            /// @since 4.1-glvl
            inline vector<Unit> get_rendered_chunk(int id) {
                __functree_call__(Gmeng::Level::get_rendered_chunk);
                return this->render_chunk(this->chunks.at(id));
            };

            /// Renders a given chunk with the internal chunk
            /// renderer for the level. does not add to chunk map.
            ///
            /// @since 7.0.0-d
            inline vector<Unit> v_render_chunk(chunk& chunk) {
                return this->render_chunk(chunk);
            };

            /// Returns every model that covers a specified drawpoint.
            /// For example,
            /// if find_models_at( 10, 10 ) is called and there is;
            ///     Model( { 0,0 -> 15, 15 } ),
            ///     Model( { 5,5 -> 10, 10 } ),
            /// these models will be returned as pointers.
            ///
            /// This method, by default, will ignore a model if the
            /// search drawpoint is covered by it, but is a transparent unit.
            /// Set `including_transparent` to true to include them.
            ///
            /// This method ignores inactive models.
            inline vector<Renderer::Model*> find_models_at( Renderer::drawpoint dp, bool including_transparent = false ) {
                vector<Renderer::Model*> models;
                /// Search every chunk.
                for ( auto& chunk : this->chunks ) {
                    /// Seach the chunk's models
                    for ( auto& model : chunk.models ) {
                        if ( viewpoint_includes_dp( Renderer::model_view( model ), dp ) )
                            models.push_back( &model );
                    };
                };

                return models;
            };
    };

};

#ifndef GMENG_NO_SOL

#include "../utils/scripts.h"
static int __gmeng_define_lua_level_method = ([]() -> int {
    using namespace Gmeng;
    using namespace Scripts;
    using namespace Renderer;

    renderer_lua_namespace.new_usertype<Renderer::LevelBase>("LevelBase",
        "lvl_template", &LevelBase::lvl_template,
        "width", &LevelBase::width,
        "height", &LevelBase::height
    );

    gmeng_lua_namespace.new_usertype<chunk>("Chunk",
        "vp", &chunk::vp,
        "models", &chunk::models,
        "rendered_units", &chunk::rendered_units
    );

    gmeng_lua_namespace.new_usertype<LevelInfo>("LevelInfo",
        "base", &LevelInfo::base,
        "name", &LevelInfo::name,
        "display_res", &LevelInfo::display_res,
        "chunks", &LevelInfo::chunks,
        "description", &LevelInfo::description
    );

    gmeng_lua_namespace.new_usertype<Level>("Level",
        "renderscale", &Level::renderscale,
        "base", &Level::base,
        "display", &Level::display,
        "chunks", &Level::chunks,
        "desc", &Level::desc,
        "name", &Level::name,
        "base_init", &Level::base_init,
        "CACHED_BASE", &Level::CACHED_BASE
    );

    return 0;
})();
#endif

