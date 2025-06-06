#include <iostream>

#include "../lib/bin/gmeng.h"
#include "../lib/bin/utils/serialization.cpp"

using namespace Gmeng;
using namespace Gmeng::Util;

static Level level;
static gmeng_properties_t cfg;
static EventLoop ev;

/// Example Entity Type
class Mogus : public Entity<Mogus> {
  public:
    int interaction_proximity = 10;
}; REGISTER_ENTITY_TYPE( Mogus ); /// Registers the Entity Type.



void reset() {
    _uread_into_vgm("envs/models");
    if (!filesystem::exists("gamestate.cfg")) writeout_properties("gamestate.cfg", default_properties);
    cfg = read_properties("gamestate.cfg");

    texture empty_texture = Renderer::generate_empty_texture(cfg.SKY_WIDTH, cfg.SKY_HEIGHT);
    texture_fill(empty_texture, cfg.SKY_COLOR);
    level_set_skybox(&level, empty_texture);

    texture cake_texture = default_texture_search("smol_player");
    texture_replace_color(cake_texture, RED, GREEN);

    texture island_texture = default_texture_search("01_island");
    texture gift_texture = default_texture_search("02_gift");
    texture real_cake_texture = default_texture_search("01_cake_txtr");
    texture table_texture = default_texture_search("03_table");

    texture balloon_texture = default_texture_search("05_balloon");
    texture balloon_texture_2 = default_texture_search("05_balloon");
    texture_replace_color(balloon_texture_2, RED, GREEN);

    level.chunks.at(0) = { { {0, 0}, {99, 99} },
        {
            model_from_txtr( cake_texture, cfg.model_positions["player"] ), // this is actually the player

            model_from_txtr(table_texture, cfg.model_positions["table1"]),
            model_from_txtr(table_texture, cfg.model_positions["table2"]),

            model_from_txtr(real_cake_texture, cfg.model_positions["cake"]),

            model_from_txtr(gift_texture, cfg.model_positions["gift1"]),
            model_from_txtr(gift_texture, cfg.model_positions["gift2"]),

            model_from_txtr(balloon_texture, cfg.model_positions["balloon1"]),
            model_from_txtr(balloon_texture_2, cfg.model_positions["balloon2"]),
        }
    };

    level.display.viewpoint = { {0,0}, { cfg.DEF_DELTAX, cfg.DEF_DELTAY } };
    level.display.set_resolution(cfg.DEF_DELTAX, cfg.DEF_DELTAY);

    if (filesystem::exists("envs/proto_level.glvl")) read_level_data("envs/proto_level.glvl", level);

    ev.level = &level;

};

int main(int argc, char** argv) {
    _uread_into_vgm("envs/models");
    patch_argv_global(argc, argv); /// gmeng initialization

    if (!filesystem::exists("gamestate.cfg")) writeout_properties("gamestate.cfg", default_properties);
    cfg = read_properties("gamestate.cfg");

    int* DEF_DELTAY = &cfg.DEF_DELTAY;
    int* DEF_DELTAX = &cfg.DEF_DELTAX;

    texture empty_texture = Renderer::generate_empty_texture(cfg.SKY_WIDTH, cfg.SKY_HEIGHT);
    texture_fill(empty_texture, cfg.SKY_COLOR);
    level_set_skybox(&level, empty_texture);

    texture cake_texture = default_texture_search("smol_player");
    texture_replace_color(cake_texture, RED, GREEN);

    texture island_texture = default_texture_search("01_island");
    texture gift_texture = default_texture_search("02_gift");
    texture real_cake_texture = default_texture_search("01_cake_txtr");
    texture table_texture = default_texture_search("03_table");
    level.load_chunk({ { {0, 0}, {99, 99} }, {} });

    /// initially load state
    reset();

    std::vector<Unit> renderscale;

    ev.add_hook({ INIT }, [&](Level* level, EventInfo* info) {
        std::cout << "Initialized the Game Event Hook (external hook received this event)\n";

        renderscale = get_renderscale(*level);

        level->display.camera.clear_screen();

        ev.call_event(FIXED_UPDATE, *info);
        level->display.set_cursor_visibility(false);

        //write_level_data("envs/proto_level.glvl", *level);
    });

    ev.add_hook( { EXIT }, [&](Level* level, EventInfo*) {
        write_level_data("envs/proto_level.glvl", *level);
    } );

    ev.add_hook( { FIXED_UPDATE }, [&](Level* level, EventInfo* info) {
        if (info->EVENT == MOUSE_MOVE) return;
        renderscale = get_renderscale(*level);
        std::string lvl_view = get_lvl_view(*level, renderscale);
        emplace_lvl_camera(*level, lvl_view);
        level->display.camera.reset_cur();
        auto time = GET_TIME();
        std::cout << level->display.camera.draw() << "\n" << Gmeng::resetcolor;
        level->display.camera.draw_time = GET_TIME() - time;
        if (level->display.camera.modifiers.get_value("draw_info") == 1) level->display.camera.draw_info(vp_width(level->display.viewpoint)+2, 0);
    });

    ev.add_hook({ KEYPRESS }, [&](Level* level, EventInfo* info) {
        if (info->KEYPRESS_CODE == 'q') {
            std::cout << "quiting the game event loop.\n";
            ev.cancelled = true; // cancel the game event loop
            info->prevent_default = true; // disable other default hooks
        };

        if (info->KEYPRESS_CODE == 'y') {
            Mogus mogus_char;
            mogus_char.position = { 30, 25 };
            mogus_char.sprite = vd_find_texture(vgm_defaults::vg_textures, "smol_player");

            level->entities.push_back( std::move( std::make_unique<Mogus>( mogus_char ) ) );
        };

        if (info->KEYPRESS_CODE == 'u') {
            Player player;
            player.position = { 0, 0 };
            player.sprite = vd_find_texture(vgm_defaults::vg_textures, "mini_allah");

            level->entities.push_back( std::move( std::make_unique<Player>(player) ) );
        };

        auto cake_model = level->entities.at(0).get();

        auto real_cake_model = level->chunks.at(0).models.at(2);
        bool nomove = info->prevent_default;

        switch (info->KEYPRESS_CODE) {
            case 'a': case 'A':
                if (cake_model->position.x-((int)nomove) > 0) {
                    cake_model->position.x--;
                    if (cake_model->position.x-((int)nomove)-cake_model->sprite.width <= level->display.viewpoint.start.x) {
                        level->display.viewpoint.start.x--;
                        level->display.viewpoint.end.x--;
                    };
                    if (nomove) cake_model->position.x++;
                    if (info->KEYPRESS_CODE == 'A') cake_model->position.x--;
                };
                break;
            case 'd': case 'D':
                if (cake_model->position.x+((int)nomove)+cake_model->sprite.width < level->base.width-1) {
                    cake_model->position.x++;
                    if (cake_model->position.x+((int)nomove)+((cake_model->sprite.width)*2) > level->display.viewpoint.end.x) {
                        level->display.viewpoint.start.x++;
                        level->display.viewpoint.end.x++;
                    };
                    if (nomove) cake_model->position.x--;
                    if (info->KEYPRESS_CODE == 'D') cake_model->position.x++;
                };
                break;
            case 'w': case 'W':
                if (cake_model->position.y-((int)nomove) > 0) {
                    cake_model->position.y--;
                    if (cake_model->position.y-((int)nomove) <= level->display.viewpoint.start.y) {
                        level->display.viewpoint.start.y--;
                        level->display.viewpoint.end.y--;
                    };
                    if (nomove) cake_model->position.y++;
                    if (info->KEYPRESS_CODE == 'W') cake_model->position.y--;
                };
                break;
            case 's': case 'S':
                if (cake_model->position.y+((int)nomove)+cake_model->sprite.height < level->base.height-1) {
                    cake_model->position.y++;
                    if (cake_model->position.y+((int)nomove)+((cake_model->sprite.height)) >= level->display.viewpoint.end.y) {
                        level->display.viewpoint.start.y++;
                        level->display.viewpoint.end.y++;
                    };
                    if (nomove) cake_model->position.y--;
                    if (info->KEYPRESS_CODE == 'S') cake_model->position.y++;
                };
                break;
            case 'r': case 'R':
                level->display.camera.clear_screen();
                if (!info->prevent_default) { // scope
                    auto tim_ca = GET_TIME();
                    reset();
                    tim_ca = GET_TIME() - tim_ca;
                    level->display.camera.set_curXY(3,*DEF_DELTAX+2);
                    WRITE_PARSED("RESET performed in "$(tim_ca)"ms");
                };
                level->display.camera.draw_info(*DEF_DELTAX+2, 0);
                break;
            case 'e': case 'E':
                if (calculate_proximity(cake_model->position, real_cake_model.position) <= 3) {
                    color_t prev_color = BLUE;
                    for (int e = 0; e < cfg.A00_CAKE_INTERACT_LOOPC; e++) {
                        color_t color = (color_t)(e%8);
                        auto tim = GET_TIME();
                        texture_replace_color(level->base.lvl_template, prev_color, color);
                        prev_color = color;
                        ev.call_event(FIXED_UPDATE, Gmeng::NO_EVENT_INFO);
                        std::this_thread::sleep_for(std::chrono::milliseconds( std::max(cfg.model_positions["CAKE_INTERACT_TIMES"].x, cfg.model_positions["CAKE_INTERACT_TIMES"].y - (int)(GET_TIME()-tim)) ));
                    };
                    texture_replace_color(level->base.lvl_template, prev_color, BLUE);
                };
                break;
            default:
                break;
        };

        if (level->display.viewpoint.start.x < 0) level->display.viewpoint.start.x = 0, level->display.viewpoint.end.x = *DEF_DELTAX;
        if (level->display.viewpoint.start.y < 0) level->display.viewpoint.start.y = 0, level->display.viewpoint.end.y = *DEF_DELTAY;
        if (level->display.viewpoint.end.x > level->base.width) level->display.viewpoint.end.x = level->base.width, level->display.viewpoint.start.x = level->base.width-*DEF_DELTAX;
        if (level->display.viewpoint.end.y > level->base.height) level->display.viewpoint.end.y = level->base.height, level->display.viewpoint.start.y = level->base.height-*DEF_DELTAY;

        level->chunks.at(0).models.at(2) = real_cake_model;
    });

    ev.add_hook({ MOUSE_CLICK_LEFT_START, MOUSE_CLICK_MIDDLE_START, MOUSE_CLICK_RIGHT_START },
            [&](Level* level, EventInfo* info) {
                string button_name = info->EVENT == MOUSE_CLICK_LEFT_START ? "left" :
                                    ( info->EVENT == MOUSE_CLICK_RIGHT_START ? "right" : "middle" );
                //std::cout << "MOUSE CLICK: " << colors[GREEN] << button_name << " click at " << v_str(info->MOUSE_X_POS) << "," << v_str(info->MOUSE_Y_POS) << "\n" << colors[WHITE];
        if (info->EVENT == MOUSE_CLICK_LEFT_START) {
        if (!viewpoint_includes_dp(
                    level->chunks.at(0).vp, {
                        (int)level->chunks.at(0).models.at(0).width  + info->MOUSE_X_POS,
                        (int)level->chunks.at(0).models.at(0).height + info->MOUSE_Y_POS
                    }
            )) return;
            level->entities.at(0)->position = { info->MOUSE_X_POS, info->MOUSE_Y_POS*2 };
        };
    });

    ev.add_hook({ MOUSE_SCROLL_UP, MOUSE_SCROLL_DOWN },
    [&](Level* lv, EventInfo* info) {
        // asumes user goes down
        EventInfo info_d = EventInfo { KEYPRESS, '{', -1, -1, true };


        switch ((int)info->alternative) { // shift key is pressed
            case 0: { // not alternative
                if (info->EVENT == MOUSE_SCROLL_UP) {
                    lv->display.viewpoint.start.y--;
                    lv->display.viewpoint.end.y--;
                } else if (info->EVENT == MOUSE_SCROLL_DOWN) {
                    lv->display.viewpoint.start.y++;
                    lv->display.viewpoint.end.y++;
                };
            };
            break;
            case 1: { // alternative
                if (info->EVENT == MOUSE_SCROLL_UP) {
                    lv->display.viewpoint.start.x--;
                    lv->display.viewpoint.end.x--;
                } else if (info->EVENT == MOUSE_SCROLL_DOWN) {
                    lv->display.viewpoint.start.x++;
                    lv->display.viewpoint.end.x++;
                };
            };
            break;
        }



        ev.call_event(KEYPRESS, info_d);
    });

    do_event_loop(&ev);
    std::cout << "end program\n";
    level.display.set_cursor_visibility(true);
    return 0;
};
