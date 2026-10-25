/// Tiny5 font: text for the terminal-based renderer
/// font characters are at minimum 1x5 and maximum
/// 5x6 width/height. See assets/term-font/<CHAR>.gt
#pragma once

#include "../strings/contains.cc"
#include "../types/texture.h"
#include "../src/objects.cpp"
#include "../src/textures.cpp"

#include "log.h"

#include <array>
#include <filesystem>
#include <cctype>
#include <ios>

namespace Gmeng {
    static std::unordered_map<char, Gmeng::Blob> CHARACTER_FONT_TEXTURES;
};


struct LookupEntry {
    char key;
    Objects::coord value;
};

inline constexpr std::array<LookupEntry, 69> CHAR_TEXTURE_SIZES{{
    {' ', { 1,1 } },
    {'!', { 1,6 } },
    {'"', { 3,6 } },
    {'#', { 5,6 } },
    {'$', { 3,6 } },
    {'%', { 4,6 } },
    {'&', { 4,6 } },
    {'\'',{ 1,6 } },
    {'(', { 2,6 } },
    {')', { 2,6 } },
    {'*', { 3,6 } },
    {'+', { 3,6 } },
    {',', { 3,6 } },
    {'-', { 3,6 } },
    {'.', { 3,6 } },
    {'/', { 2,6 } },

    {'0', { 3,5 } },
    {'1', { 2,5 } },
    {'2', { 3,5 } },
    {'3', { 3,5 } },
    {'4', { 3,5 } },
    {'5', { 3,5 } },
    {'6', { 3,5 } },
    {'7', { 3,5 } },
    {'8', { 3,5 } },
    {'9', { 3,5 } },

    {':', { 2,6 } },
    {';', { 2,6 } },
    {'<', { 2,6 } },
    {'=', { 3,6 } },
    {'>', { 2,6 } },
    {'?', { 3,6 } },
    {'@', { 4,6 } },

    {'A', { 4,5 } },
    {'B', { 4,5 } },
    {'C', { 4,5 } },
    {'D', { 4,5 } },
    {'E', { 4,5 } },
    {'F', { 4,5 } },
    {'G', { 4,5 } },
    {'H', { 4,5 } },
    {'I', { 3,5 } },
    {'J', { 3,5 } },
    {'K', { 4,5 } },
    {'L', { 4,5 } },
    {'M', { 5,5 } },
    {'N', { 4,5 } },
    {'O', { 4,5 } },
    {'P', { 4,5 } },
    {'Q', { 4,5 } },
    {'R', { 4,5 } },
    {'S', { 4,5 } },
    {'T', { 3,5 } },
    {'U', { 4,5 } },
    {'V', { 3,5 } },
    {'W', { 5,5 } },
    {'X', { 3,5 } },
    {'Y', { 3,5 } },
    {'Z', { 3,5 } },

    {'[', { 2,6 } },
    {'\\',{ 2,6 } },
    {']', { 2,6 } },
    {'^', { 3,6 } },
    {'_', { 3,6 } },
    {'`', { 2,6 } },
    {'{', { 3,6 } },
    {'|', { 3,6 } },
    {'}', { 3,6 } },
    {'~', { 2,6 } },
}};

constexpr Objects::coord texture_lookup(const char c) {
    auto it = std::lower_bound(CHAR_TEXTURE_SIZES.begin(), CHAR_TEXTURE_SIZES.end(), c,
        [](const LookupEntry& entry, char key) {
            return entry.key < key;
        });

    if (it != CHAR_TEXTURE_SIZES.end() && it->key == c) {
        return it->value;
    }
    return { -1, -1 };
};

static Gmeng::Blob& gmeng_terminal_font_get_char(char chr) {
    auto& tx = Gmeng::CHARACTER_FONT_TEXTURES.at( toupper(chr) );
    return tx;
};

static int gmeng_terminal_font_init( char* path = "assets/term-font" ) {
    __functree_call__( gmeng_terminal_font_init );
    if ( !std::filesystem::exists(path ) ||
         !std::filesystem::is_directory( path ) ) return 1;

    gm_log("initializing...");

    for ( const auto& entry : std::filesystem::directory_iterator( path ) ) {
        if ( !entry.is_regular_file() ) continue; /// must be a file
        if ( !contains( entry.path().string(), ".gt" ) ) continue; /// must be *.gt
        if ( entry.path().filename().string().length() != 4 &&
             !contains( entry.path().string(), "PUNC" ) ) continue; /// must be X.gt or *PUNC.gt

        gm_log("loading font character: " + entry.path().filename().string());
        char char_id = entry.path().filename().string().at(0);
        if ( contains( entry.path().string(), "SLASHPUNC.gt") ) char_id = '/';
        Gmeng::CHARACTER_FONT_TEXTURES[ char_id ] = Gmeng::LoadTexture( entry.path().string() );
    };

    gm_log("total characters: "$(Gmeng::CHARACTER_FONT_TEXTURES.size())". (expecting minimum 69)");
    return 0;
};


/// Calculates the width and height of a given string. Accepts newline
/// characters as well. Punctuation is auto-handled. All text is treated uppercase.
static const Objects::coord gmeng_terminal_font_calculate_text_size(std::string_view text, float scale = 1.0f) {
    Objects::coord size = { 0, 0 };

    int cur_line_width = 0;
    for ( const auto& chr : text ) {
        if ( chr == '\n' ) {
            if ( cur_line_width > size.x ) size.x = cur_line_width;
            size.y *= 2;
            cur_line_width = 0;
            continue;
        };

        const Objects::coord chrsize = texture_lookup( toupper(chr) );

        cur_line_width += chrsize.x + 1;
        if ( chrsize.y > size.y ) size.y = chrsize.y;
    };

    if ( cur_line_width > size.x ) size.x = cur_line_width;

    size.x *= scale;
    size.y *= scale;

    return size;
};
