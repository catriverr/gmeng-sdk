#include "../lib/bin/easy.h"
#include "../lib/bin/utils/text.h"

int main(int argc, char** argv) {
    Gmeng::global.log_stout = true;
    (void)gmeng_terminal_font_init();

    int abb = 0;
    for ( auto v : Gmeng::CHARACTER_FONT_TEXTURES ) {
        auto tlk = texture_lookup( v.first );
        if ( Objects::coord { (int)v.second.width, (int)v.second.height } != tlk )
            std::cout << "char " << v.first << " is wrong in lookup map\n\titsval:" + v_str( tlk.x ) + "," + v_str( tlk.y ) + "\n\ttrueval: " + v_str( v.second.width ) + "," + v_str( v.second.height ) + '\n';
        else std::cout << abb++ <<  " | char " << v.first << " is correct\n";
    };

    for ( auto a : CHAR_TEXTURE_SIZES ) {
        std::cout << a.key << " check: " << Gmeng::CHARACTER_FONT_TEXTURES.contains( a.key ) << "\n";
    };

    std::string text = "longer, text line\nsecond line";
    auto size = gmeng_terminal_font_calculate_text_size(text);
    std::cout << "text size: " + v_str( size.x ) + "," + v_str( size.y ) << "\n";
    return 0;
};
