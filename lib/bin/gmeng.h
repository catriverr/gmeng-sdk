#pragma once
#include <cerrno>
#include <codecvt>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <iterator>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include <cstring>
#include <future>
#include <functional>
#include <random>
#include <map>
#include <algorithm>
#include <sstream>
#include <functional>
#include <atomic>
#include <climits>
#include <execinfo.h> // function call tracing
#include <cxxabi.h> // function call tracing



#include "src/objects.cpp"
/// class initialization
#include "utils/setup_class.h"
/// Gmeng::global utility
#include "utils/global.h"
/// Gmeng::Assert utility
#include "utils/assert.h"

/// gm_log() logging utility
#include "utils/log.h"

#include <filesystem>
#if _WIN32 == false
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#endif
#include <unistd.h> // for STDOUT_FILENO
#ifdef _WIN32
    #include <windows.h>
#else
    #include <limits.h>
#endif



using std::vector;
using std::string;



/// sets a timeout and asyncronously calls a callback once the time in milliseconds has passed
void set_timeout(std::function<void()> cb, uint64_t delay_ms) {
    (void)std::async(std::launch::async, [cb, delay_ms]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
        cb();
    });
}

#include "utils/tracefunc.h"

/// Gets the current working directory
static std::string get_cwd() {
    char buffer[PATH_MAX];

#ifdef _WIN32
    if (GetCurrentDirectoryA(PATH_MAX, buffer)) {
        return std::string(buffer);
    }
#else
    if (getcwd(buffer, sizeof(buffer)) != nullptr) {
        return std::string(buffer);
    }
#endif
    return std::string();
}


/// flips a vector vertically (used for mirroring textures & other vectors 2d grids.)
template<typename T> void flip_vector(std::vector<T>& data, int width, int height) {
    if (width <= 0 || height <= 0 || data.size() != static_cast<size_t>(width * height))
        return; // Invalid input

    for (int y = 0; y < height; ++y) {
        int rowStart = y * width;
        int rowEnd = rowStart + width;
        std::reverse(data.begin() + rowStart, data.begin() + rowEnd);
    }
}

/// returns the username of the current user
static std::string get_username() {
    const char* username = nullptr;
#ifdef _WIN32
    username = getenv("USERNAME");
#else
    username = getenv("USER");
#endif
    if (username) return std::string(username);
    else return std::string();
};

#ifdef __GMENG_OBJECTINIT__


#include "utils/functree.h"



/// Creates a Kitty terminal config that is
/// compatible with Gmeng's requirements.
void kitty_create_config(std::string fname) {
    std::ofstream f(fname);

    if (!f.is_open()) std::cerr << "error writing";
    else {
        f << "allow_remote_control yes\n";
        f << "font_size 18.0\n";

        f.close();
        std::cout << "written to " << fname;
    };

};

/// Kitty terminal : resizes the window.
/// Requires an unknown amount of grace milliseconds.
void kitty_resize_window(int width, int height) {
    std::system(("kitten @ resize-os-window --width " + std::to_string(width) + " --height " + std::to_string(height)).c_str());
};

/// Kitty terminal : sets font size.
void kitty_set_font_size(int size) {
    std::system(("kitten @ set-font-size " + std::to_string(size)).c_str());
};



/// Returns the address of any given object.
template<typename T>
static uintptr_t _uget_addr(const T& obj) {
    //__functree_call__(_uget_addr);
    return (reinterpret_cast<uintptr_t>(&obj));
};

/// Gmeng : returns whether a file with the given filename exists.
static bool file_exists(std::string f) {
    __functree_call__(file_exists);
    return std::filesystem::is_directory(f) || std::filesystem::exists(f);
}

/// Gmeng : Repeats a given string `times` number of times.
/// The string is referenced but is not modified,
/// the function returns the generated string.
static std::string repeatString(const std::string& str, int times) {
    //__functree_call__(repeatString);
    std::string result = "";
    for (int i = 0; i < times; i++) {
        result += str;
    }
    return result;
}

/// Gmeng : Repeats any `Thing` of any type `times` number of times.
template<typename Thing>
static std::vector<Thing> repeatThing(Thing obj, int times) {
    std::vector<Thing> Things;
    for (int i = 0; i < times; i++) Things.push_back(obj);
    return Things;
};

/// Gmeng : Splits a vector of any type with a user-provided `checker` function.
/// If the checker function returns true, the vector is split.
/// Returns a vector of a vector of things.
template<typename Thing>
static std::vector<std::vector<Thing>> splitThing(std::vector<Thing> obj, std::function<bool(Thing)> checker) {
    std::vector<std::vector<Thing>> Things;
    int vec_c = 0;
    for (int i = 0; i < obj.size(); i++) {
        Thing lndx = obj[i];
        if (vec_c == Things.size()) Things.push_back(std::vector<Thing>{});
        if (!checker(lndx)) Things[vec_c].push_back(lndx);
        else vec_c++;
    };
    return Things;
};

/// Gmeng : returns the hex value of an integer
static std::string _uconv_1ihx(int value) {
    //__functree_call__(_conv_1ihx);
    std::stringstream stream;
    stream << "0x" << std::hex << value;
    return stream.str();
}


#define cpps(str) ( std::string(str) )
using namespace std;
using MouseClickCallback = std::function<void()>;
static bool endsWith(const std::string& str, const char* suffix, unsigned suffixLen)
{
    return str.size() >= suffixLen && 0 == str.compare(str.size()-suffixLen, suffixLen, suffix, suffixLen);
};

static bool endsWith(const std::string& str, const char* suffix)
{
    return endsWith(str, suffix, std::string::traits_type::length(suffix));
};

static bool startsWith(const std::string& str, const char* prefix, unsigned prefixLen)
{
    return str.size() >= prefixLen && 0 == str.compare(0, prefixLen, prefix, prefixLen);
};

static bool startsWith(const std::string& str, const char* prefix)
{
    return startsWith(str, prefix, std::string::traits_type::length(prefix));
};

/// @deprecated sleeps the current thread for a given delay
/// and calls a callbackfunction after that delay (in milliseconds).
static void g_setTimeout(std::function<void()> callback, int delay) {
    __functree_call__(g_setTimeout);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    callback();
}

/// Gmeng : Reads a file and returns it as a string.
/// lines are split with `\n` or `\r\n` (depending on the line feed)
static std::string g_readFile(const string &fileName)
{
    __functree_call__(g_readFile);
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return std::string(bytes.data(), fileSize);
};

/// Gmeng : Removes a group of characters from a given string.
/// All occurences of the characters in the given char[] array
/// are removed from the string. Since it is a reference, nothing is returned.
static void g_rmChar( string &str, char* charsToRemove ) {
    __functree_call__(g_rmChar);
   for ( unsigned int i = 0; i < strlen(charsToRemove); ++i ) {
      str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end() );
   }
}


namespace Gmeng {
    /// current version of the engine.
    /// "-d" suffix means the version is a developer version, high unstability level
    /// "-b" suffix means the version is a beta version, low unstability level but unpolished
    /// "-c" suffix means the version is a coroded version, low to medium unstability level but specific methods will not perform as expected
    static std::string version = "13.3.0";
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

	enum CONSTANTS {
		/// integer values
        UNITMAP_SIZE = 32767,
		vl_nomdl_id = 0x0FFFF0, vl_notxtr_id = 0x0FFFF1, vl_nochunk_id = 0x0FFFF2,
		// C_PlugEvent is event type of 'plugin event',
        // C_InputEvent is event type of 'keyboard/mouse input'
		// C_LogEvent is event type of 'log' (written to stdout by parent process)
		// TYPES
		// PE_Type0 is Player move event
		// PE_Type1 is Command run event
        // PE_Type2 is modifier change event
        // IE_Type0 is mouse click event
		C_LogEvent = 8534, LE_Type0 = 8535, LE_Type1 = 8536,
		C_PlugEvent = 8544, PE_Type0 = 8545, PE_Type1 = 8546, PE_Type2 = 8547,
        C_InputEvent = 8554, IE_Type0 = 8555, IE_Type1 = 8556, IE_Type2 = 8557,
	};
    /// @deprecated Gmeng 1.1 Stream Events.
    /// In previous versions of gmeng, a typescript
    /// event handler would receive these event objects
    /// and handle them accordingly. As this was painful to maintain
    /// and was obviously inefficient and insensible, it has been
    /// deprecated. This functionality (or type structure) is no longer used.
	struct event {
		std::string name = ""; int id = 0;
		std::vector<std::string> params = {};
	};
    /// For Gmeng 1.1 Framework
    /// DEPRECATED, no replacement required.
    /// Gmeng 4.0 Framework does not require events as it does not rely on Parent Processes.
	class EventHandler {
		public:
		// __etp = event type
		// __est = event script
			inline void cast_ev(int __etp, std::string __est) {
                __functree_call__(Gmeng::EventHandler::cast_ev);
                if (__etp < 8500 || __etp > 8599) throw std::invalid_argument("Gmeng::EventHandler->cast_ev(int __etp <- right here | " + std::to_string(__etp) + " is not a valid event type"); 
                if (__etp == Gmeng::CONSTANTS::C_InputEvent) std::cerr << __est;
				if (__etp == Gmeng::CONSTANTS::C_PlugEvent) std::cerr << __est;
				else if (__etp == Gmeng::CONSTANTS::C_LogEvent) std::cerr << "[gm0:core/__log] " + __est;
				return;
			};
			inline std::string gen_estr(Gmeng::event __e) {
                //__functree_call__(Gmeng::EventHandler::gen_estr);
                if (__e.id == Gmeng::CONSTANTS::IE_Type0) return std::string ("[gm0:core/__MOUSECLICK__]");
				return std::string ("[gm0:core/__EVCAST] " +
				std::to_string(__e.id) +
				" " + __e.name + " " +
				g_joinStr(__e.params, "!:"));
			};
	};
    /// X and Y position Coordinate object.
	struct Coordinate {
		int x = 0; int y = 0;
	};
    /// Entire logs of the game, engine, dependencies.
    static std::stringstream completelog;
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
    // unicode characters for the 'unit' pixel
    // terminal-only - not used with sdl2 or ncurses
    // for ncurses, see wc_unit
    const char c_unit[4] = "\u2588";
    /// Ceiling outer_unit. Half a character in height.
    /// In monospaced fonts, this is a perfect 1x1 square.
	const char c_outer_unit[4] = "\u2584";
    /// Floor outer unit. Half a character in height.
    /// In monospaced fonts, this is a perfect 1x1 square.
	const char c_outer_unit_floor[4] = "\u2580";

    //
    // 2D Vector
    //
    struct Vec2 {
        float x, y;

        // Constructor
        Vec2(float x = 0, float y = 0) : x(x), y(y) {}

        // Vector addition
        Vec2 operator+(const Vec2& other) const {
            return Vec2(x + other.x, y + other.y);
        }

        // Vector subtraction
        Vec2 operator-(const Vec2& other) const {
            return Vec2(x - other.x, y - other.y);
        }

        // Scalar multiplication
        Vec2 operator*(float scalar) const {
            return Vec2(x * scalar, y * scalar);
        }

        // Scalar division
        Vec2 operator/(float scalar) const {
            return Vec2(x / scalar, y / scalar);
        }

        // Dot product
        float dot(const Vec2& other) const {
            return x * other.x + y * other.y;
        }

        // Length (magnitude)
        float length() const {
            return std::sqrt(dot(*this));
        }

        // Normalize to unit vector
        Vec2 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec2();
        }
    };

    //
    // 3D Vector
    //
    struct Vec3 {
        float x, y, z;

        Vec3(float x = 0, float y = 0, float z = 0)
            : x(x), y(y), z(z) {}

        Vec3 operator+(const Vec3& other) const {
            return Vec3(x + other.x, y + other.y, z + other.z);
        }

        Vec3 operator-(const Vec3& other) const {
            return Vec3(x - other.x, y - other.y, z - other.z);
        }

        Vec3 operator*(float scalar) const {
            return Vec3(x * scalar, y * scalar, z * scalar);
        }

        Vec3 operator/(float scalar) const {
            return Vec3(x / scalar, y / scalar, z / scalar);
        }

        // Dot product: returns scalar
        float dot(const Vec3& other) const {
            return x * other.x + y * other.y + z * other.z;
        }

        // Cross product: returns vector perpendicular to both
        Vec3 cross(const Vec3& other) const {
            return Vec3(
                y * other.z - z * other.y,
                z * other.x - x * other.z,
                x * other.y - y * other.x
            );
        }

        float length() const {
            return std::sqrt(dot(*this));
        }

        Vec3 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec3();
        }
    };

    //
    // 4D Vector
    //
    struct Vec4 {
        float x, y, z, w;

        Vec4(float x = 0, float y = 0, float z = 0, float w = 0)
            : x(x), y(y), z(z), w(w) {}

        Vec4 operator+(const Vec4& other) const {
            return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
        }

        Vec4 operator-(const Vec4& other) const {
            return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
        }

        Vec4 operator*(float scalar) const {
            return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
        }

        Vec4 operator/(float scalar) const {
            return Vec4(x / scalar, y / scalar, z / scalar, w / scalar);
        }

        float dot(const Vec4& other) const {
            return x * other.x + y * other.y + z * other.z + w * other.w;
        }

        float length() const {
            return std::sqrt(dot(*this));
        }

        Vec4 normalized() const {
            float len = length();
            return len > 0 ? *this / len : Vec4();
        }
    };

    inline std::ostream& operator<<(std::ostream& os, const Vec2& v) {
        return os << "Vec2(" << v.x << ", " << v.y << ")";
    }

    inline std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        return os << "Vec3(" << v.x << ", " << v.y << ", " << v.z << ")";
    }

    inline std::ostream& operator<<(std::ostream& os, const Vec4& v) {
        return os << "Vec4(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")";
    }

    /// horrible type decl hack
    struct color32_t;
    Gmeng::color32_t color32_from_uint32(uint32_t rgb);

    struct color32_t {
        uint8_t r, g, b;  // Red, Green, Blue channels in 0–255

        // Constructor
        color32_t(uint8_t r = 0, uint8_t g = 0, uint8_t b = 0)
            : r(r), g(g), b(b) {}

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

        operator Vec3() const { return this->to_vec3(); };
    };

    /// converts a color32_t to a uint32_t object
    uint32_t uint32_from_color32(const color32_t& color) {
        return (static_cast<uint32_t>(color.r) << 16) |
               (static_cast<uint32_t>(color.g) << 8)  |
               (static_cast<uint32_t>(color.b));
    }

    /// converts a Vec3 or color32_t to color_t (legacy color_8)
    color_t conv_rgb_col8(const Vec3 rgb) {
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

    /// Unit : Gmeng Implementation for screen piece.
    /// Works as a pixel, a fragment of a texture, model,
    /// entity, or display map.
	struct Unit {
      public:
            /// 24-bit representation of RGB derived
            /// from `gmeng::color32_t`. values 0-7 are
            /// reserved for Gmeng's default palette.
			uint32_t color = 1;
            /// whether the Unit can be collided with.
            /// If set to true, collision is disabled
            /// and entities can collide with this specific
            /// unit within its larger container (Models, Entities.)
            bool collidable = true;
            /// @deprecated gmeng 1.1 - whether the unit is the player entity
            /// kept for backwards compatibility but is unused since gmeng 4.0.
            bool is_player = false;
            /// @deprecated gmeng 1.1 - whether the unit is part of an entity.
            /// kept for backwards compatibility but is unused since gmeng 4.0.
            bool is_entity = false;
            /// @deprecated gmeng 1.1 - G_Player player instance within the unit.
            /// kept for backwards compatibility but is unused since gmeng 4.0.
            Objects::G_Player player={};
            /// whether the Unit is transparent. If set
            /// to true, the unit is treated as nonexistent,
            /// and will not have collision enabled even if
            /// its `collidable` property is set to true.
            bool transparent = false;
            /// whether the unit is special. Special units
            /// will break `cubic_render` functionality and
            /// display a full-height character in the size
            /// of a 2x1 rectangle, but allow for displaying
            /// custom items within 2 units. For esoteric
            /// textures and models, this can come in handy.
            bool special = false;
            /// color of the special unit. If `special` is
            /// enabled, `special_clr` will set its foreground
            /// color (24-bit RGB derived from `gmeng::color32_t`).
            uint32_t special_clr = 0;
            /// @deprecated gmeng 1.1 - G_Entity entity instance within the unit.
            /// kept for backwards compatibility but is unused since gmeng 4.0.
			Objects::G_Entity entity={};
            /// character string of the special unit. If
            /// `special` is set, this item will be displayed
            /// in a 2x1 rectangular full-size character occupying
            /// the next Y coordinate's same X position.
            std::string special_c_unit = "";

	};

    /// Blob : Render blob that contains a vector of units
    template<std::size_t w = 0, std::size_t h = 0>
    struct Blob {
      public:
        /// Width of the render blob.
        std::size_t width = w;
        /// Height of the render blob.
        std::size_t height = h;
        /// Unit vector of the render blob. The vector
        /// is treated as a 2D entity with a size of
        /// `VECTOR( WIDTH * HEIGHT )`, every time
        /// `vector.at(n) % WIDTH` is `0 (divisable)`,
        /// the next `Y++` value within the `HEIGHT`
        /// of the blob is present.
        std::vector<Gmeng::Unit> units;

    };

    /// Texture : Gmeng Implementation for textures.
    /// Works as an image, as the terminal does not support
    /// importing image files to draw, this is a custom implementation.
    struct texture : public Blob<0, 0> {
        /// Collision property. If set to true,
        /// the texture (weirdly having this parameter)
        /// will not allow for phasing (multiple-item collision).
        bool collidable = false;
        /// Name of the texture. Used by the VisualCache
        /// Graphics Manager to sort and quickly access
        /// items within the texture dictionary.
        std::string name;

        GMENG_INIT_TYPE( width, height, units,
                         collidable, name );
    };


    /// Sets the Units of g_tx to the Units of g_tx2, copying
    /// the second texture's image data into the first. This
    /// function assumes that the second texture is either
    /// equal or larger in size to the first texture.
    extern void set_texturemap(Gmeng::texture &g_tx, Gmeng::texture &g_tx2);
    /// Loads a gmeng texture from a file `__fname`.
    /// This function only loads parsable string-based
    /// texture files and will throw an error with
    /// serialized binary-based files introduced in 10.0.0.
    extern Gmeng::texture LoadTexture(std::string __fname);

    /// Modifier : a named in-game variable that can
    /// be modified by the developer console. Used
    /// for cases where in-game changes may be requested
    /// by the player during runtime. Serialized into
    /// binary with `serialize_modifier`.
    struct modifier {
	  public:
        /// Name of the modifier. Accessable in
        /// `Gmeng::ModifierList::modifiers.at( name )`.
		std::string name = "v_unallocated_modifier";
        /// Value of the modifier. Only integer values
        /// are accepted, for multiple reasons, main
        /// one being the fact that most of the time,
        /// modifiers are either enums (glorified integers)
        /// or integer values by nature anyways.
        int value = 0;
	};

    /// ModifierList : Container for modifiers
    /// concerning any structure. Used in multiple
    /// internal classes like `Gmeng::Level` and `Gmeng::Camera`.
	struct ModifierList {
	  public:
        /// List of modifiers within the list.
		std::vector<modifier> values = {};

        /// Returns the value of a modifier with the key
        /// provided. The function will return the first
        /// instance of the value, so if there are multiple
        /// stacked within the list only the first one will
        /// be returned. If no instances of a modifier with
        /// the given key is found, `-1` is returned.
        int get_value(std::string key) {
            for (const auto& val : values)
                if (val.name == key) return val.value;
            return -1;
        };

        /// Sets the vawlue of a modifier with the key
        /// provided. The function will only set the first
        /// instance of the value, so if there are multiple
        /// stacked within the list only the first one will
        /// be modified. If no instances of a modifier with
        /// the given key is found, nothing is modified.
        ///
        /// This function does NOT create a new entry if
        /// a modifier with the key provided exists.
        void set_value(std::string key, int value) {
            for (auto& val : values)
                if (val.name == key) { val.value = value; break; };
        };
	};

    /// @deprecated Gmeng 1.1 Renderer Options.
	struct RendererOptions {
		public:
        /// Whether the system should count
        /// single-unit objects. Useful for
        /// statistics and performance
        bool countObjects = false;
        /// Whether the system should use the
        /// current game title.
        bool useTitle = false;
        /// Sets the game title.
		std::string title = "";
	};

    /// DisplayMap, container for the unitmap within the
    /// camera. Units are not raw, so they can be accessed
    /// by external functions to be modified. Such changes
    /// will be reflected after `get_lvl_view` and before
    /// `emplace_lvl_camera` is called. Modifying them
    /// directly will only reflect for a single frame.
	template<std::size_t d_width, std::size_t d_height>
	class DisplayMap {
		public:
		/// Height of the DisplayMap.
        int __h = d_width;
        /// Width of the DisplayMap.
        int __w = d_height;
        /// Contains the undrawn unitmap within the camera.
        /// Units are not raw, so they can be accessed by
        /// external functions to be modified. Such changes
        /// will be reflected after `get_lvl_view` and before
        /// `emplace_lvl_camera` is called. Modifying them
        /// directly will only reflect for a single frame.
		Gmeng::Unit unitmap[CONSTANTS::UNITMAP_SIZE] = {};
        /// Pool size of the unitmap. Differs from `CONSTANTS::UNITMAP_SIZE`.
        /// This property calculates (at compile-time) the byte size of the
        /// unitmap in accordance to the byte size of one `Gmeng::Unit` instance.
        int pool_size = (sizeof unitmap / sizeof(Gmeng::Unit));
	};
    /// @deprecated gmeng 1.1 renderer. Kept for backwards compatibility. Unused.
    ///
    /// Gmeng 1.1 instances of this class will contain single-unit object counts,
    /// width and height of the display, the displaymap, renderer options and
    /// a method to set the current viewport of the camera to a given 2D array of units.
	template<std::size_t _width, std::size_t _height>
	class G_Renderer {
		public:
        /// @deprecated - gmeng 1.1 Width of the Renderer.
		std::size_t width = _width;
        /// @deprecated - gmeng 1.1 Height of the Renderer.
        std::size_t height = _height;
        /// @deprecated - gmeng 1.1 Total single-unit object count.
		int totalObjects;
        /// @deprecated - gmeng 1.1 Display map. Contains the unitmap
        /// of the renderer, which can be drawn by a Camera instance.
        /// Unused since gmeng 4.1_glvl.
        Gmeng::DisplayMap<_width, _height> display;
        /// @deprecated - gmeng 1.1 Renderer options. Contains modifiers
        /// for `count_objects`, `pool_size` and `renderer_size`. Unused.
        Gmeng::RendererOptions options;
        /// @deprecated - gmeng 1.1 Sets the units of the display map.
        /// Not raw data, only accepts `Gmeng::Unit` objects. Unitmap
        /// must have a size of `this->width * this->height` in Array form.
		inline void setUnits(Unit unitmap[_width * _height]) {
            __functree_call__(Gmeng::G_Renderer::setUnits);
            /// Loop through the width and height, effectively
            /// for the entire size of the 2D landscape within
            /// the renderer and display for the camera.
			for (int i = 0; i < _width * _height; i++) {
                /// Set the unit at the given position to the
                /// unit provided to the function
				this->display.unitmap[i] = unitmap[i];
			};
		};
	};


};

/// Sets a Global Controller at `index` to `value`.
/// Global controllers are currently unused in the engine.
/// They may be used by game instances via accessing `Gmeng::global`.
inline void controller_set(int index, std::string value) {
    __functree_call__(controller_set);
    Gmeng::global.indexes.push_back(index);
    Gmeng::global.containers.push_back(value);
};

/// Switches the state of the global dev_console
/// variable. This will not close/open the console,
/// it will enable or disable the functionality to do so.
inline void switch_dev_console() {
    __functree_call__(switch_dev_console);
    Gmeng::global.dev_console = !Gmeng::global.dev_console;
};


/// Returns the thread ID of the current thread as a hex string.
inline std::string _uthread_id(const std::thread& thread) {
    /// __functree_call__(_uthread_id);
    std::thread::id threadId = thread.get_id();
    size_t hashValue = std::hash<std::thread::id>{}(threadId);
    return std::to_string(hashValue);
};

/// splits a text into characters but includes colorcodes in chars.
std::vector<std::string> split_with_ansi(const std::string& input)
{
    std::vector<std::string> result;
    std::string pending_ansi;

    auto read_ansi = [&](size_t& i) {
        size_t start = i++;
        if (i >= input.size()) return;

        if (input[i] == '[') // CSI
        {
            ++i;
            while (i < input.size() &&
                   (input[i] < '@' || input[i] > '~'))
                ++i;
            if (i < input.size()) ++i;
        }
        else if (input[i] == ']') // OSC
        {
            ++i;
            while (i < input.size() &&
                   !(input[i] == '\x07' ||
                     (input[i] == '\x1B' && i + 1 < input.size() && input[i + 1] == '\\')))
                ++i;
            if (i < input.size())
            {
                if (input[i] == '\x1B') i += 2;
                else ++i;
            }
        }
        else // single-char ESC
        {
            ++i;
        }

        pending_ansi.append(input, start, i - start);
    };

    for (size_t i = 0; i < input.size();)
    {
        if (input[i] == '\x1B')
        {
            read_ansi(i);
        }
        else
        {
            std::string s;
            s.reserve(pending_ansi.size() + 1);
            s += pending_ansi;
            pending_ansi.clear();

            s += input[i++];
            result.push_back(std::move(s));
        }
    }

    // Trailing ANSI → attach to last character
    if (!pending_ansi.empty() && !result.empty())
    {
        result.back() += pending_ansi;
    }

    return result;
}

/// Returns whether the character is
/// the start to an ANSI escape code sequence (`\x1B`)
inline bool is_ansi_start(char c) noexcept
{
    return c == '\x1B';
}

/// Returns an index to a logical index (index in a string EXCLUDING ansi escape codes)
/// as a physical index (index in a string INCLUDING ansi escape codes).
/// If the index cannot be found, `std::string::npos` is returned.
size_t logical_to_physical_index(const std::string& s, size_t logical_index)
{
    size_t logical = 0;
    size_t i = 0;
    const size_t n = s.size();

    while (i < n)
    {
        if (is_ansi_start(s[i]))
        {
            ++i;
            if (i >= n) break;

            // CSI: ESC [
            if (s[i] == '[')
            {
                ++i;
                while (i < n && (s[i] < '@' || s[i] > '~'))
                    ++i;
                if (i < n) ++i;
            }
            // OSC: ESC ]
            else if (s[i] == ']')
            {
                ++i;
                while (i < n &&
                       !(s[i] == '\x07' ||
                        (s[i] == '\x1B' && i + 1 < n && s[i + 1] == '\\')))
                    ++i;
                if (i < n)
                {
                    if (s[i] == '\x1B') i += 2;
                    else ++i;
                }
            }
            // Single-character ESC
            else
            {
                ++i;
            }
        }
        else
        {
            if (logical == logical_index)
                return i;

            ++logical;
            ++i;
        }
    }

    return std::string::npos;
}


/// strips all ANSI escape codes / color codes from a text.
/// Used for getting the actual length of strings without accounting
/// for colorcodes.
std::string strip_ansi(const std::string& input)
{
    std::string out;
    out.reserve(input.size());

    for (size_t i = 0; i < input.size();)
    {
        if (input[i] == '\x1B') // ESC
        {
            ++i;
            if (i >= input.size()) break;

            // CSI: ESC [
            if (input[i] == '[')
            {
                ++i;
                while (i < input.size() &&
                       (input[i] < '@' || input[i] > '~'))
                {
                    ++i;
                }
                if (i < input.size()) ++i;
            }
            // OSC: ESC ]
            else if (input[i] == ']')
            {
                ++i;
                while (i < input.size() &&
                       !(input[i] == '\x07' ||
                        (input[i] == '\x1B' && i + 1 < input.size() && input[i + 1] == '\\')))
                {
                    ++i;
                }
                if (i < input.size())
                {
                    if (input[i] == '\x1B') i += 2;
                    else ++i;
                }
            }
            // Other ESC sequences
            else
            {
                ++i;
            }
        }
        else
        {
            out += input[i++];
        }
    }

    return out;
}

#define v_intl int
#define v_static_cast static_cast
#define v_sizel std::size_t
#define v_title std::string
#define v_constl const
#define v_endl std::endl
#define v_nl "\n"
#define v_rcol Gmeng::resetcolor

/// converts a uint32_t integer containing
/// an RGB value (NOT! RGBA) to a color32_t value.
Gmeng::color32_t Gmeng::color32_from_uint32(uint32_t rgb) {
    return {
        static_cast<uint8_t>((rgb >> 16) & 0xFF), // Red   (bits 16-23)
        static_cast<uint8_t>((rgb >> 8) & 0xFF),  // Green (bits 8-15)
        static_cast<uint8_t>(rgb & 0xFF)          // Blue  (bits 0-7)
    };
}

/// @deprecated use ModifierList::get_value
/// Returns the index of a modifier with `targetName` within a vector.
inline int g_find_modifier(const std::vector<Gmeng::modifier>& modifiers, const std::string& targetName) {
    __functree_call__(g_find_modifier);
    for (size_t i = 0; i < modifiers.size(); ++i) { if (modifiers[i].name == targetName) { return static_cast<int>(i); }; };
    return -1;
}

inline std::vector<std::string> _ulogc_gen1dvfc(int ln = 7400) {
    __functree_call__(_ulogc_gen1dvfc);
    std::vector<std::string> vector;
    for (int i = 0; i < ln; i++) {
        vector.push_back("\x0b");
    };
    return vector;
};




/// @deprecated @nosource
/// Logs with newline to the console.
static void gm_nlog(std::string msg) {
    #ifndef __GMENG_ALLOW_LOG__
        return;
    #endif
    #if __GMENG_ALLOW_LOG__ == true
        /// std::cerr << msg;
        /// DEPRECATED & DISABLED
    #endif
};

#define g_file __FILE__
#define g_delim ":"
#define g_line __LINE__

/// Returns the filename from a file path.
#define GET_FILENAME(x) (std::string(x).substr(std::string(__FILE__).rfind("/") + 1)).c_str()
/// current filename without the file path.
#define FILENAME GET_FILENAME(__FILE__)

/// Returns the current file and line.
#define __gmeng_attribute__() ({ \
    std::ostringstream oss; \
    oss << FILENAME << ":" << __LINE__; \
    oss.str(); \
})

/// Replaces all occurences of a substring within
/// a given string to a given substring.
static std::string str_replace(const std::string& str, const std::string& from, const std::string& to) {
    // __functree_call__(str_replace);
    std::string result = str; std::size_t start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return result;
};

/// foregroundcolor text identifiers.
/// TODO: convert to `char` array.
static std::vector<std::string> colornames = {
    "w", "b", "g", "c", "r", "p", "y", "0", "_", "o", "1"
    // white blue green cyan red pink yellow black
};

/// backgroundcolor text identifiers.
static std::vector<std::string> bgcolornames = {
    "Bw", "Bb", "Bg", "Bc", "Br", "Bp", "By", "B0", "Bw", "B1"
    // white blue green cyan red pink yellow black
};

/// Formats a text with colorcodes. Converts `~<colorname>~` indexes
/// into ANSI escape code sequences.
/// Example:
/// ```
/// std::string f_string = "~g~green text ~n~normal text ~b~blue text ...";
/// std::string formatted = colorformat(f_string);
/// ```
static std::string colorformat(std::string data) {
    std::string formatted = data;
    for (int i = 0; i < (sizeof(Gmeng::colors)/sizeof(*Gmeng::colors)); i++) {
        formatted = str_replace(formatted, "~" + colornames[i] + "~", Gmeng::colors[i]);
    };
    for (int i = 0; i < (sizeof(Gmeng::bgcolors)/sizeof(*Gmeng::bgcolors)); i++) {
        formatted = str_replace(formatted, "~" + bgcolornames[i] + "~", Gmeng::bgcolors[i]);
    };
    formatted = str_replace(formatted, "~h~", "\033[1m");
    formatted = str_replace(formatted, "~n~", "\033[0m");
    formatted = str_replace(formatted, "~i~", "\033[3m");
    formatted = str_replace(formatted, "~u~", "\033[4m");
    formatted = str_replace(formatted, "~st~", "\033[9m");
    return formatted + "\033[0m";
};

/// Writes to the standard output with color formatting.
/// See `colorformat()` to see ANSI escape code sequencing.
#define SAY std::cout << colorformat
/// Writes to the standard output with information formatting.
/// See `colorformat()` to see ANSI escape code sequencing.
#define INF std::cout << "\033[1m" + Gmeng::colors[Gmeng::BLUE] + "(i) >> \033[0m" + Gmeng::colors[Gmeng::WHITE] + colorformat


#include "strings/replace_all.cc"

/// Parses string variables (VERSION and BUILD).
/// str vars are accessed with the `$!__VARIABLE_NAME` syntax.
static std::string parse_str_vars(std::string a) {
    std::string res = a;
    replace_all(res, "$!__VERSION", Gmeng::version);
    replace_all(res, "$!__BUILD", GMENG_BUILD_NO);
    return res;
};

/// Writes a parsed string to the standard output.
/// Parsed strings include string variables. See `colorformat()` for
/// ANSI escape sequencing and highlighting functionality.
#define WRITE_PARSED(x) std::cout << colorformat(parse_str_vars(x))

#include <iomanip>

/// Returns the last line of a stringstream.
static std::string get_last_line(std::stringstream& ss) {
    std::string s = ss.str();
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
        s.pop_back();

    auto pos = s.find_last_of('\n');
    std::string line = (pos == std::string::npos) ? s : s.substr(pos + 1);

    auto p = line.find(')');
    return (p == std::string::npos) ? line : line.substr(p + 1);
}



/// Gmeng : logs to the logger with an unknown source.
static void dgm_log(const char* file, int line, std::string _msg, bool use_endl = true) {
    if (Gmeng::global.shush) return;
    _gm_log(file, line, "UNKNOWN_SOURCE", _msg, use_endl);
};

/// Gmeng : logs to the logger with an unknown source.
static void dgm_log(std::string _msg, bool use_endl = true) {
    if (Gmeng::global.shush) return;
    _gm_log(":",0,"UNKNOWN_SOURCE",_msg,use_endl);
};



/// Gmeng : logs to the logger with an unknown source with a title.
static void gm_slog(Gmeng::color_t color, std::string title, std::string text) {
    if (Gmeng::global.shush) return;
    _gm_log(":", 0, "UNKNOWN_SOURCE", Gmeng::colors[color] + title + " " + Gmeng::colors[Gmeng::WHITE] + text);
};



namespace Gmeng {
    /// i now realise this may not be very secure
    static std::vector<std::thread> v_threads;
    static std::thread create_thread(std::function<void()> func) {
        __functree_call__(Gmeng::create_thread);
        return (Gmeng::v_threads.emplace_back(func)).detach(), std::move(Gmeng::v_threads.back());
    };
    static void clear_threads() {
        __annot__(Gmeng::clear_threads, "clears all used internal threads to prepare the environment for exiting.");
        __functree_call__(Gmeng::clear_threads);
        v_threads.erase(std::remove_if(v_threads.begin(), v_threads.end(), [](const std::thread& t) { return !t.joinable(); }), v_threads.end());
    };
    static void join_threads() {
        __annot__(Gmeng::join_threads, "joins all used internal threads by Gmeng. Ran before a program closes.");
        __functree_call__(Gmeng::join_threads);
        for (auto& thread : Gmeng::v_threads) {
            gm_log("Gmeng::join_threads -> gm:v_thread, create_thread() -> T_MEMADDR: " + _uconv_1ihx(_uget_addr(&thread)) + " - MAIN THREAD ID: " + _uget_thread() + " - T_THREAD_ID: " + _uthread_id(thread));
            try {
                if (thread.joinable()) thread.join();
                clear_threads();
            } catch (std::exception& e) {
                std::cerr << (Gmeng::colors[4] + "join_threads() -> *error :: could not join thread, skipping... (pretty big internal error please report / see https://gmeng.org/bug-report)");
                gm_log(" :::: error cause -> " + std::string(e.what())); };
        };
    };
};

static string boolstr(bool x) {
    if (x == true) return "true";
    else return "false";
};

static void init_logc(int ms = 250) {
    __functree_call__(init_logc);
    #if __GMENG_DISABLE_LOG__ == true
        return;
    #endif
    if (!Gmeng::global.restarted_instance) {
        /// initialize the log
        __gmeng_write_log__("gmeng.log", "-- cleared previous log --\n", false);
        gm_log("Gmeng "+Gmeng::version+" (build " + GMENG_BUILD_NO + ").\n\nGmeng is built and maintained by catriverr.\n\nDocumentation available in https://gmeng.org.\nGmeng is an open source project. https://gmeng.org/git.\nPlease report bugs or unexpected behaviour at https://gmeng.org/report.\n\nGmeng: Go-To Console Game Engine.\n\nSPAWN(1) = v_success / at " + get_curtime() + "/" + get_curdate() + "\ncontroller_t of termui/_udisplay_of(GMENG, window) handed over to: controller_t(gmeng::threads::get(0))\n");
        gm_log("startup information:\n\n----------------------------------\nExecutable Name: " + Gmeng::global.executable + "\nCurrent Working Directory: " + Gmeng::global.pwd + "\nCurrent User: " + Gmeng::global.user + "\n----------------------------------\n" +
                ("Global Variables\n\t- restarted_instance: " + std::string(Gmeng::global.restarted_instance ? "true" : "false" ) + "\n\t- devmode: " + boolstr(Gmeng::global.dev_mode) + "\n\t- debugger: " + boolstr(Gmeng::global.debugger) + "\n\t- silenced: " + boolstr(Gmeng::global.shush) + "\n\t- dont_hold_back: " + boolstr(Gmeng::global.dont_hold_back) + "\n----------------------------------\n"));
    } else {
        /// print restart message
        gm_log("-- RESTARTED GMENG INSTANCE --\n");
    };

    if (!Gmeng::global.shush) Gmeng::create_thread([&]() {
            __functree_call__(_glog_thread_create);
/*        for ( ;; ) {
            if (!Gmeng::global.dev_console) continue;
            if (Gmeng::logstream.str().length() > Gmeng::logc.v_drawpoints.size()) {
               Gmeng::completelog << Gmeng::logstream.str();
               Gmeng::logstream.str(""); /// flush sstream
               _uflush_display(Gmeng::logc, 5);
               gm_log("t_display *job_flush -> flushed display at gm:thread" + _uget_thread() + " (detached from gm:thread0 / generated from gm:thread0) ; display memory address: " + _uconv_1ihx(_uget_addr(&Gmeng::logc)));
            };
            _udraw_display(Gmeng::logc);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }*/
        /// close log file
    });
};


#include "types/termui.h"

namespace Gmeng {
    static t_display logc = {
        .pos = { .x=94, .y=2 },
        .v_cursor = 0,
        .v_outline_color = 1,
        .v_width = 100,
        .v_height = 40,
        .init=true,
        .v_textcolor = 2,
        .v_drawpoints=_ulogc_gen1dvfc(100*40),
        .title="gm:0/logstream"
    };
};




/// Call to catchup with Gmeng's threads
static void _gthread_catchup() {
    __annotation__(_gthread_catchup, "Gmeng::_uthread catchup function, attaches to all threads and clears them.");
    __functree_call__(_gthread_catchup);
    gm_log("_gthread_catchup() -> waiting for " + v_str(Gmeng::v_threads.size()) + " threads to catch-up to thread:" + (_uget_thread()));
    Gmeng::join_threads();
};

/// Generates a hash
static constexpr uint32_t _ghash(const char* data, size_t const size) noexcept {
    uint32_t hash = 5381;
    for(const char *c = data; c < data + size; ++c) hash = ((hash << 5) + hash) + (unsigned char) *c;
    return hash;
}

/// Concatenates two wchar_t* strings
static wchar_t* concat_wstr(const wchar_t* str1, const wchar_t* str2) {
    size_t len1 = wcslen(str1); size_t len2 = wcslen(str2);
    wchar_t* concatenated = new wchar_t[len1 + len2 + 1];
    wcscpy(concatenated, str1); wcscat(concatenated, str2);
    return concatenated;
};

/// Concatenates a wchar_t* string with a std::string
static wchar_t* concat_wstr(const wchar_t* str1, const std::string& str2) {
    // Convert std::string to wide character string
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    std::wstring wstr2 = converter.from_bytes(str2);

    // Allocate memory for the concatenated string
    size_t len1 = wcslen(str1);
    size_t len2 = wstr2.length();
    wchar_t* concatenated = new wchar_t[len1 + len2 + 1];

    // Copy str1 into concatenated
    wcscpy(concatenated, str1);

    // Concatenate wstr2 onto concatenated
    wcscat(concatenated, wstr2.c_str());

    return concatenated;
}

static void repeat(int count, std::function<void()> func) {
    for (int i = 0; i < count; i++) func();
};

static void repeat(int count, std::function<void(int count)> func) {
    for (int i = 0; i < count; i++) func(i);
};

static wchar_t* repeat_wstring(wchar_t* wc, int times) {
    if (times <= 0) {
        return L"";
    };
    wchar_t* str = new wchar_t[times + 1];
    for (int i = 0; i < times; ++i) {
        str[i] = *wc;
    }
    str[times] = L'\0'; // Null-terminate the string

    return str;
};

/// Repeats a wide string `times` amount of times
static wchar_t* repeat_wstring(const wchar_t* wc, int times) {
    if (times <= 0) {
        return L"";
    };
    wchar_t* str = new wchar_t[times + 1];
    for (int i = 0; i < times; ++i) {
        str[i] = *wc;
    }
    str[times] = L'\0'; // Null-terminate the string

    return str;
};

/// converts a wide string to a normal string.
static std::string ws2s(const std::wstring& wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
};


static std::map<std::string, std::function<void()>> gmeng_warnings =
{
    { "bad_terminal", []() {
        /// TMUX slows down input handling
        /// and writing to stdout extremely.
        /// show warning against this.
        /// Do not modify this warning.
        std::cout << Gmeng::colors[Gmeng::YELLOW] << "WARNING!" << Gmeng::resetcolor;
        std::cout << " " << "Gmeng (the engine this game runs on) has identified your terminal program/shell to be incompatible.\n"
                  <<        "While this will not cause crashes, it will almost certainly cause display issues and bad input processing.\n\n";
        std::cout << "\nTMUX is" << Gmeng::boldcolor << " ABSOLUTELY NOT RECOMMENDED " << Gmeng::resetcolor;
        std::cout << "as it slows down input receiver signals, introduces massive input lag,\n";
        std::cout << "is unable to handle multiple keypresses at once";
        std::cout << " and slows down output writing by up to 5 times.\n\n";
        std::cout << "If your terminal depends on tmux for color output,";
        std::cout << " do not run this game with it.\n\n";
        std::cout << "Recommended Terminal Programs:";
        std::cout << "\n- windows: Windows Terminal\n";
        std::cout << "- macOS: Apple Terminal (if using macOS Tahoe or later)\n";
        std::cout << "- macOS: iTerm2 (if using an older version of macOS than Tahoe [26.0])\n";
        std::cout << "- linux / Windows Subsystem for linux: the default tty will suffice.\n";
        std::cout << "\nPress CTRL+C to quit.\n";
        std::cout << "\nPress any key to continue anyway.";
        cin.get();
    } },
    { "windows", []() {
        std::cout << Gmeng::colors[Gmeng::RED] << "ERROR! " << Gmeng::resetcolor << "Gmeng (the engine this game runs on) has identified your OS as WINDOWS." << '\n';
        std::cout << Gmeng::colors[Gmeng::RED] << "ERROR! " << Gmeng::resetcolor << Gmeng::boldcolor << "This error is raised by the engine itself, not the game." << Gmeng::resetcolor << '\n';
        std::cout << Gmeng::colors[Gmeng::RED] << "ERROR! " << Gmeng::resetcolor << "Currently, Gmeng only supports unix based operating systems. If you must use windows, install WSL. https://aka.ms/WSL" << '\n';
        std::cout << '\n';
        std::cout << Gmeng::colors[Gmeng::BLUE] << "ONGOING EFFORTS: PORTING GMENG TO WINDOWS :: " << Gmeng::colors[Gmeng::CYAN] << Gmeng::boldcolor << "https://github.com/catriverr/gmeng-sdk https://gmeng.org\n";
        std::cout << Gmeng::resetcolor << "Contribute to the project.\n";
    } },

    { "port", []() {
        std::cout << Gmeng::colors[Gmeng::RED] << "FAIL!" << Gmeng::resetcolor;
        std::cout << " "  << "Gmeng (the engine this game runs on) has identified an external error in this game instance.\n"
                  << "\n" << Gmeng::boldcolor << "This game (or the engine) has attempted to run the NOBLE script for client-side server instance handling." << Gmeng::resetcolor
                  << " "  << "However, the system could not bind to a port in the 7388-7488 range.\n"
                  << "\n" << Gmeng::boldcolor << "The port range of 7388 to 7488 must have at least one open port for GMENG to create a server instance.\n" << Gmeng::resetcolor
                  << " "  << "Please empty one of these ports to run a server.\n"
                  << "\n" << "Press any key to exit.\n";
        cin.get();
    } },
    { "init_from_main", []() {
        std::cout << Gmeng::colors[Gmeng::YELLOW] + "DEVELOPER ERROR!" << Gmeng::resetcolor;
        std::cout << " "  << "Gmeng (the engine this game runs on) has identified a developer error in this game instance.\n"
                  << "\n" << "this error is non-fatal, so it can be ignored, but various functionality may cause issues."
                  << "\n" << "Please reach out to the developers of this game and notify them about gmeng:warning:init_from_main.\n"
                  << "\n" << "note to developers of this game:\n"
                  << "\n" << "when initializing gmeng, run gmeng function 'patch_argv_global' within your c main() function."
                  << "\n" << "If gmeng is not initialized properly, you will break various engine functionality.\n";

        std::cout << "\nPress CTRL+C to exit or any key to continue.\n";
        cin.get();
    } },
    { "no_truecolor", []() {
        std::cout << Gmeng::colors[Gmeng::YELLOW] + "WARNING! " << Gmeng::resetcolor;
        std::cout << "Gmeng (the engine this game runs on) has identified that you have a non-truecolor shell.\n";
        std::cout << "\nUsual culprit is TMUX. Apple Terminal, iTerm and other ttys usually support truecolor by default, but TMUX overrides it.\n";
        std::cout << "\nIf you wish to use tmux with truecolor (RGB support) see " << Gmeng::colors[Gmeng::BLUE] << "gmeng.org/?doc=truecolor" << Gmeng::resetcolor << ".\n";

        std::cout << "\nPress CTRL+C to exit or any key to continue.\n";
        cin.get();
    } }
};

struct TerminalSize {
    int width; int height;

    bool operator==( const TerminalSize& other ) const = default;

};

/// LMAO cross-platform as if this engine will run on windows any time soon
TerminalSize get_terminal_size() {
    TerminalSize size{0, 0};

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        size.width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        size.height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0) {
        size.width = w.ws_col;
        size.height = w.ws_row;
    }
#endif

    return size;
}


// internal warning method.
// displays warnings related to functions of the engine.
// do not use this method for your game. implement your own
// warning screen mechanisms.
static void _gmeng_show_warning(std::string warning_, char* filename, int fileline, TRACEFUNC) {
    printf("\033c"); // clear screen
    auto term_size = get_terminal_size();
    /// print warning tabtitle
    std::string warn_tab_text = "gmeng_show_warning(" + v_str(fileline) + ") " + _uget_thread() + ".gmeng:" + warning_ + "(0) :: " + filename + ":" + v_str( fileline ) + " @ " + std::string( func_caller );
    while( warn_tab_text.size() < term_size.width ) warn_tab_text += " "; // fill tab width
    std::cout << Gmeng::bgcolors_bright[Gmeng::WHITE] + Gmeng::colors[Gmeng::BLACK] + warn_tab_text << '\n' << Gmeng::resetcolor;

    if (gmeng_warnings.count(warning_) != 0) {
        gmeng_warnings.find(warning_)->second();
    } else {
        /// default warning

        std::cout << Gmeng::colors[Gmeng::RED];
        std::cout << "GMENG WARNING!" << Gmeng::resetcolor;
        std::cout << " <gmeng-default-warning-page>\n";
        std::cout << "[gmeng is the engine that powers this game - this warning is produced by the engine itself]";

        std::cout << "\n\nan internal subsystem of gmeng (" << get_filename(filename) << ':' << fileline << " & " << TRACEFUNC_STR <<  ")\n";
        std::cout << "has invoked a warning for '" << warning_ << "',\n";

        std::cout << "\npress CTRL+C to quit or any other key to continue anyway.\n";

        std::cin.get();
    };
};

// internal warning method. displays warnings related to functions of the engine.
// do not use this method for your game. implement your own warning screen mechanisms.
#define gmeng_show_warning(x) _gmeng_show_warning( x, __FILE__, __LINE__ )


static void gm_err(v_intl type, v_title err_title, TRACEFUNC) {
    __functree_call__(gm_err);
    _gm_log(func_caller_file, func_caller_line, func_caller, Gmeng::colors[Gmeng::RED] + "ERR! " + Gmeng::resetcolor + err_title);
    switch (type) {
        case 0: // v_gm_err case 0: continue running program
            gmeng_show_warning( err_title );
            break;
        case 1: // v_gm_err case 1: exception (stop execution)
            throw std::invalid_argument(Gmeng::colors[4] + "gm:0 *error >> " + err_title + v_nl + v_rcol);
            break;
    };
    return;
};

static void set_terminal_size(int width, int height) {
    // The ANSI sequence to resize the window is \033[8;{height};{width}t
    std::cout << "\033[8;" << height << ";" << width << "t" << std::flush;
}

void resize_terminal(int columns, int rows, bool force = false) {
    if (!Gmeng::global.window_control) return;
    char* term_prog = getenv("TERM_PROGRAM");
    char* term = getenv("TERM");
    /// MacOS Terminal
    if ( term_prog != nullptr && ( std::string(term_prog) == "Apple_Terminal") ) {
        // osascript to command the Terminal application directly.
        std::string script =
            "osascript "
            "-e 'tell application \"Terminal\"' "
            "-e 'set number of columns of front window to " + std::to_string(columns) + "' "
            "-e 'set number of rows of front window to " + std::to_string(rows) + "' "
            "-e 'end tell' > /dev/null 2>&1";

        std::system(script.c_str());
    }
    /// Kitty
    else if ( term != nullptr && std::string(term) == "xterm-kitty" ) {
        if (force) kitty_resize_window(columns,rows);
    };
}

static void set_terminal_title(const std::string& title) {
    // \033]0; starts the title sequence
    // \007 (BEL character) ends the sequence
    std::cout << "\033]0;" << title << "\007" << std::flush;
};

///// __controller_satisfy__
///// OS Error for Windows
static void print_windows_error_message() {
    __functree_call__(print_windows_error_message);
    gmeng_show_warning("windows");
    exit(1);
};

/// sends \033[2J\003[H (ansi clear screen)
/// to the terminal clearing the stdout.
static void ansi_clear_screen() {
    std::cout << "\033[2J\033[H"; // ANSI clear screen
    std::cout.flush();
}

namespace fs = std::filesystem;

#ifdef __APPLE__
#include <mach-o/dyld.h>

/// (Gmeng) installs a font to the user's fonts directory
/// if the font is not already installed.
bool g_install_font(const std::string& fontFilePath) {
    // get user's home directory (~)
    const char* homeDir = std::getenv("HOME");
    if (!homeDir) {
        std::cerr << "Error: Could not determine user home directory.\n";
        return false;
    }

    // validate the source file exists
    fs::path sourcePath(fontFilePath);
    if (!fs::exists(sourcePath)) {
        std::cerr << "Error: Font file not found at " << fontFilePath << "\n";
        return false;
    }

    // copy to fonts directory path (~/Library/Fonts/FontName.ttf)
    fs::path destDir = fs::path(homeDir) / "Library" / "Fonts";
    fs::path destPath = destDir / sourcePath.filename();

    // check if the font is already installed
    if (fs::exists(destPath)) {
        // compare file sizes to ensure it's not corrupted
        // it's basic but should at least work (sometimes idk)
        if (fs::file_size(sourcePath) == fs::file_size(destPath)) {
            return true; // Already installed, silently proceed
        }
    }

    // copy the font into the user's Fonts folder
    try {
        fs::copy_file(sourcePath, destPath, fs::copy_options::overwrite_existing);
        std::cout << "Successfully installed font: " << sourcePath.filename().string() << "\n";
        return true;
    } catch (const fs::filesystem_error& e) {
        gm_err(1, ("Filesystem error installing font: " + std::string(e.what()) + "\n"), __FUNCTION__, __FILE__, __LINE__);
        return false;
    }
}

/// Returns the directory in which the current
/// executable is in. Absolute directory only,
/// calls realpath().
std::string get_executable_directory() {
    char rawPath[PATH_MAX];
    uint32_t size = sizeof(rawPath);

    if (_NSGetExecutablePath(rawPath, &size) != 0) {
        // In the extremely rare case the path exceeds PATH_MAX, _NSGetExecutablePath
        // updates 'size' to the required size (as defined in man 3 dyld), handling that dynamically here.
        std::string dynamicPath(size, '\0');
        if (_NSGetExecutablePath(&dynamicPath[0], &size) != 0) {
            return ""; // Failed to get path
        }
        // Copy back to rawPath for realpath processing
        std::strncpy(rawPath, dynamicPath.c_str(), PATH_MAX);
    }

    // Use realpath() to resolve absolute path, removing './', '../', and symlinks
    char resolvedPath[PATH_MAX];
    if (realpath(rawPath, resolvedPath) == nullptr) {
        return ""; // Failed to resolve path
    }

    // extract the parent directory
    std::filesystem::path fullPath(resolvedPath);
    return fullPath.parent_path().string();
}


/// Forces the macos terminal to use the correct
/// terminal profile (provided by gmeng) when running
/// the program.
///
/// Return values:
///    [success] 0 -> the profile is already set up, proceed
///    [error] 1 -> the current executable path too long
///    [error] 2 -> fail while copying terminal profile to /tmp
///    [success] 3 -> terminal profile launched
static int enforce_macos_terminal_profile(const std::string& templateProfilePath, bool already_set_up) {
    // If the engine already detected the flag, we are good to go.
    if (already_set_up) {
        return 0;
    }

    // Get the absolute path of this running executable
    char pathBuf[1024];
    uint32_t size = sizeof(pathBuf);
    if (_NSGetExecutablePath(pathBuf, &size) != 0) {
        std::cerr << "Error: Engine executable path is too long.\n";
        return 1;
    }
    char pathreal[1024];
    /// set to absolute file path ( so no symlinks or directory options like /../ )
    realpath(pathBuf, pathreal);

    std::string execPath = pathreal;

    std::string uniqueId = v_str(g_mkid());

    // temporary file locations
    std::string tempFile = "/tmp/Gmeng_RequiredProfile_" + uniqueId + ".terminal";
    std::string tempScript = "/tmp/Gmeng_Launcher_" + uniqueId + ".sh";

    std::ofstream scriptFile(tempScript);
    if (!scriptFile) {
        std::cerr << "Error: Could not create temporary launcher script.\n";
        return 2;
    }
    scriptFile << "#!/bin/bash\n";
    scriptFile << "cd \"" << get_executable_directory() << "\"\n";
    scriptFile << "exec \"" << execPath << "\" --profile\n";
    scriptFile.close();

    std::string chmodCmd = "chmod +x \"" + tempScript + "\"";
    std::system(chmodCmd.c_str());


    // copy the base template to the temporary location
    std::string copyCmd = "cp \"" + templateProfilePath + "\" \"" + tempFile + "\"";
    if (std::system(copyCmd.c_str()) != 0) {
        std::cerr << "Error: Failed to copy terminal profile template.\n";
        return 2;
    }

    std::string launchCmd = "cd \\\"" + get_executable_directory() + "\\\" && \\\"" + execPath + "\\\" --profile";

    // Inject the command into the temporary .terminal file using PlistBuddy
    std::string delCmdStr = "/usr/libexec/PlistBuddy -c \"Delete :CommandString\" \"" + tempFile + "\" 2>/dev/null";
    std::string addCmdStr = "/usr/libexec/PlistBuddy -c \"Add :CommandString string '" + tempScript + "'\" \"" + tempFile + "\"";

    std::string delRunShell = "/usr/libexec/PlistBuddy -c \"Delete :RunCommandAsShell\" \"" + tempFile + "\" 2>/dev/null";
    std::string addRunShell = "/usr/libexec/PlistBuddy -c \"Add :RunCommandAsShell bool true\" \"" + tempFile + "\"";

    // shellExitAction 2 = Don't close window
    std::string delExitAction = "/usr/libexec/PlistBuddy -c \"Delete :shellExitAction\" \"" + tempFile + "\" 2>/dev/null";
    std::string addExitAction = "/usr/libexec/PlistBuddy -c \"Add :shellExitAction integer 2\" \"" + tempFile + "\"";

    std::string delScroll = "/usr/libexec/PlistBuddy -c \"Delete :ScrollbackLines\" \"" + tempFile + "\" 2>/dev/null";
    std::string addScroll = "/usr/libexec/PlistBuddy -c \"Add :ScrollbackLines integer 0\" \"" + tempFile + "\"";

    std::system(delScroll.c_str());
    std::system(addScroll.c_str());

    std::system(delCmdStr.c_str());
    std::system(addCmdStr.c_str());

    std::system(delRunShell.c_str());
    std::system(addRunShell.c_str());

    std::system(delExitAction.c_str());
    std::system(addExitAction.c_str());

    // Launch program
    std::string openCmd = "open \"" + tempFile + "\"";
    std::system(openCmd.c_str());
    // Tell the caller to exit this generic terminal instance
    return 3;
}



/// Gmeng's internal setup for macOS terminals.
/// This installs the truetype font Hacker Nerd Font Mono,
/// Sets up the required Apple Terminal Profile for games
/// and ensures `make setup was successful`
bool gmeng_macos_terminal_setup(int argc, char* argv[], TRACEFUNC) {
    __annot__(patch_argv_global, "sets up Gmeng for MacOS & Apple Terminal");
    __functree_call__(gmeng_macos_terminal_setup);

    INF("you will be prompted to access the terminal's controls.\n");
    INF("Allow this setting so Gmeng can receive input from your keyboard and mouse.\n");
    INF("~i~you can change this later in System Settings > Privacy & Security > Automation > Terminal.app~n~\n");

    std::string exec_path = get_executable_directory();
    if (exec_path.empty()) {
        gm_err(1, "an error occured while beginning setup: gmeng.h/internal/macos/func/get_executable_directory() returned an empty string.",
                __FUNCTION__, __FILE__, __LINE__);
        return false;
    };
    if (!fs::exists(exec_path + "/.gmeng_setup")) {
        /// Gmeng Setup
        ansi_clear_screen();
        std::cout << Gmeng::bgcolors[Gmeng::YELLOW] << Gmeng::colors[Gmeng::RED] << "Gmeng " << Gmeng::version
                  << Gmeng::resetcolor << " > Go-To Console Game Engine.\n";
        std::cout << "\nGmeng (the game engine architecture that this game runs on) needs to initialize"
                  << "\n its fonts and terminal profile before this game is playable.\n";

        std::cout << "\nThe setup will install the following prequisites:"
                  << "\n\t- Hack Nerd Font Mono (the preffered font for displaying Gmeng games),"
                  << "\n\t- Default Gmeng Apple Terminal Profile (used to launch all Gmeng games),"
                  << "\n\t- Morhetz/Gruvbox colorprofile and theme (default palette for Gmeng)\n";

        std::cout << "\nDuring the setup, the engine will need to access your font directory."
                  << "\nThis process will only be ran once and will not be required when launching"
                  << " this game in the future.\n";

        std::cout << "\nFor more information, visit " << Gmeng::colors[Gmeng::BLUE] << "https://gmeng.org/?doc=macos-setup" << Gmeng::resetcolor << ".";
        std::cout << Gmeng::boldcolor << "\nPress CTRL+C to cancel the installation or any other key to proceed.\n";

        std::cin.get();
        ansi_clear_screen();

        std::cout << Gmeng::bgcolors[Gmeng::YELLOW] << Gmeng::colors[Gmeng::RED] << "Gmeng " << Gmeng::version
                  << Gmeng::resetcolor << " > Go-To Console Game Engine.\n";
        std::cout <<"\nBeginning installation...";
        if (!fs::exists(exec_path + "/assets/gmeng.terminal")) {
            gm_err(1, "install directory for assets/gmeng.terminal is missing. Abort", __FUNCTION__, __FILE__, __LINE__);
            return false;
        };
        std::cout << "Morhetz/Gruvbox colorprofile (found in <assets/gmeng.terminal>)\n";
        std::cout << "Default Gmeng Apple Terminal Profile (found in <assets/gmeng.terminal>)\n";
        std::cout << "\nInstalling fonts... (found in <assets/HackNerdFont-*.ttf)\n";

        std::cout << "\t> Installing HackNerdFontMono-Regular.ttf...\n";

        if (!fs::exists(exec_path + "/assets/HackNerdFontMono-Regular.ttf")) {
            gm_err(1, "install directory for assets/HackNerdFontMono-Regular.ttf is missing. Abort", __FUNCTION__, __FILE__, __LINE__);
            return false;
        };
        auto v = g_install_font(exec_path + "/assets/HackNerdFontMono-Regular.ttf");
        if (!v) gm_err(0, "unable to install /assets/HackNerdFontMono-Regular.ttf to the system.", __FUNCTION__, __FILE__, __LINE__);

        std::cout << "\t> Installing HackNerdFontMono-Bold.ttf...\n";

        if (!fs::exists(exec_path + "/assets/HackNerdFontMono-Bold.ttf")) {
            gm_err(1, "install directory for assets/HackNerdFontMono-Bold.ttf is missing. Abort", __FUNCTION__, __FILE__, __LINE__);
            return false;
        };

        auto v2 = g_install_font(exec_path + "/assets/HackNerdFontMono-Bold.ttf");
        if (!v2) gm_err(0, "unable to install /assets/HackNerdFontMono-Bold.ttf to the system.", __FUNCTION__, __FILE__, __LINE__);

        std::cout << "\t> Installing HackNerdFontMono-Italic.ttf...\n";

        if (!fs::exists(exec_path + "/assets/HackNerdFontMono-Italic.ttf")) {
            gm_err(1, "install directory for assets/HackNerdFontMono-Italic.ttf is missing. Abort", __FUNCTION__, __FILE__, __LINE__);
            return false;
        };
        auto v3 = g_install_font(exec_path + "/assets/HackNerdFontMono-Italic.ttf");
        if (!v3) gm_err(0, "unable to install /assets/HackNerdFontMono-Italic.ttf to the system.", __FUNCTION__, __FILE__, __LINE__);

        std::cout << "\t> Installing HackNerdFontMono-BoldItalic.ttf...\n";

        if (!fs::exists(exec_path + "/assets/HackNerdFontMono-BoldItalic.ttf")) {
            gm_err(1, "install directory for assets/HackNerdFontMono-BoldItalic.ttf is missing. Abort", __FUNCTION__, __FILE__, __LINE__);
            return false;
        };
        auto v4 = g_install_font(exec_path + "/assets/HackNerdFontMono-BoldItalic.ttf");
        if (!v4) gm_err(0, "unable to install /assets/HackNerdFontMono-BoldItalic.ttf to the system.", __FUNCTION__, __FILE__, __LINE__);

        std::cout << "grace period 3s to ensure the Apple Font daemon registers the newly added fonts...\n";
        std::system("sleep 3");

        std::cout << "declaring that the setup is complete...\n";
        std::system(std::string("touch " + exec_path + "/.gmeng_setup").c_str());
        std::cout << "checking if setup has been successfully declared complete...\n";
        if (!fs::exists(exec_path + "/.gmeng_setup")) {
            gm_err(1, "unable to declare setup complete. Check if the program has the required read/write permissions for the current directory as well as the ~/Library/Fonts directory.",
                    __FUNCTION__, __FILE__, __LINE__);
            return false;
        }
        std::cout << "setup complete. proceeding to the game...\n";
    };
    return true;
};

#endif // __APPLE__


/// Patches gmeng's 'global' variables required by the engine,
/// used for stuff like parsing command-line arguments.
/// Generally, this method should be run for almost
/// every program depending on gmeng.
///
/// While edge cases are existent, if you do not extensively know
/// what this method does, it is advised to run it in your
/// `main()` function.
static void patch_argv_global(int argc, char* argv[], TRACEFUNC) {
    __annot__(patch_argv_global, "patches the Gmeng::global variable with the command-line arguments.");
    __functree_call__(patch_argv_global);

    #if _WIN32
        print_windows_error_message();
        return;
    #endif

        if ( std::string( func_caller ) != "main" ) {
            gmeng_show_warning( "init_from_main" );
        };

        auto* colorterm = std::getenv("COLORTERM") ? std::getenv("COLORTERM") : "";

        if ( std::string(colorterm) != "truecolor" ) {
            gmeng_show_warning("no_truecolor");
        };

    Gmeng::global.pwd = get_cwd();
    Gmeng::global.user = get_username();

#if _WIN32 == false
    Gmeng::global.prog_argc = argc;
    Gmeng::global.prog_argv = argv;

    Gmeng::global.executable = (std::string(argv[0]).substr(std::string(argv[0]).rfind("/") + 1)).c_str();
    Gmeng::global.raw_executable_name = argv[0];

    char *restart_flag = "--GMENG_INTERNAL_RESTARTED_INSTANCE";

    std::vector<char *> v_argv;
    for (int i = 0; i < argc; i++) {
        v_argv.push_back(argv[i]);
        Gmeng::global.raw_arguments += argv[i] + std::string(" ");
    };

    int has_restart_flag = std::count(v_argv.begin(), v_argv.end(), restart_flag);
    bool restarted_instance = false;
    if (has_restart_flag > 0) restarted_instance = true;

    Gmeng::global.restarted_instance = restarted_instance;

    if (!restarted_instance) Gmeng::outfile.open("gmeng.log");
    else Gmeng::outfile.open("gmeng.log", std::ios::app);

    bool IS_USING_GMENG_APPLETERMINAL_THEMED = false;

    for (int i = 0; i < argc; i++) {
        char *v_arg = argv[i];
        std::string argument (v_arg);

        /// For the game to launch using the correct apple terminal profile.
        if (argument == "--profile") { IS_USING_GMENG_APPLETERMINAL_THEMED = true; };
        if (argument == "--no-window-control") { Gmeng::global.window_control = false; };

        if ( argument == "-help" || argument == "/help" || argument == "--help" || argument == "/?" || argument == "-?" ) {
            __functree_call__(__gmeng__help__menu__);
            struct winsize size;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
            int times = size.ws_col-11;

            gm_log("command-line argument requested help menu");
            SAY("~Br~\x0F~h~\x0F~y~GMENG " + (Gmeng::version) + "~n~ | " + Gmeng::colors[6] + "Terminal-Based 2D Game Engine~n~ | Help Menu\n");
            SAY("~_~~st~" + repeatString("-", times+11) + "~n~\n");
            SAY("~h~~r~Gmeng~n~ is a standalone terminal-based game engine, utilizing ~y~pthread~n~ and the ~b~C++ Standard library~n~.\n");
            SAY("~_~Currently, Gmeng is only available to MacOS and Linux users while on its active development phase.\n");
            SAY("~_~You may contribute to porting ~r~Gmeng~_~ to windows: ~b~https://github.com/catriverr/gmeng-sdk\n");
            SAY("~b~~i~PARAMETERS ~st~" + repeatString("-", times) + "~n~\n");
            SAY("\t    ~y~gmeng ~p~-devc \t\t~_~enables developer console\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-no-devc\t\t~_~disables developer console\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-devmode\t\t~_~enables developer diagnostics\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~--profile\t\t~_~asumes terminal is correct\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-log-to-cout\t\t~_~streams logs to stout\t\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-no-functree\t\t~_~disables the gmeng functree\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-debugger -debug\t~_~enables extensive debug logs\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-shut-the-fuck-up\t~_~silences all logging\t\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-functree-extensive\t~_~makes functree logs extensive\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-tell-me-everything\t~_~enables all logging methods\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("~b~" + repeatString(" ", 11) + "~st~" + repeatString("-", times) + "~n~\n");
            exit(0);
        };
        if ( argument == "-devc" ) Gmeng::global.dev_console = true;
        if ( argument == "-no-devc" || argument == "-shut-the-fuck-up" ) Gmeng::global.dev_console = false;
        if ( argument == "-shut-the-fuck-up" ) Gmeng::global.shush = true;
        if ( argument == "-debugger" || argument == "-debug" || argument == "--debugger" ) Gmeng::global.debugger = true;
        if ( argument == "-log-to-cout" || argument == "-lc" ) Gmeng::global.log_stout = true;
        if ( argument == "-devmode" ) Gmeng::global.dev_mode = true;
        if ( argument == "-tell-me-everything" ) Gmeng::global.shush = false, Gmeng::global.dev_mode = true, Gmeng::global.dev_console = true, Gmeng::global.dont_hold_back = true, Gmeng::global.debugger = true;
        if ( argument == "-no-functree" ) { Gmeng::functree_enabled = false; SAY("~b~\x0F~y~WARN! ~_~it is not recommended to disable the Gmeng Functree.\n"); };
        if ( argument == "-functree-extensive" ) { Gmeng::functree_extensive = true; };
        if ( argument == "-weird" ) Gmeng::global.weird_ass = true;
        if ( argument == "-ignore-assert" ) Gmeng::global.ignore_assert = true;
    };

#ifdef __APPLE__
    if ( std::string( getenv("TERM") ) != "xterm-kitty" ) {
        /// Launch Gmeng MacOS setup (setup will exit immediately if the game directory was already set-up)
        bool setup = gmeng_macos_terminal_setup(argc, argv);
        if (!setup) {
            INF("[apple_setup] error (0) nontrue return value from gmeng_macos_terminal_setup. cannot proceed.\n");
            exit(1);
        }

        /// Force MacOS to use Apple Terminal

        std::string exc_path = get_executable_directory();
        int force_profile = enforce_macos_terminal_profile(exc_path + "/assets/gmeng.terminal", IS_USING_GMENG_APPLETERMINAL_THEMED);
        /// return value (3) means the game has launched with the required gmeng profile.
        if (force_profile == 3) exit(0);
        /// return value (0) means the game is already using the required gmeng profile.
        /// otherwise, error.
        if (force_profile != 0) {
            INF("[apple_setup] error (" + v_str(force_profile) +") nonzero return value from enforce_macos_terminal_profile. cannot proceed.\n");
            exit(1);
        };
    } else if (!IS_USING_GMENG_APPLETERMINAL_THEMED) {
        std::string exc_path = get_executable_directory();
        std::system(("kitty --config \"" + exc_path + "/assets/gmeng_kitty.conf\" " + argv[0] + " --profile").c_str());
        exit(0);
    };
#endif
    set_terminal_title("GMENG " + Gmeng::version + " | build " + GMENG_BUILD_NO);
    init_logc();
    gm_log("hello");
#endif
};




/// Restarts the current executable/program.
/// Requires `patch_argv_global()` to have been called.
/// This method runs `execvp` from the `unistd.h` library
/// to replace the current program image in the memory
/// with a new instance of the current program, in `argv[0]`.
static int restart_program() {
    __annot__(restart_program, "Restarts the current executable.");
    __functree_call__(restart_program);

    std::string arg0 = Gmeng::global.raw_executable_name;

    if (arg0.empty()) {
        gm_log("cannot restart program: `Gmeng::global.raw_executable_name` is empty. `patch_argv_global()` must be called with the correct arguments before this function can be used");
        return 1;
    };

    // add `restarted-instance` flag to argv if it does not exist
    std::string restart_flag = "--GMENG_INTERNAL_RESTARTED_INSTANCE";
    bool flag_exists = false;

    for (int i = 0; i < Gmeng::global.prog_argc; ++i) {
        if (std::strcmp(Gmeng::global.prog_argv[i], restart_flag.c_str()) == 0) {
            flag_exists = true;
            break;
        }
    };


    std::vector<std::string> new_argv;

    if (!flag_exists) new_argv.push_back(restart_flag);

    for (int i = 0; i < Gmeng::global.prog_argc; ++i) {
        new_argv.push_back(Gmeng::global.prog_argv[i]);
    };

    std::vector<char*> prog_argv;
    for ( auto v : new_argv )
        prog_argv.push_back( const_cast<char*>( v.c_str() ) );


    gm_log("restarting program...");

    if (execvp(arg0.c_str(), prog_argv.data()) == -1) {
        gm_log("cannot restart program: `execvp()` returned non-zero value (-1): " + std::to_string(errno));
        return 2;
    };

    return 0;
};

#define __GMENG_INIT__ true /// initialized first because the source files check this value before initialization
#include "src/textures.cpp"
#include "src/gmeng.cpp"
#include "src/renderer.cpp"
#include "types/map.h"
#if _WIN32 == false
#include "utils/network.cpp"
#ifndef GMENG_NO_CURSES
    /// for github build workflow
    #include "types/interface.h"
    #include "utils/interface.cpp"
#endif
#if GMENG_SDL
    #include "types/window.h"
    #include "utils/window.cpp"
#endif
#endif
#include "utils/util.cpp"
#include "src/audio.cpp"

namespace g = Gmeng;
namespace gm = Gmeng;
namespace gmeng = Gmeng;

static g::LinearRenderBufferPositionController lrbpc;
static gm::HorizontalRenderBufferPositionController hrbpc;
static gmeng::t_charpos a;

#endif
