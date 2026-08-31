#include "../easy.h"
#include "../gmeng.h"
#include "../utils/util.cpp"
#include "../utils/serialization.cpp"
#include <csignal>
#include <fstream>

int main() {
    Gmeng::Level lvl;
    texture g = Gmeng::Renderer::generate_empty_texture(300, 300);
    Gmeng::Util::texture_replace_color(g,
            color32_from_uint32(Gmeng::PINK),
            color32_from_uint32(Gmeng::BLACK)
    );
    lvl.base.lvl_template = g;
    lvl.base.height = g.height;
    lvl.base.width = g.width;

    lvl.name = "level"$(g_mkid())"";
    lvl.desc = "no_description";

    lvl.display.viewpoint = { {0, 0}, { 170, 40 } };
    lvl.display.width = 170;
    lvl.display.height = 40;

    Gmeng::chunk c;
    c.vp = { { 0, 0 }, { 300, 300 } };
    c.models.push_back( Gmeng::Renderer::generate_empty_model(15, 15) );
    lvl.chunks.push_back( c );

    std::ofstream file("level.glvl");
    if (!file.is_open()) return 1;
    write_level_data( file, lvl );
};
