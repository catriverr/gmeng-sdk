#include <algorithm>
#include <cstdint>
#include <iostream>
#include <memory>

#include "lib/bin/easy.h"

#include "../lib/bin/utils/serialization.cpp"
#include "lib/bin/src/renderer.cpp"

using namespace Gmeng;
using namespace Gmeng::Util;

static Level level;
static EventLoop ev;



    int defx = 960, defy = 540;
    int* DEF_DELTAX = &defx;
    int* DEF_DELTAY = &defy;


void reset() {
    _uread_into_vgm("envs/models");

    if (filesystem::exists("envs/city_level.glvl")) read_level_data("envs/city_level.glvl", level);
    level.display.viewpoint =  Gmeng::Renderer::viewpoint { { 0, 0 }, { *DEF_DELTAX, *DEF_DELTAY } };

    ev.level = &(level);
};

Gmeng::color32_t rainbow_next(Gmeng::color32_t clr, int& transitionPhase, int& colorStep, int stepSize) {
    uint8_t r = clr.r,  g = clr.g, b = clr.b;
    colorStep += stepSize;

    if (colorStep >= 255) {
        colorStep = 0;
        transitionPhase = (transitionPhase + 1) % 6;
    }

    switch (transitionPhase) {
        case 0: r = 255; g = colorStep; b = 0; break;
        case 1: r = 255 - colorStep; g = 255; b = 0; break;
        case 2: r = 0; g = 255; b = colorStep; break;
        case 3: r = 0; g = 255 - colorStep; b = 255; break;
        case 4: r = colorStep; g = 0; b = 255; break;
        case 5: r = 255; g = 0; b = 255 - colorStep; break;
    }

    return { r, g, b };
}

int main(int argc, char** argv) {
    _uread_into_vgm("envs/models");
    patch_argv_global(argc, argv); /// gmeng initialization



    /// initially load state
    reset();

    int clr_step = 0, clr_phase = 0, clr_size = 1;

    std::vector<std::vector<Unit>> renderscale;

    ev.add_hook({ EXIT }, [&](Level* level, EventInfo* info) {
        //write_level_data("envs/city_level.glvl", *level);
    });

    ev.add_hook({ INIT }, [&](Level* level, EventInfo* info) {

        level->display.camera.clear_screen();

        level->base.resize(2.0f);

        level->display.viewpoint = { { 0, 0 }, { 960*2, 540*2 } };


        ev.call_event(FIXED_UPDATE, *info);
        level->display.set_cursor_visibility(false);

        //write_level_data("envs/proto_level.glvl", *level);
    });

    bool light_go_boom = false;
    bool test_go_boom = false;
    bool mouse_dragging = false;

    bool entity_spawned = false;
    int id = -1;


    Gmeng::Renderer::drawpoint drag_begin_pos = { -1, -1 };
    Gmeng::Renderer::drawpoint mouse_last_pos = { -1, -1 };
    uint32_t lcolor = 0;

    bool text_shown = false;
    ev.add_hook( { UPDATE }, [&](Level* level, EventInfo* info) {

        if ( text_shown ) level->display.camera.add_text({ 0, 10 },
            "color: " + v_str(lcolor) +
            "\nmouse: " + Renderer::conv_dp( mouse_last_pos ) +
            "\nframe: " + v_str(level->display.camera.frame_time ) +
            "\ndraw:" + v_str(level->display.camera.draw_time) +
            "\nFPS:" + v_str(1000.0f / level->display.camera.draw_time), RED);

        if ( mouse_dragging ) {
            if ( mouse_last_pos.x - drag_begin_pos.x > 0 &&
                 mouse_last_pos.y - drag_begin_pos.y > 0  ) {
                level->display.camera.add_draw( { drag_begin_pos.x, drag_begin_pos.y },
                    BLOB_RECTANGLE_BORDER( mouse_last_pos.x-drag_begin_pos.x, mouse_last_pos.y-drag_begin_pos.y, (uint32_t)WHITE )
                );
            };
        };

        // draw frame
        level->display.refresh(level);
    });

    ev.add_hook({ FIXED_UPDATE }, [&](Level* level, EventInfo* info) {
        if (entity_spawned) {
            auto entity = std::find_if(level->entities.begin(), level->entities.end(), [&](const shared_ptr<EntityBase>& obj) { return obj->entity_id == id; })->get();

            int entity_x_center_pos = entity->position.x + (entity->sprite.width/2);
            int entity_y_center_pos = entity->position.y + (entity->sprite.height/2);

            bool entity_in_x_center = level->display.viewpoint.start.x + ( level->display.width/2 )
                                    == entity_x_center_pos;
            bool entity_in_y_center = level->display.viewpoint.start.y + ( level->display.height/2 )
                                    == entity_y_center_pos;

            if ( getkey( gk_W ) ) {
                entity->position.y--;
                if ( entity_in_y_center ) level->display.move_viewpoint_by( 0, -1 );
            }
            if ( getkey( gk_S ) ) {
                entity->position.y++;
                if ( entity_in_y_center ) level->display.move_viewpoint_by( 0, 1 );
            }
            if ( getkey( gk_A ) ) {
                entity->position.x--;
                if ( entity_in_x_center ) level->display.move_viewpoint_by( -1, 0 );
            }
            if ( getkey( gk_D ) ) {
                entity->position.x++;
                if ( entity_in_x_center ) level->display.move_viewpoint_by( 1, 0 );
            };
        };
    });


    ev.add_hook( { MOUSE_CLICK_LEFT_START, MOUSE_CLICK_LEFT_END, MOUSE_MOVE }, [&](Level* lvl, EventInfo* info) {

            if ( info->EVENT == MOUSE_CLICK_LEFT_START ) {
                Gmeng::Renderer::drawpoint dp = info->RAW_MOUSE_POS;

                for ( auto mdl : lvl->find_models_at( dp ) ) {
                    mdl->highlighted = !mdl->highlighted;
                };
            };


            if ( info->EVENT == MOUSE_MOVE ) {
            mouse_last_pos = { info->RAW_MOUSE_POS.x, info->RAW_MOUSE_POS.y };
            } else if ( info->EVENT == MOUSE_CLICK_LEFT_START ) {
                mouse_dragging = true;
                drag_begin_pos = { info->RAW_MOUSE_POS.x, info->RAW_MOUSE_POS.y };
            } else if ( info->EVENT == MOUSE_CLICK_LEFT_END ) {
                mouse_dragging = false;
            };
    } );

    ev.add_hook( { KEYPRESS }, [&](Level* level, EventInfo* info) {
        if ( info->KEYPRESS_CODE == 'h' ) test_go_boom = !test_go_boom;
        if ( info->KEYPRESS_CODE == 'q' ) {
            ev.cancel();
        };

        if ( info->KEYPRESS_CODE == 't' ) {
            text_shown = !text_shown;
        };

        if ( info->KEYPRESS_CODE == 'z' ) {
            resize_terminal(100, 30, true);
        };

        if ( info->KEYPRESS_CODE == 'p' ) {
            light_go_boom = !light_go_boom;
        }

        if ( info->KEYPRESS_CODE == 'o' ) {
            Gmeng::Animated_Entity tnt_entity;
            tnt_entity.always_animate = true;
            tnt_entity.position = { 0, 0 };
            tnt_entity.anim_textures = {
                vgm_texture("tnt_1"),
                vgm_texture("tnt_2"),
                vgm_texture("tnt_3"),
                vgm_texture("tnt_4")
            };
            id = tnt_entity.entity_id;


            for ( auto& tx : tnt_entity.anim_textures )
                tx.resize(3.5f);

            tnt_entity.modifiers.add_value("fps", 15);

            level->entities.push_back( std::make_shared<Gmeng::Animated_Entity>(tnt_entity) );
            entity_spawned = true;
        };

        if ( info->KEYPRESS_CODE == 'y' ) {
            Gmeng::Animated_Entity animated_entity;
            animated_entity.always_animate = true;

            animated_entity.position = { 0, 0 };
            animated_entity.anim_textures = {
                vgm_texture( "space_shuttle" ),
                vgm_texture( "space_shuttle_booster1" ),
                vgm_texture( "space_shuttle_booster2" )
            };


            id = animated_entity.entity_id;

            level->entities.push_back( std::make_shared<Gmeng::Animated_Entity>(animated_entity) );
            entity_spawned = true;
        };

        return;


    } );


    do_event_loop(&ev);
    std::cout << "end program\n";
    level.display.set_cursor_visibility(true);
    return 0;
};
