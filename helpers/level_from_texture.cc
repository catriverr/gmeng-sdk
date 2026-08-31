#include "../lib/bin/easy.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char** argv) {
    Gmeng::global.log_stout = true;
    SAY("gmeng " + Gmeng::version + " level generator from texture utility\n");

    if ( argc < 2 ) {
        SAY("~r~err~n~ provide a parameter: level_from_texture <texture_path>\n");
        return 1;
    };

    auto level = std::make_unique<Gmeng::Level>();

    char* filename = argv[1];

    if ( !fs::exists(filename) || !fs::is_regular_file(filename) ) {
        SAY("~r~err~n~ invalid file\n");
        return 1;
    };

    Gmeng::texture tex;

    try {
        tex = Gmeng::LoadTexture( filename );
    } catch ( std::exception& e ) {
        SAY("~r~err~n~ texture file is invalid/corrupt.\n");
        return 1;
    };

    level->base.height = tex.height;
    level->base.width = tex.width;
    level->base.lvl_template = tex;

    level->name = "level"$(g_mkid())"";
    level->desc = "no_description";

    level->display.viewpoint = { { 0, 0 }, {
        (int)tex.width-1,
        (int)tex.height-1
    } };

    level->load_chunk( Gmeng::chunk {
        {
            { 0, 0 },
            { (int)tex.width-1, (int)tex.height-1 }
        },
        {}
    } );

    std::string level_filename = g_splitStr(filename, ".").at(0) + "_level.glvl";
    write_level_data( level_filename, *level );
    SAY("created level file: " + level_filename + " from " + filename + "\n" );

    return 0;
};
