#pragma once
#include <algorithm>

#include "../gmeng.h"


/// General Utilities for Gmeng
namespace Gmeng::Util {
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
        };
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
