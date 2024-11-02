#include <cwchar>
#include <iostream>
#include "../lib/bin/gmeng.h"

using namespace Gmeng;
using namespace Gmeng::Util;

static Level level;

int main(int argc, char** argv) {
    _uread_into_vgm("envs/models");
    patch_argv_global(argc, argv); /// gmeng initialization
    EventLoop ev( {} );

    int DEF_DELTAX = 50;
    int DEF_DELTAY = 25;


    texture empty_texture = Renderer::generate_empty_texture(100, 100);
    texture_fill(empty_texture, BLUE);
    level_set_skybox(&level, empty_texture);
    texture cake_texture = default_texture_search("smol_player");
    texture island_texture = default_texture_search("01_island");
    texture gift_texture = default_texture_search("02_gift");
    texture real_cake_texture = default_texture_search("01_cake_txtr");
    texture table_texture = default_texture_search("03_table");
    level.load_chunk({ { {0, 0}, {99, 99} },
        {
            model_from_txtr( cake_texture, { 0,0 } ),
            model_from_txtr(island_texture, {10, 10}),
            model_from_txtr(gift_texture, { 0, DEF_DELTAY-(int)gift_texture.height+1 })
        }
    });

    level.display.viewpoint = { {0,0}, { DEF_DELTAX, DEF_DELTAY } };
    level.display.set_resolution(DEF_DELTAX, DEF_DELTAY);
    ev.level = &level;
    std::vector<Unit> renderscale;

    ev.add_hook({ INIT }, [&](Level* level, EventInfo* info) {
        std::cout << "Initialized the Game Event Hook (external hook received this event)\n";
        renderscale = get_renderscale(*level);
        level->display.camera.clear_screen();
    });

    ev.add_hook( { FIXED_UPDATE },
    [&](Level* level, EventInfo* info) {
        if (info->EVENT == MOUSE_MOVE) return;
        std::string lvl_view = get_lvl_view(*level, renderscale);
        emplace_lvl_camera(*level, lvl_view);
        level->display.camera.reset_cur();
        auto time = GET_TIME();
        std::cout << level->display.camera.draw() << "\n" << Gmeng::resetcolor;
        level->display.camera.draw_time = GET_TIME() - time;
        level->display.camera.draw_info(vp_width(level->display.viewpoint)+2, 0);
    });

    ev.add_hook({ KEYPRESS }, [&](Level* level, EventInfo* info) {
        if (info->KEYPRESS_CODE == 'q') {
            std::cout << "quiting the game event loop.\n";
            ev.cancelled = true; // cancel the game event loop
            info->prevent_default = true; // disable other default hooks
        };
        auto cake_model = level->chunks.at(0).models.at(0);
        bool nomove = info->prevent_default;
        switch (info->KEYPRESS_CODE) {
            case 'a': case 'A':
                if (cake_model.position.x-((int)nomove) > 0) {
                    cake_model.position.x--;
                    if (cake_model.position.x-((int)nomove)-cake_model.width <= level->display.viewpoint.start.x) {
                        level->display.viewpoint.start.x--;
                        level->display.viewpoint.end.x--;
                    };
                    if (nomove) cake_model.position.x++;
                };
                break;
            case 'd': case 'D':
                if (cake_model.position.x+((int)nomove)+cake_model.width < level->base.width-1) {
                    cake_model.position.x++;
                    if (cake_model.position.x+((int)nomove)+((cake_model.width)*2) > level->display.viewpoint.end.x) {
                        level->display.viewpoint.start.x++;
                        level->display.viewpoint.end.x++;
                    };
                    if (nomove) cake_model.position.x--;
                };
                break;
            case 'w': case 'W':
                if (cake_model.position.y-((int)nomove) > 0) {
                    cake_model.position.y--;
                    if (cake_model.position.y-((int)nomove)-cake_model.height <= level->display.viewpoint.start.y) {
                        level->display.viewpoint.start.y--;
                        level->display.viewpoint.end.y--;
                    };
                    if (nomove) cake_model.position.y++;
                };
                break;
            case 's': case 'S':
                if (cake_model.position.y+((int)nomove)+cake_model.height < level->base.height-1) {
                    cake_model.position.y++;
                    if (cake_model.position.y+((int)nomove)+((cake_model.height)*2) >= level->display.viewpoint.end.y) {
                        level->display.viewpoint.start.y++;
                        level->display.viewpoint.end.y++;
                    };
                    if (nomove) cake_model.position.y--;
                };
                break;
            default:
                break;
        };

        if (level->display.viewpoint.start.x < 0) level->display.viewpoint.start.x = 0, level->display.viewpoint.end.x = DEF_DELTAX;
        if (level->display.viewpoint.start.y < 0) level->display.viewpoint.start.y = 0, level->display.viewpoint.end.y = DEF_DELTAY;
        if (level->display.viewpoint.end.x > level->base.width) level->display.viewpoint.end.x = level->base.width, level->display.viewpoint.start.x = level->base.width-DEF_DELTAX;
        if (level->display.viewpoint.end.y > level->base.height) level->display.viewpoint.end.y = level->base.height, level->display.viewpoint.start.y = level->base.height-DEF_DELTAY;

        level->chunks.at(0).models.at(0) = cake_model;
        renderscale = get_renderscale(*level);
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
            level->chunks.at(0).models.at(0).position = { info->MOUSE_X_POS, info->MOUSE_Y_POS*2 };
            renderscale = get_renderscale(*level);
        };
    });

    ev.add_hook({ MOUSE_SCROLL_UP, MOUSE_SCROLL_DOWN },
    [&](Level* lv, EventInfo* info) {
        // asumes user goes down
        EventInfo info_d = EventInfo { KEYPRESS, '{', -1, -1, true };
        if (info->EVENT == MOUSE_SCROLL_UP) {
            lv->display.viewpoint.start.y--;
            lv->display.viewpoint.end.y--;
        } else if (info->EVENT == MOUSE_SCROLL_DOWN) {
            lv->display.viewpoint.start.y++;
            lv->display.viewpoint.end.y++;
        };
        ev.call_event(KEYPRESS, info_d);
    });

    do_event_loop(&ev);
    std::cout << "end program\n";
    return 0;
};
