#pragma once
#include "vector.h"
#include <cstdint>


#define RGB_ASCII_CODE_FG(r, g, b) std::string("\x1b[38;2;"$(r)";"$(g)";"$(b)"m")
#define RGB_ASCII_CODE_BG(r, g, b) std::string("\x1b[48;2;"$(r)";"$(g)";"$(b)"m")
#define BCOLOR32_TO_ASCII( col32_obj ) RGB_ASCII_CODE_BG( col32_obj.r, col32_obj.g, col32_obj.b )
#define FCOLOR32_TO_ASCII( col32_obj ) RGB_ASCII_CODE_FG( col32_obj.r, col32_obj.g, col32_obj.b )

/// sets the current terminal foreground color to the RGB value
#define TERM_RGB_FG(buf, r, g, b) std::snprintf(buf, 32, "\x1b[38;2;%d;%d;%dm", (r), (g), (b))
/// sets the current terminal background color to the RGB value
#define TERM_RGB_BG(buf, r, g, b) std::snprintf(buf, 32, "\x1b[48;2;%d;%d;%dm", (r), (g), (b))
/// resets the terminal color with ansi escape code
#define TERM_RGB_RESET "\x1b[0m"

namespace Gmeng {

    enum color_t {
        WHITE  = 0,
        BLUE   = 1,
        GREEN  = 2,
        CYAN   = 3,
        RED    = 4,
        PINK   = 5,
        YELLOW = 6,
        BLACK  = 7
    };

    /// Default foreground colorcodes
    /// to each Gmeng::color_t index.
	static std::string colors[] = {
		"\x1B[37m", "\x1B[34m", "\x1B[32m", "\x1B[36m", "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m", "\x1B[37m"
	};
    /// Default background colorcodes
    /// to each Gmeng::color_t index.
    static std::string bgcolors[] = {
        "\x1B[47m", "\x1B[44m", "\x1B[42m", "\x1B[46m", "\x1B[41m", "\x1B[45m", "\x1B[43m", "\x1B[40m", "\x1B[47m"
    };
    /// Default BOLD foreground colorcodes
    /// to each Gmeng::color_t index.
    static std::string bgcolors_bright[] = { // match the 'bold' foreground colors (bright)
        "\x1B[107m", "\x1B[104m", "\x1B[102m", "\x1B[106m", "\x1B[101m", "\x1B[105m", "\x1B[103m", "\x1B[40m", "\x1B[107m"
    };
    /// Colorcodes from ANSI escape code to Gmeng::color_t index.
	static std::string colorids[] = { "7", "4", "2", "6", "1", "5", "3", "0" };
    /// Resets all ANSI escape code sequences, effectively setting the color to normal.
	static std::string resetcolor = "\033[22m\033[0m";
    /// ANSI escape code sequence to make text bold.
    /// Must be used before the colorcode, as this will
    /// override it otherwise.
    static std::string boldcolor = "\033[1m";


    /// RGB_UINT Colors
    /// use as rgb_colors[ (color_t) ]
    static int rgb_colors[9][3] = {
        {255,255,255},
        {131, 165, 152},
        {184, 187, 38},
        {134, 180, 117},
        {244, 73, 52},
        {211, 134, 155},
        {249, 188, 47},
        {40, 40, 40},
        {249, 128, 25}
    };

    /// horrible type decl hack
    struct color32_t;
    Gmeng::color32_t color32_from_uint32(uint32_t rgb);

    struct color32_t {
        uint8_t r, g, b;  // Red, Green, Blue channels in 0–255

        // Constructor
        color32_t(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0)
            : r(r), g(g), b(b) {};

        // Convert to Vec3 (float RGB in range [0.0f, 1.0f])
        Vec3 to_vec3() const {
            return Vec3(
                static_cast<float>(r) / 255.0f,
                static_cast<float>(g) / 255.0f,
                static_cast<float>(b) / 255.0f
            );
        }

        /// initialize from legacy color
        color32_t( uint32_t color_value ) {
            if ( color_value <= 7 ) {
                auto rgb_values = Gmeng::rgb_colors[ color_value ];
                *this = { (uint8_t)rgb_values[0], (uint8_t)rgb_values[1], (uint8_t)rgb_values[2] };
            } else {
                *this = color32_from_uint32( color_value );
            };
        };

        bool operator==( const color32_t& other ) const = default;
        operator Vec3() const { return this->to_vec3(); };
    };

    constexpr const uint32_t GMENG_COL_0_UINT32 = 16777215;
    constexpr const uint32_t GMENG_COL_1_UINT32 = 8627608;
    constexpr const uint32_t GMENG_COL_2_UINT32 = 12106534;
    constexpr const uint32_t GMENG_COL_3_UINT32 = 8828021;
    constexpr const uint32_t GMENG_COL_4_UINT32 = 16009524;
    constexpr const uint32_t GMENG_COL_5_UINT32 = 13862555;
    constexpr const uint32_t GMENG_COL_6_UINT32 = 16366639;
    constexpr const uint32_t GMENG_COL_7_UINT32 = 2631720;

    /// automatically converts gmeng's default color palette
    /// values into packed RGB uint32_t values. Non-default
    /// colors are not swapped and returned as is.
    inline constexpr const uint32_t& GMENG_COL32( const uint32_t& col ) {
        switch(col) {
            case 0:
                return GMENG_COL_0_UINT32;
                break;
            case 1:
                return GMENG_COL_1_UINT32;
                break;
            case 2:
                return GMENG_COL_2_UINT32;
                break;
            case 3:
                return GMENG_COL_3_UINT32;
                break;
            case 4:
                return GMENG_COL_4_UINT32;
                break;
            case 5:
                return GMENG_COL_5_UINT32;
                break;
            case 6:
                return GMENG_COL_6_UINT32;
                break;
            case 7:
                return GMENG_COL_7_UINT32;
                break;
            default:
                return col;
        };
    };

    /// converts a color32_t to a uint32_t object
    inline uint32_t uint32_from_color32(const color32_t& color) {

        return (static_cast<uint32_t>(color.r) << 16) |
               (static_cast<uint32_t>(color.g) << 8)  |
               (static_cast<uint32_t>(color.b));
    }

    /// converts a Vec3 or color32_t to color_t (legacy color_8)
    inline color_t conv_rgb_col8(const Vec3& rgb) {
        // Threshold the RGB channels to 0 or 1 based on mid value
        int r = rgb.x > 0.5f ? 1 : 0;
        int g = rgb.y > 0.5f ? 1 : 0;
        int b = rgb.z > 0.5f ? 1 : 0;

        // Encode RGB as 3-bit number
        int encoded = (r << 2) | (g << 1) | b;

        // Invert bits to match color_t enum (optional depending on mapping)
        switch (encoded) {
            case 0b000: return BLACK;   // 0
            case 0b001: return BLUE;    // 1
            case 0b010: return GREEN;   // 2
            case 0b011: return CYAN;    // 3
            case 0b100: return RED;     // 4
            case 0b101: return PINK;    // 5 (red + blue)
            case 0b110: return YELLOW;  // 6 (red + green)
            case 0b111: return WHITE;   // 7
            default:    return BLACK;   // fallback
        }
    }

    /// unit color type. Can be legacy (0-8) colors,
    /// or RGB.
    struct unitcolor_t {

        public:
            // color type for units, 8-color for legacy (default)
            // or rgb value.
            enum unitcoltype {
                COLOR_8, RGB
            };
            /// color option type, COLOR_8 default, RGB for uint32_t
            unitcoltype type = COLOR_8;
            /// value for COLOR_8 legacy.
            /// '-1' if type != COLOR_8.
            int c8_value = -1;
            /// value for RGB.
            /// '0, 0, 0' if type != RGB.
            color32_t rgb_value = { 0, 0, 0 };

            /// Legacy COLOR_8 color type operator.
            int operator=( int value ) {
                this->type = COLOR_8;
                this->rgb_value = { 0, 0, 0 };
                this->c8_value = value;

                return this->c8_value;
            };

            color_t operator=( color_t value ) {
                *this = (int)value;
                return (color_t)this->c8_value;
            };

            color32_t operator=( color32_t value ) {
                this->type = RGB;
                this->c8_value = -1;
                this->rgb_value = value;
                return this->rgb_value;
            };

            operator int() const {
                if ( this->c8_value == COLOR_8 )
                    return this->c8_value;
                else
                    return conv_rgb_col8( this->rgb_value );
            };

            operator color_t() const {
                return (color_t) ((int)this->c8_value);
            };

            operator color32_t() const {
                return this->rgb_value;
            };

            unitcolor_t( int value ) {
                *this = value;
            };

            unitcolor_t( color_t value ) {
                *this = value;
            };

            unitcolor_t( color32_t value ) {
                *this = value;
            };
    };

};
