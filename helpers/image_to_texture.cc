#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

#include "../lib/bin/gmeng.h"
#include "../lib/bin/utils/log.h"

int main(int argc, char** argv) {
    Gmeng::global.log_stout = true;
    SAY("gmeng " + Gmeng::version + " image to texture conversion utility\n");

    if (argc < 2) {
        SAY("~r~err~n~ provide an argument: `" + get_filename(argv[0]) + " <...paths>`\n");
        return 1;
    };

    for ( int i = 1; i < argc; i++ ) {
        char* fname = argv[i];

        if ( !fs::exists( fname ) || !fs::is_regular_file( fname ) ) {
            SAY("~r~err~n~ file " + (std::string)(fname) + " does not exist or is invalid, skipping\n");
            continue;
        };

        std::cout << "image_to_texture: attempting to convert " << fname << "..." << '\n';

        int width, height, channels;

        unsigned char* rgb_data = stbi_load( fname, &width, &height, &channels, 4 );
        if ( rgb_data == nullptr ) {
            SAY("~r~err~n~ stbi_load(" + (std::string)fname + ") failed, file is probably incompatible\n");
            continue;
        } else {
            std::cout << "image " << fname <<  " loaded successfully, width: " << width << ", height: " << height << "\n";
        };

        Gmeng::texture texture;
        texture.width = width;
        texture.height = height;
        texture.name = g_splitStr(fname, ".").at(0);

        texture.units.resize( width * height );

        for ( int y = 0; y < height; ++y ) {
            for ( int x = 0; x < width; ++x ) {
                int pixel_index = (y * width + x) * 4;

                unsigned char r = rgb_data[pixel_index];
                unsigned char g = rgb_data[pixel_index + 1];
                unsigned char b = rgb_data[pixel_index + 2];
                unsigned char a = rgb_data[pixel_index + 3];

                if ( a < 255 ) {
                    texture.units.at( y * width + x ) = { .transparent = true };
                } else {
                    Gmeng::color32_t col(r, g, b);
                    texture.units.at( y * width + x ) = { uint32_from_color32( col ) };
                };

            };
        };

        std::string real_file_name = g_splitStr(fname, ".").at(0) + ".gt";
        std::cout << "texture created from " << fname << ", saving...\n";
        if (Gmeng::save_texture( real_file_name, texture )) {
            std::cout << "file saved to " << real_file_name << '\n';
        } else {
            SAY("~r~err~n~ could not save file: " + real_file_name + ", maybe write permission errors?\n");
        };
    };

    return 0;
};
