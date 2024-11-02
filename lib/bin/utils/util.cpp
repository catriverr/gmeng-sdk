#pragma once
#include <algorithm>

#include "../gmeng.h"


/// General Utilities for Gmeng
namespace Gmeng::Util {
    /// @since 10.1.0
    /// TODO: does not support cubic rendering, add option
    std::string draw_texture_string(Gmeng::texture txt) {
        Gmeng::Camera<1,1> cam;
        std::string final;
        for (int i = 0; i < txt.width*txt.height; i++) {
            if (i != 0 && i % txt.width == 0) final += '\n';
            if (i >= txt.units.size()) final += cam.draw_unit({
                .color = 0,
                .collidable = true,
                .special = true,
                .special_clr = 4,
                .special_c_unit = "?"
            });
            else final += cam.draw_unit(txt.units.at(i));
        };
        return final;
    };
    /// fills a texture with a single color
    void texture_fill(texture& tx, color_t color) {
        for (int i = 0; i < tx.width*tx.height && i < tx.units.size(); i++) {
            tx.units.at(i).color = color;
        };
    };

    /// replaces the colors of a texture (color1 -> color2)
    void texture_replace_color(texture& tx, color_t color1, color_t color2) {
        for (int i = 0; i < tx.width*tx.height && i < tx.units.size(); i++) {
            auto d = tx.units.at(i);
            if (d.color == color1) d.color = color2;
            tx.units[i] = d;
        };
    };

    int calculate_proximity(Renderer::drawpoint pos1, Renderer::drawpoint pos2) {
        return std::abs(pos1.x - pos2.x) + std::abs(pos1.y - pos2.y);
    };

    void level_set_skybox(Level* lvl, texture& tx) {
        lvl->base.height = tx.height;
        lvl->base.width = tx.width;

        lvl->base.lvl_template = tx;
    };

    Renderer::Model model_create(int width, int height, Renderer::drawpoint pos) {
        Renderer::Model model = Renderer::generate_empty_model(width, height);
        model.position = pos;
        model.name = v_str(g_mkid());
        model.id = g_mkid();
        return model;
    };

    Renderer::Model model_from_txtr(texture& texture, Renderer::drawpoint pos) {
        return Renderer::Model {
            .width = texture.width,
            .height = texture.height,
            .size = texture.width*texture.height,
            .position = pos,
            .name = v_str(g_mkid()),
            .texture = texture,
            .id = g_mkid(),
        };
    };

    texture default_texture_search(std::string name) {
        return vd_find_texture(vgm_defaults::vg_textures, name);
    };

    Renderer::Model default_model_search(std::string name) {
        return vd_find_model(vgm_defaults::vg_rdmodels, name);
    };

    void viewpoint_go_right(Renderer::viewpoint& vp, int times) {
        vp.start.x += times; vp.end.x += times;
    };
    void viewpoint_go_left(Renderer::viewpoint& vp, int times) {
        vp.start.x -= times; vp.end.x -= times;
    };

    void viewpoint_go_up(Renderer::viewpoint& vp, int times) {
        vp.start.y -= times; vp.end.y -= times;
    };
    void viewpoint_go_down(Renderer::viewpoint& vp, int times) {
        vp.start.y += times; vp.end.y += times;
    };
};
