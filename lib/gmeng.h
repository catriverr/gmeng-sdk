/// -----------------------------------
/// Gmeng: Console Game Engine.
/// (c) catriverr, all rights reserved.
/// -----------------------------------
///      GMENG DECLARATION HEADER
///
/// this header does not contain code,
/// only declarations of structures and
/// functions within the Gmeng engine.

#include <deque>
#include <set>
#include <termios.h>

#ifndef GMENG_DECLARATION_ONLY

#define GMENG_DECLARATION_ONLY 1

#include "bin/types/vector.h"
#include "bin/utils/setup_class.h"

#include <cerrno>
#include <codecvt>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <fstream>
#include <iterator>
#include <string_view>
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

#define v_str std::to_string
/// in-text number variable input like `"hi user"$(id)""`
#define $(x) + v_str(x) +


#ifndef __GMENG_LOG_TO_COUT__

#if GMENG_SDL
#define __GMENG_LOG_TO_COUT__ true
#else
    #define __GMENG_LOG_TO_COUT__ false
#endif

#endif

#define time_rn std::chrono::steady_clock::now().time_since_epoch()
#define GET_TIME() ( std::chrono::duration_cast<std::chrono::milliseconds>(time_rn).count() )

extern int g_mkid();
extern std::string get_curtime();
extern std::string get_curdate();

#ifndef GMENG_BUILD_NO
#define GMENG_BUILD_NO "(UNKNOWN_BUILD)"
#endif

std::string _uget_thread();

namespace Gmeng {
    enum KEYEVENT {
        gkEv_PRESS   = 1,
        gkEv_REPEAT  = 2,
        gkEv_RELEASE = 3
    };

    enum KEY {
        /// Alphabet
        gk_KEYS_START = 97,
        gk_A = 97,
        gk_B = 98,
        gk_C = 99,
        gk_D = 100,
        gk_E = 101,
        gk_F = 102,
        gk_G = 103,
        gk_H = 104,
        gk_I = 105,
        gk_J = 106,
        gk_K = 107,
        gk_L = 108,
        gk_M = 109,
        gk_N = 110,
        gk_O = 111,
        gk_P = 112,
        gk_Q = 113,
        gk_R = 114,
        gk_S = 115,
        gk_T = 116,
        gk_U = 117,
        gk_V = 118,
        gk_W = 119,
        gk_X = 120,
        gk_Y = 121,
        gk_Z = 122,
        gk_KEYS_END = 122,

        /// Numbers
        gk_NUMBERS_START = 48,
        gk_0 = 48,
        gk_1 = 49,
        gk_2 = 50,
        gk_3 = 51,
        gk_4 = 52,
        gk_5 = 53,
        gk_6 = 54,
        gk_7 = 55,
        gk_8 = 56,
        gk_9 = 57,
        gk_NUMBERS_END = 57,

        /// Special Characters
        gk_SPECIAL_START  =  96,
        gk_BACKQUOTE      =  96,
        gk_SPACE          =  32,
        gk_TAB            =   9,
        gk_BACKSPACE      = 127,
        gk_SLASH          =  47,
        gk_ENTER          =  13,
        gk_BACKSLASH      =  92,
        gk_MINUS          =  45,
        gk_EQUALS         =  61,
        gk_LSQUAREBRACKET =  91,
        gk_RSQUAREBRACKET =  93,
        gk_SEMICOLON      =  59,
        gk_APOSTROPHE     =  39,
        gk_COMMA          =  44,
        gk_DOT            =  46,
        gk_SPECIAL_END    =  46,

        /// Modifier characters
        gk_CAPSLOCK       = 57358,
        gk_ESC            =    27,
        gk_LSHIFT         = 57441,
        gk_LCTRL          = 57442,
        gk_LALT           = 57443,
        gk_LMETA          = 57444,
        gk_RMETA          = 57450,
        gk_RCTRL          = 57448,
        gk_RSHIFT         = 57447,

        /// ARROW KEYS
        gk_UP    = 57399,
        gk_DOWN  = 57400,
        gk_LEFT  = 57401,
        gk_RIGHT = 57402,

        /// Navigation
        gk_INSERT    = 57396,
        gk_DELETE    = 57397,
        gk_PAGE_UP   = 57398,
        gk_PAGE_DOWN = 57395,
        gk_HOME      = 57412,
        gk_END       = 57414,
    };

    /// Returns the 'pressed' state of a given key. Support is only enabled
    /// for this method if the `Kitty Terminal Keyboard Protocol` is supported
    /// by the terminal. Checked via `gmeng_keyboard_init()` and `gmeng_keyboard_deinit()`.
    extern const bool& getkey(const KEY& key);
    /// Gmeng internal key states map. Contains ASCII keycode to pressed state
    /// mappings. This map is updated via the function `gmeng_update_keys()`.
    /// Gmeng's `do_event_loop()` functionality automatically handles everything
    /// for tracking individual keys. users can get key states via `Gmeng::getkey()`.
    /// For a list of mappings to key codes, see the `Gmeng::KEY` enum.
    extern std::unordered_map<int, bool> key_states;
};


namespace Gmeng {
    /// Coordinate object.
    /// 2D plane, X and Y.
    struct Coordinate {
        int x;
        int y;
    };
    /// Gmeng's Renderer types. They might differ.
    /// A game can be launched via the console or
    /// the optional SDL2 based windowed system.
    enum Renderer_Type { CONSOLE, EXTERNAL };

    /// Gmeng's Renderer utilities. All functions related
    /// to rendering are contained in this namespace.
    namespace Renderer {
        /// A position within a viewpoint.
        struct drawpoint {
            /// X coordinate of the point.
            int x;
            /// Y coordinate of the point.
            int y;

            bool operator<(const drawpoint&) const;
            bool operator==(const drawpoint&) const;
            bool operator!=(const drawpoint&) const;

            const drawpoint operator*(size_t) const;

            /// Converts a drawpoint to a string. Useful when
/// logging or writing to the screen a drawpoint.
const std::string to_string();
        };

        /// A plane with a start and end position.
        /// A viewpoint can be many things, it can
        /// define a rectangle, define the display's
        /// boundaries, or a texture's displacement.
        struct viewpoint {
          public:
            /// starting position of the viewpoint.
            drawpoint start;
            /// ending position of the viewpoint.
            drawpoint end;

            const viewpoint operator*(size_t) const;
/// Checker for a drawpoint. Checks if a drawpoint is within
/// the boundaries of the viewpoint. p must be a constant,
      /// it can be dependant on or a subpoint of a viewpoint.
            /// E.g.: point(0,0) of viewpoint({ 5, 5 }, { 10, 10 })
            /// can not be used with this operator, you should first
            /// call viewpoint[ subpoint_or_dependant ] and pass its
            /// return value to the viewpoint() operator().
            const bool operator()(const drawpoint&) const;

            /// returns the true position of a subpoint within the viewpoint.
            /// For example:
            ///     let V = viewpoint({ 5, 5 }, { 10, 10 })
///     V[{ 0, 0 }] = { 5,   5 }
            ///     V[{ 5, 5 }] = { 10, 10 }
            ///
            /// THIS DOES NOT PERFORM BOUNDS CHECKING.
            const Renderer::drawpoint operator[](const drawpoint&) const;

/// moves a viewpoint by the provided offsets x and y.
            void move_by(int, int);
        };
    };
};

namespace Objects {
    struct coord {
        int x; int y;

        bool operator==(const coord&) const;
        operator Gmeng::Renderer::drawpoint() const;
    };
};

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
    /// Default 8-color palette of Gmeng.
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

    extern std::string colors[8];
    extern std::string bgcolors[8];
    extern std::string bgcolors_bright[8];
    extern std::string colorids[7];
    extern std::string resetcolor;
    extern std::string boldcolor;
    extern int rgb_colors[9][3];

    struct color32_t {
        uint8_t r, g, b;

        color32_t(uint8_t, uint8_t, uint8_t);
        color32_t(uint32_t);
        color32_t() = default;

        Vec3 to_vec3() const;

        bool operator==(const color32_t&) const;
        operator Vec3() const;
    };

    constexpr const uint32_t GMENG_COL_0_UINT32 = 16777215;
    constexpr const uint32_t GMENG_COL_1_UINT32 = 8627608;
    constexpr const uint32_t GMENG_COL_2_UINT32 = 12106534;
    constexpr const uint32_t GMENG_COL_3_UINT32 = 8828021;
    constexpr const uint32_t GMENG_COL_4_UINT32 = 16009524;
    constexpr const uint32_t GMENG_COL_5_UINT32 = 13862555;
    constexpr const uint32_t GMENG_COL_6_UINT32 = 16366639;
    constexpr const uint32_t GMENG_COL_7_UINT32 = 2631720;

    extern uint32_t uint32_from_color32(const color32_t&);    

    extern color_t conv_rgb_col8(const Vec3& rgb);

    struct unitcolor_t {
public:
          enum unitcoltype {
            COLOR_8, RGB
          };
          unitcoltype type = COLOR_8;
          int c8_value = -1;
          color32_t rgb_value = { 0, 0, 0 };

          int operator=(int);
          color_t operator=(color_t);
          color32_t operator=(color32_t);
          operator int() const;
          operator color_t() const;
          operator color32_t() const;
          unitcolor_t(int);
          unitcolor_t(color_t);
          unitcolor_t(color32_t);
};
    
};

namespace Gmeng {
    extern std::ofstream outfile;
    extern std::stringstream logstream;

    struct __global_object__ {
        std::vector<int> indexes;
        std::vector<std::string> containers;

        bool dev_console;
        bool debugger;
        bool log_stout;
        bool dev_mode;
        bool dont_hold_back;
        bool shush;
        bool weird_ass;
        bool restarted_instance;
        bool ignore_assert;

        std::string raw_arguments;
        std::string executable;
        std::string raw_executable_name;
        std::string user;
        std::string pwd;

        int prog_argc;
        char** prog_argv;

        bool window_control;
        
        bool terminal_has_sgr_pixel_support;
        bool terminal_has_kgp_pixel_support;
        bool terminal_has_keyboard_support;
        bool no_advanced_keyboard;
        bool bypass_splashscreen;
        bool app_focused;

        GMENG_INIT_TYPE( indexes, containers, dev_console, debugger,
                         log_stout, dev_mode, dont_hold_back, shush,
                         weird_ass, restarted_instance, ignore_assert,
                         raw_arguments, executable, raw_executable_name,
                         user, pwd, prog_argc, prog_argv, window_control,

                         terminal_has_sgr_pixel_support,
                         terminal_has_kgp_pixel_support,
                         terminal_has_keyboard_support,
                         no_advanced_keyboard,
                         bypass_splashscreen,
                         app_focused
                       );
    };

    extern __global_object__ global;
};

extern std::string g_joinStr(const std::vector<std::string>&,
                      const std::string&);

extern std::string get_executable_directory();

#define GMENG_GLOBAL


namespace Gmeng {
    struct Routine {
      public:
        /// ID of the routine.
        int id = g_mkid();

        /// priority of the subroutine. Higher priority routines
/// are executed first, and lower priority ones last.
        int priority;

        /// whether the routine has been initialized.
        bool init;
        /// name of the routine.
        std::string title;

        /// function to execute when running the subroutine.
        std::string entry_point;
        /// source code for the routine.
        std::string lua_source;

        /// value to determine how often the routine will run.
        /// Default is 1.0f, which is equal to 1 second. Value of
        /// 0.0f can be set, in which case the routine will be
        /// executed on every update() call.
        float period;
        /// progress of the subroutine, a counter updated with
        /// the default FIXED_UPDATE event handler which increments
        /// this value by 1.0f / display.FIXED_UPDATE_FPS every frame.
        /// progress / period returns how close to execution a subroutine
        /// is, in a 0.0 -> 1.0f range where 1.0f is the t = execute.
        float progress;
        /// whether the subroutine is an executable or a function.
        /// if set to false, the function becomes accessible from
    /// other subroutines. If for example a subroutine includes
        /// a utility function like `check_entity_status`, it can
        /// be executed by another subroutine instead.
///
        /// Note that non-executable functions must follow specific
        /// declaration order. This means that a subroutine referencing
        /// such a subroutine must come AFTER the non-executable subroutine
        /// being declared. To make sure of this order being followed,
        /// use the § `priority` parameter - higher priority means
        /// executing first, lower means last. Gmeng auto-sorts
        /// ONLY when using the default subroutine editor in the
        /// level manager as part of the builtin development console.
/// sort by priority using `std::sort` on entity->routines to
        /// achieve the same functionality as the engine's implementation.
        bool executable;

        GMENG_INIT_TYPE( id, title, entry_point,
                         lua_source, period, priority,
                         progress, init, executable );
    };

    using RoutineList = std::vector<Routine>;
}

#ifndef GMENG_NO_SOL
static int __gmeng_define_lua_routine_method;
#endif

namespace Gmeng::Assertions {
    using std::string;
typedef struct assert_t {
        enum jWRAP { ON, OFF, NOT_SET };
        std::map<std::string, jWRAP> headers;
        const char* bound;
    } vd_assert;

    typedef struct assert_data_t {
        string header;
        assert_t::jWRAP state;
        const char* bound;
    } assertable_t;

    extern std::map<const char*, vd_assert> list;

    extern vd_assert to_assert_t(assertable_t);
    extern void set_assert(assertable_t);
    extern vd_assert::jWRAP get_assert(string, const char*);
}
struct GMENG_NULL_T {
    void* content;
};

template<typename T = int>
struct not_nullptr_t {
T val = 1;
};

static not_nullptr_t<int> not_nullptr_ref;
static not_nullptr_t<int>* not_nullptr = &not_nullptr_ref;

/// runs a piece of code only if
/// Gmeng's debug mode is enabled
#define DEBUGGER if (Gmeng::global.debugger)
/// runs a piece of code only if
/// Gmeng's developer mode is enabled
#define DEVMODE if (Gmeng::global.dev_mode)

#define ASSERT(x,y) Gmeng::Assertions::set_assert(  \
            {                                       \
                .header = x,                        \
                .state = y,                         \
                .bound = __FUNCTION__               \
            }                                       \
        )

#define PREF(x) Gmeng::Assertions::get_assert(      \
            x,                                      \
            __FUNCTION__                            \
        )

#define GET_PREF(x, f) Gmeng::Assertions::get_assert( \
            x, f                                      \
        )

#define IS_SET Gmeng::Assertions::vd_assert::ON ==
#define IS_DISABLED Gmeng::Assertions::vd_assert::OFF ==
#define IS_UNKNOWN Gmeng::Assertions::vd_assert::NOT_SET ==

#define DISABLE() Gmeng::Assertions::vd_assert::OFF
#define ENABLE() Gmeng::Assertions::vd_assert::ON

#define p_no DISABLE()
#define p_yes ENABLE()

extern void __gmeng_write_log__(const std::string&, const std::string&, bool append = true);

#define vl_get_name(x) #x
#define vl_filename(path) (strrchr(path, '/') ? strrchr(path, '/') + 1 : path)

/// Gmeng : Splits a string with a given delimeter.
/// If no occurences of the delimiter is found, the
/// returned vector will only have 1 entry that contains
/// the original string without any modifications.
extern std::vector<std::string> g_splitStr(std::string s, std::string delimiter);

extern std::string get_filename(std::string);

extern void _gm_log(const char*, int, const char*, std::string, bool use_endl = true);

#define gm_log(x) _gm_log(__FILE__, __LINE__, __FUNCTION__, x)

#define GMENG_LOG

namespace Gmeng {
    /// Modifier : a named in-game variable that can
    /// be modified by the developer console. Used
    /// for cases where in-game changes may be requested
    /// by the player during runtime. Serialized into
    /// binary with `serialize_modifier`.
    struct modifier {
	  public:
        /// Name of the modifier. Accessable in
        /// `Gmeng::ModifierList::modifiers.at( name )`.
		std::string name;
        /// Value of the modifier. Only integer values
        /// are accepted, for multiple reasons, main
        /// one being the fact that most of the time,
        /// modifiers are either enums (glorified integers)
        /// or integer values by nature anyways.
        int value;
	};

    /// ModifierList : Container for modifiers
    /// concerning any structure. Used in multiple
    /// internal classes like `Gmeng::Level` and `Gmeng::Camera`.
	struct ModifierList {
	  public:
        /// List of modifiers within the list.
		std::vector<modifier> values;

        /// Sorts the vector for binary-search operations,
        /// this makes access to values much faster.
        void sort();

        modifier& operator[](std::string_view key);

        /// Erases a modifier with the given key. If a modifier
        /// with the given key is not found, no item is erased.
/// If a modifier is found, the array is sorted after
        /// deleting the modifier.
        ///
        /// This method will only delete the first instance of
        /// a modifier with a given key. It needs to be reexecuted
        /// n times if there are n amount of instances of the modifier.
        void erase(std::string_view key);

        bool has_value(std::string_view key) const;

        /// Returns the value of a modifier with the key
        /// provided. The function will return an unknown
        /// instance of the value, so if there are multiple
        /// stacked within the list any one may be returned.
        /// If no instances of a modifier with the given key
        /// is found, `-1` is returned.
        int get_value(std::string_view key) const;

        /// Sets the value of a modifier with the key
        /// provided. The function will only set the first
        /// instance of the value, so if there are multiple
        /// stacked within the list only the first one will
        /// be modified. If no instances of a modifier with
        /// the given key is found, nothing is modified.
        ///
        /// This function does NOT create a new entry if
        /// a modifier with the key provided exists.
        void set_value(std::string_view key, int value);

        /// Adds a modifier to the list. If there is already
        /// a modifier with the same key, this function does
        /// NOT modify the value. In every case, this function
        /// will add the new modifier to the end of the list.
        /// Even if there is already a modifier with the same key.
        void _add_value(modifier mod);
        /// Adds a modifier to the list. If there is already
        /// a modifier with the same key, this function does
        /// NOT modify the value. In every case, this function
        /// will add the new modifier to the end of the list.
        /// Even if there is already a modifier with the same key.
        void add_value(std::string key, int value);

	};    
}

namespace Gmeng {
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

    /// Unit : Gmeng Implementation for screen piece.
    /// Works as a pixel, a fragment of a texture, model,
    /// entity, or display map.
	struct Unit {
      public:
            /// 24-bit representation of RGB derived
            /// from `gmeng::color32_t`. values 0-7 are
            /// reserved for Gmeng's default palette.
			uint32_t color;
            /// whether the Unit can be collided with.
            /// If set to true, collision is disabled
            /// and entities can collide with this specific
            /// unit within its larger container (Models, Entities.)
            bool collidable;
            /// @deprecated gmeng 1.1 - whether the unit is part of an entity.
            /// kept for backwards compatibility but is unused since gmeng 4.0.
            bool is_entity;
            /// whether the Unit is transparent. If set
            /// to true, the unit is treated as nonexistent,
/// and will not have collision enabled even if
            /// its `collidable` property is set to true.
            bool transparent;
            /// whether the unit is special. Special units
            /// will break `cubic_render` functionality and
            /// display a full-height character in the size
            /// of a 2x1 rectangle, but allow for displaying
            /// custom items within 2 units. For esoteric
            /// textures and models, this can come in handy.
            bool special;
            /// color of the special unit. If `special` is
            /// enabled, `special_clr` will set its foreground
            /// color (24-bit RGB derived from `gmeng::color32_t`).
            int special_clr;
            /// character string of the special unit. If
            /// `special` is set, this item will be displayed
            /// in a 2x1 rectangular full-size character occupying
            /// the next Y coordinate's same X position.
            char special_c_unit;

            ~Unit() = default;

            bool operator==(const Unit&) const;
	};
};

namespace Gmeng {
struct BaseImageMethods {
        virtual void resize( const std::size_t& width, const std::size_t& height ) = 0;
        virtual void resize( float scale ) = 0;
        virtual void recolor( uint32_t color ) = 0;
        virtual void recolor( uint32_t color, uint32_t to ) = 0;
        virtual uint8_t* rgb_array() = 0;

        bool operator==(const BaseImageMethods&) const = default;

        virtual ~BaseImageMethods() = default; 
    };

    /// Blob : Render blob that contains a vector of units
    struct Blob : public BaseImageMethods {
      public:
        /// Width of the render blob.
        std::size_t width;
        /// Height of the render blob.
        std::size_t height;
        /// Unit vector of the render blob. The vector
        /// is treated as a 2D entity with a size of
        /// `VECTOR( WIDTH * HEIGHT )`, every time
        /// `vector.at(n) % WIDTH` is `0 (divisable)`,
        /// the next `Y++` value within the `HEIGHT`
        /// of the blob is present.
        std::vector<Gmeng::Unit> units;

Blob(std::size_t w, std::size_t h, std::vector<Gmeng::Unit> u = {})
            : width(w), height(h), units(std::move(u)) {};
        Blob() = default;

        /// Recolors the Blob to the specified color.
        void recolor( uint32_t color ) override;
        /// Recolors units within the Blob that are colored
        /// `color` with the color specified with `to`.
        void recolor( uint32_t color, uint32_t to ) override;

        /// Resizes the Blob and the vector of Units.
        virtual void resize( const std::size_t& new_width, const std::size_t& new_height ) override;

        /// Resizes the Blob and the vector of Units.
        virtual void resize( float scale ) override;

        /// returns the texture's units as a uint8_t RGB array.
        virtual uint8_t* rgb_array() override;

        virtual const Blob get_resized( float scale ) const;

        bool operator==(const Blob&) const = default;

      protected:
        void _resize( const std::size_t& new_width, const std::size_t& new_height );
    };

    /// Texture : Gmeng Implementation for textures.
    /// Works as an image, as the terminal does not support
    /// importing image files to draw, this is a custom implementation.
    struct texture : public Blob {
      public:
        /// Collision property. If set to true,
        /// the texture (weirdly having this parameter)
/// will not allow for phasing (multiple-item collision).
        bool collidable;
        /// Name of the texture. Used by the VisualCache
        /// Graphics Manager to sort and quickly access
        /// items within the texture dictionary.
        std::string name;

        using Blob::resize;

        texture() = default;
        texture( const Blob& b, bool _collidable, std::string name ) :
            collidable( _collidable ), name( name ) {
                this->width  = b.width;
                this->height = b.height;
                this->units  = (b.units);

            };

        void resize(const std::size_t &new_width, const std::size_t &new_height) override;

        void resize( float scale ) override;

        GMENG_INIT_TYPE( width, height, units,
                         collidable, name );
    };

    extern texture notxtr;
};

namespace Gmeng {
    extern std::map<std::string, std::string> func_annotations;
    extern std::ofstream funclog;
    extern bool functree_init;
    volatile extern bool functree_enabled;
    volatile extern bool functree_extensive;
    extern std::vector<std::string> func_last;
    extern std::stringstream functree_calls;
};

// annotates a function, like information about a function.
extern void _func_annot(const char* func, const char* info);


#define __annot__(func, info)      _func_annot(vl_get_name(func), info)
#define __info__                   __annot__
#define __annotation__             __annot__

#define __functree_init__() if (!Gmeng::functree_init) Gmeng::funclog << "-- cleared previous log --\n~~GMENG_FUNCTREE~~\n*** This file is used for diagnostics ***\n", Gmeng::functree_init = true

extern void _functree_vl(char*, int, const char*, const char*);

#define __functree_call__(func) _functree_vl(__FILE__, __LINE__, vl_get_name(func), __PRETTY_FUNCTION__)

namespace Gmeng::Renderer {
        /// returns drawpoint object as log string
        extern std::string conv_dp(drawpoint p);
        extern std::string conv_vp(viewpoint vp);        
        /// Generates an empty texture with a pink-black layer
        /// (the default null layer), can be placed into a Model
        /// to become a textureless object.
        extern Gmeng::texture generate_empty_texture(int width, int height);
};

namespace Gmeng {
    /// current version of the engine.
    /// "-d" suffix means the version is a developer version, high unstability level
    /// "-b" suffix means the version is a beta version, low unstability level but unpolished
    /// "-c" suffix means the version is a coroded version, low to medium unstability level but specific methods will not perform as expected
    extern std::string version;
};

namespace Gmeng {
    class EntityBase;

    struct Entity_Interaction {
      public:
        enum Interaction_Type {
            LMB_CLICK, RMB_CLICK,
            MMB_CLICK, KEYPRESS,
            PROXIMITY, SCRIPT,
            UNKNOWN_INTERACTION
        };

        Interaction_Type type;
        std::shared_ptr<EntityBase> interacted_by;
    };
    
    class Level;
    class EventLoop;

    class EntityBase {
      protected:
        // Entity derivatory class id assignment
        // this is increasingly added each time
        // a new class derives from 'Entity'.
// with the Entity class.
        static int id_counter;

        // Registers a new entity derivatory.
        // Required for serialization, otherwise
        // all entities would be saved as a different type.
        template <typename Derived>
        static void register_derived_class(int id);

      public:
        /// DO NOT TOUCH, USE OR RUN THIS METHOD IF YOU
        /// ARE NOT AWARE OF WHAT IT WILL DO. YOU WILL BREAK
        /// YOUR ENTITY ID SYSTEM AND FUCK YOUR LEVEL UP.
        static int incr_id(int value = -1);
        /// decides whether an entity will be rendered / utilized or not.
        bool active;

        /// Returns the derivatory factory containing the map
        /// for the function that constructs the current Entity type.
        ///
        /// Used for serialization.
        static std::unordered_map<int, std::function<std::unique_ptr<EntityBase>()>>& get_derived_factory();

        // well, brave calling this a "sprite".
        // Texture of the entity.
texture sprite;
        // position of the Entity.
        // The position is not CENTRAL.
        // The 'position' value returns the position of the 1st unit of the
        // entity.
        // For example:
//
        // 'o' for map, 'x' for entity units
        //
        // oooo
        // ooXx
        // ooxx
        //
        // the capitalised 'X' is the position value.
        // So in this case it would be (x=2,y=1)
        Renderer::drawpoint position;
        // random id, to differenciate entities
        // does not have to be user-set unless
        // the entity is being duplicated.
        //
        // constructor() initializes with g_mkid().
        // see gmeng.h:g_mkid() for info.
        unsigned int entity_id = g_mkid();
// interaction proximity,
        // the amount of units a player should be
        // in proximity to the entity for the
        // entity->interact() function to be called.
        //
        // Defaults to 5
        unsigned int interaction_proximity;

        /// cache status of the entity.
        /// Can be used to send client updates in multiplayer scenarios.
        bool cached;

        /// modifiers of the entity.
        ModifierList modifiers;

        /// Routines of the entity. They are executed according
        /// to a specified period with its handler being a lua
        /// function `entity_handler`.
        RoutineList routines;

        GMENG_INIT_TYPE( active, sprite, position, entity_id, routines,
                         interaction_proximity, cached, modifiers );

        /// Entity creation
        EntityBase() = default;

        /// Entity from a texture and position.
        EntityBase(texture model_, Renderer::drawpoint pos_) :
            sprite(model_), position(pos_) {
                if (!this->modifiers.has_value("z_index"))
                    this->modifiers.add_value("z_index", 0);
            };

        /// Default deconstructor
        virtual ~EntityBase() = default;

        /// Returns the serialization id of the
        /// derived Entity class.
        virtual int get_serialization_id() const = 0;

        /// default serialization code for base entity objects.
        void default_serialize( std::ostream& out ) const;

        /// default deserialization code for base entity objects.
        void default_deserialize( std::istream& in );
        /// serialization function.
        /// Classes can override this function to add different serialization code.
        virtual void serialize(std::ostream& out) const;
        /// deserialization function.
        /// Classes can override this function to add diferent deserialization code.
        virtual void deserialize(std::istream& in);

        /// Method to be called when requesting a texture
        /// for the entity.
        ///
        /// This is used because the Entity::sprite texture
        /// may not be finalized.
        ///
        /// The draw function allows for changes in the sprite
        /// with the ability to take into account other variables
        /// like the EventLoop states.
        ///
        /// e.g. zombies eyes turn red when the player is
        /// within 5 units of proximity
        virtual texture& draw( Renderer_Type type = CONSOLE );

        /// Internal execution function for this->interact()
        /// for multiplayer-mode caching check
        void _interact( Entity_Interaction interaction, Level* lvl );

        /// Internal execution function for this->periodic()
        /// for multiplayer-mode caching check
        void _periodic( EventLoop* ev );

        /// Internal execution function for this->animate()
        /// for multiplayer-mode caching check
        void _animate( EventLoop* ev, int FPS = 60);

        /// Method to be called when any interaction with
        /// an entity takes place.
        ///
        /// Interactions have proximity, using the
        /// Entity::interaction_proximity value.
        virtual void interact( Entity_Interaction, Level* );

        /// Method to be called periodically at every FIXED_UPDATE event
        /// overriding this method allows you to give functionality & AI to entities.
        ///
        /// Examples of this can be following the player,
        /// checking for proximity with other entities, etc.
        virtual void periodic( EventLoop* ev = NULL );

        /// This method is called periodically every 250ms for all entities.
        /// It is to provide animation capabilities with a stable interval.
        ///
        /// This way, you do not have to rely on the periodic() method to animate
        /// the texture of an entity.
        ///
        /// NOT REQUIRED! Animated sprites are optional.
        virtual void animate( EventLoop* ev = NULL, int FPS = 60 );
    };
    /// sets off the derivatory id count from 0.    
    int EntityBase::id_counter = 0;

    /// Virtual class for Extendable Entity
    /// classes.
    ///
    /// Derive this class with:
    /// ```
    /// class MyEntityType : public Entity<MyEntityType> {
    ///
    /// };
    /// ```
    /// and add behaviour to interact(), draw(), animate()
    /// and etc to your new entity type.
    template <typename Derived>
    class Entity : public EntityBase {
      public:
        /// Derived entity ID, used to distingish entity types.
        /// Entity<Derived>::id will be uniquely incremented from
        /// any other Entity<Derived2>. But, Entity<Derived>::id will
        /// be equal to another Entity<Derived>::id.
        ///
        /// To distinguish Entities from each other (not entity types),
        /// use Entity<Derived>.entity_id. Uniquely generated with g_mkid().
        /// see gmeng.h:g_mkid() for information on ID generation.
        static const int id;

        Entity() {
            // register the new class (that extends this) to the derived class factory
            Entity::register_derived_class<Derived>(id);
        };
        /// returns the serialization id (so the derived class id)
        /// of the derivatory class that extends Entity : EntityBase.
        int get_serialization_id() const override {
            return id;
        };
    };

    /// Set derivatory class ID.
    template <typename Derived>
    const int Entity<Derived>::id = EntityBase::id_counter++;

    /// Dictionary of all registered entities visible to the engine.
    /// Returns the name of the entity type from the entity serialization id.
    extern std::map<int, std::string> entity_dictionary;

    /// returns the name of an entity type from its serialization id.
    extern const std::string get_entity_name( const int serialization_id );

    template<typename Derived>
    class DerivedEntityRegistrar {
      public:
        std::string __name;
        DerivedEntityRegistrar( char* nm ) {
            gm_log("initializing derived object type " + std::string(nm) + " with id ENTITY(" + v_str(Entity<Derived>::id) + ")");
            this->__name = nm;
            Entity<Derived> eclass_1;
            Derived eclass;

            entity_dictionary[ eclass_1.get_serialization_id() ] = std::string(nm);
        };
    };

/// Registers an entity type.
/// Must be called for all new Derived Entity types.
/// also see GMENG_ENTITY_SET_ID(0).
///
/// If you do not register entity types, save files will seg-fault
/// when loading due to no initial registry to the factory having been
/// made. This ensures the factory has an overload for the Derived Class.
#define REGISTER_ENTITY_TYPE( eclass ) \
    static DerivedEntityRegistrar<eclass> derivatory__##eclass( #eclass ); \
    static std::string __derivatory__name__##eclass = derivatory__##eclass.__name; \
    static int __entity_derivatory__id_incr_##eclass = EntityBase::incr_id()

/// Returns the serialization ID of the derived entity class.
#define GMENG_ENTITYID( eclass ) Entity<eclass>::id

#define GMENG_ENTITY( object_name, cast_to_class, base_object_name ) \
    cast_to_class* object_name = dynamic_cast<cast_to_class*>( base_object_name )

/// ABSOLUTELY DO NOT USE THIS FOR YOUR ENTITIES I DO NOT KNOW ENOUGH
/// OF THE ENGLISH LANGUAGE TO PROPERLY EXPLAIN HOW DAMAGING THIS WILL BE
///
/// JUST USE REGISTER_ENTITY_TYPE. IT IS FOR USERS. IT IS FOR THE EXTERNAL
/// USAGE. NOT THIS. THIS WILL FUCK YOU. AND GIVE YOU STDS.
///
/// GMENG_INTERNAL_ENTITY DOES NOT MAKE YOUR ENTITIES COOL. IT MAKES THEM
/// UNUSABLE BECAUSE THE INTERNAL INCREMENTER HAS GONE OFF COURSE AND NOW
/// YOUR ENTITIES HAVE WRONG ID's ASSIGNED TO THEM
///
/// GOOD LUCK CASTING TO VOID* AND REFACTORING FROM EntityBase
///
/// FOR INTERNAL USAGE:
///    class_def_ending_token {}; GMENG_INTERNAL_ENTITY( EntityName ); REGISTER_ENTITY_TYPE( EntityName );
#define GMENG_INTERNAL_ENTITY( eclass ) \
    template<>      \
    inline const int Entity<eclass>::id = EntityBase::id_counter; \
    inline const int __internal_entity_derivatory_id_increment__##eclass = EntityBase::incr_id()

/// PREVIOUSLY INTERNAL UTILITY THAT I REALISED WAS GOOD FOR NORMAL USERS
/// ---------------------------------------------------------------------
/// PURPOSE: sets internal derivatory_entity_id to a static number. this
///          is good if you ever have to reorder your Entity types for
///          the entire duration of your game's development phase.
///
/// REASON:  extends REGISTER_ENTITY_TYPE(). really good practice since
///          entities are really hard to serialize by themselves because
///          there are different Entity classes with different variables
///          that needs to be saved to a level file. Gmeng does this by
///          incrementing an internal derived_factory_id_counter but this
///          will cause errors if Entity classes initialization order is
///          misaligned / the order is changed. While this is savable by
///          just reordering the Entity initialization to the previous
///          state, it's better to use GMENG_ENTITY_SET_ID( unique_id )
///          to differentiate entity derivatory types. It was a dumb
///          decision in Gmeng 11.0.0 to decide based on id and not typename,
///          but this is a good solution.
///
/// USAGE:   before calling REGISTER_ENTITY_TYPE( DerivedEntityClassName ),
///          call GMENG_ENTITY_SET_ID( DerivedEntityClassName, unique_id ).
///          the id passed into the GMENG_ENTITY_SET_ID must be unique and
///          *NOT RANDOM*. If it's random, your engine build will not be able
///          to load entities from save files and your save file will be
///          rendered useless. set them by hand. don't automate the ids if
///          you use this macro, as it's whole point is manually assigning ids.
///          if you don't want to manually assign ids, REGISTER_ENTITY_TYPE will
///          assign one, but you will have to keep your entity class definition
///          order the same and never add in between and before your previous
///          classes, only after.
/// ---------------------------------------------------------------------
/// FOR INTERNAL USAGE (ONLY GMENG DEVELOPERS *NOT* USERS):
///          VALUES LOWER THAN 0 CAN BE SET TO RESERVE ENTITY IDs FOR INTERNAL
///          ENTITIES. THIS WILL ALLOW FOR SERIALIZATION TO NOT GO CRAZY WHEN
///          NEW GMENG SOURCE ENTITIES ARE ADDED OR THE ORDER OF ENTITIES CHANGE.
/// ---------------------------------------------------------------------
#define GMENG_ENTITY_SET_ID( eclass, ___id ) \
    template<> \
    inline const int Entity<eclass>::id = ___id;

    /// Player Entity object.
    /// Stable for every gmeng instance.
    ///
    /// Extend this class to add different functionality.
    class Player : public Entity<Player> {
      public:
        int interaction_proximity;
        Renderer::drawpoint position;

    }; /* do not question why this is not set as an internal entity.
          do even more not set it as an internal entity. just don't.*/
       REGISTER_ENTITY_TYPE( Player );

    struct LightingState {
        color32_t surface_color;
        color32_t light_color;
        float r_factor;
        float g_factor;
        float b_factor;
        float modulation_factor;

        int brightness;
    };

    /// Light Source Entity object. Used for
    /// illumination in levels. add as an entity
    /// to change brightness & color in your level.
    ///
    /// Requires your level camera to have modifier
    /// 'lighting' to be set as '(int)1'.
    ///
    /// LightSource objects have a size of 1x1 units.
    class LightSource : public Entity<LightSource> {
      public:
        /// LightSource interactions do not happen.
        /// LightSources are not interactable entites,
        /// do not use this variable. Will always be -1.
        int interaction_proximity;
        /// unused : lightsource does not have a texture.
        texture sprite;
        /// color of the light source, changing it
        /// will modulate the units within reach
        /// of the source.
        color32_t color;
        /// intensity of the light source, maxes
        /// at 1. for global illumination disable
        /// lighting completely. will turn camera to
        /// full brightness, and disable shading.
        ///
        /// intensity is scaled unit wise and decrementally.
        /// if intensity is 10 at position { 0, 0 },
        /// intensity will be 9 at positions { 1, 0 },
        /// { 0, 1 } and { 0, 1 }; etc.
        int intensity;
        /// Max brightness of the LightSource.
        /// while intensity can be scaled infinitely,
        /// if a max brightness is set then a
        /// Gmeng::Camera::calculate_illumination function call
        /// will not scale higher than the max_brightness value.
        ///
        /// '10' (default) is the max value until colors are
        /// brightened & intensified above their regular values.
        int max_brightness;
        /// Whether the light source is cached.
        /// This is for optimized rendering, as if a light source is cached,
        /// unless the light source was changed the cached data is reusable.
        bool cached;
        /// cached minimum brightness value of the light source.
        /// used for rendering in get_renderscale() for cache validation
        int cached_min_brightness;
        /// cached position of the lightsource, for automatic
        /// cache validation and updating if the lightsource
        /// moves, the minimum brightness property changes, etc.
        /// used for rendering in get_renderscale() for cache validation
        Renderer::drawpoint cached_position;
        /// cached intensity of the lightsource, for automatic
        /// cache validation and updating if the lightsource's
        /// intensity changes from the inspector/commandline.
        int cached_intensity;
        /// cached color of the lightsource, for automatic
        /// cache validation and updating if the lightsource
        /// moves, the minimum brightness property changes, etc.
        /// used for rendering in get_renderscale() for cache validation
        color32_t cached_color;
        /// validates the LightSource's cache by comparing the values to
        /// the object's non-cached properties. Such as:
        ///
        /// this::cached_position COMPARE this::position,
        /// this::cached_min_brightness COMPARE @param min_brightness
        void validate_cache( int min_brightness );
        /// custom serialization function for
        /// LightSource objects since they contain
        /// variables and parameters the base Entity
        /// type doesn't contain.
        void serialize( std::ostream& out ) const override;

        /// custom deserialization function for
        /// LightSource objects since they contain
        /// variables and parameters the base Entity
        /// type doesn't contain.
        void deserialize( std::istream& in ) override;
    }; /* LightSource is set to -1 to have a fixed entity id regardless of external entity types.
        * Also, it is not set as a GMENG_INTERNAL_ENTITY since LightSources are not 'entities', but
        * entity-type objects that should not be interactable but are dynamic in design.            */
       GMENG_ENTITY_SET_ID( LightSource, -1 ); REGISTER_ENTITY_TYPE( LightSource );    

    /// For versions 12.0.0 and later : SERIALIZED_ENGINE_INFO is an entity
    /// that contains engine info about version specifics and other data
    /// that may vary how the serialization/deserialization is handled
    class SERIALIZED_ENGINE_INFO : public Entity<SERIALIZED_ENGINE_INFO> {
      public:
        std::string build;
        std::string version;

        texture sprite;

        bool active;

        void serialize( std::ostream& out ) const override;

        void deserialize( std::istream& in ) override;

    }; GMENG_ENTITY_SET_ID( SERIALIZED_ENGINE_INFO, -2 ); REGISTER_ENTITY_TYPE( SERIALIZED_ENGINE_INFO );

    /// Interactable Entity.
    /// Set the proximity for the proximity limit of the interaction.
    /// set the interaction tag for the interaction callback.
    class Interaction : public Entity<Interaction> {
      public:
        /// the tag of the interaction. When this entity is interacted with,
        /// the callback from Gmeng::Level::callbacks that hold this tag will
        /// be called with Entity* and EventLoop* parameters.
        std::string tag;

        void serialize( std::ostream& out ) const override;

        void deserialize( std::istream& in ) override;

        void interact( Entity_Interaction inter, Level* lvl ) override;
    }; GMENG_ENTITY_SET_ID( Interaction, -3 ); REGISTER_ENTITY_TYPE( Interaction );

    class Animated_Entity : public Gmeng::Entity<Animated_Entity> {
      using texture = Gmeng::texture;
      public:
        std::vector<texture> anim_textures;

        bool animation_enabled;
        bool always_animate;
        int animation_step;

        bool init;
        uint64_t frame_index;

        Gmeng::Renderer::drawpoint cached_pos;


        void ae_init();

        /// Animation function. By default, this checks the cached
        /// position of the entity and if it has moved updates the
        /// current sprite the next animation step. Basically a
        /// move/walk animation. Override this to change functionality.
        ///
        /// In version 14.0.0, this will swap to a callback function instead
        /// so not all animations require their seperate entity types.
        void animate(Gmeng::EventLoop* ev, int FPS = 60) override;

        texture& draw( Gmeng::Renderer_Type ) override;

        void serialize( std::ostream& out ) const override;

        void deserialize( std::istream& in ) override;
    }; GMENG_ENTITY_SET_ID( Animated_Entity, -4 ); REGISTER_ENTITY_TYPE( Animated_Entity );

    /// ENDS ENTITY TYPES:
    ///     GMENG_INTERNAL_ENTITY will place entities at the end of the entity derived factory list.
    ///     So, GMENG_ENTITYTYPES_END::id is a way to check for Entity id overflow.
    class GMENG_ENTITYTYPES_END : public Entity<GMENG_ENTITYTYPES_END> {
    }; GMENG_INTERNAL_ENTITY( GMENG_ENTITYTYPES_END ); REGISTER_ENTITY_TYPE( GMENG_ENTITYTYPES_END );
};

#ifndef GMENG_NO_SOL

static int __gmeng_define_lua_entity_method;

#endif

// place TRACEFUNC at the end of your function parameters
// to get information about the caller of the current function.
// creates the following variables for the scope this util was
// used in:
//
// - func_caller for the name of the calling function,
// - func_caller_file for the name of the calling file,
// - func_caller_line for the name of the calling file line.
//
// Must be placed as a function parameter, will not function
// otherwise.
//
// Usage:
// void func1(int otherparam, int someotherparam, int optionalparam = 0, TRACEFUNC) {
//    std::cout << func_caller << " from file " << func_caller_file << ":" << func_caller_line << " called func1().\n";
// };
//
// void caller() {
//    func2(0, 1);
// }
//
// can also use TRACEFUNC_STR like:
//
//void func1(TRACEFUNC) {
//    std::cout << TRACEFUNC_STR << '\n';
// };
//
// void caller() {
//    func2();
// }
//
#define TRACEFUNC const char* func_caller       = __builtin_FUNCTION(), \
                  const char* func_caller_file  = __builtin_FILE(),     \
                  int         func_caller_line  = __builtin_LINE()

// place TRACEFUNC_STR inside a function that uses a
// TRACEFUNC parameter. Will not work otherwise.
//
// Usage:
//
// void func_with_tracefunc(TRACEFUNC) {
//     std::string func_trace_string = TRACEFUNC_STR;
// }
#define TRACEFUNC_STR std::string(func_caller_file) + ":" + std::to_string(func_caller_line) + " -> " + std::string(func_caller)

struct TerminalSize {
    int width; int height;

    bool operator==( const TerminalSize& other ) const = default;

};

extern TerminalSize get_terminal_size();

extern void centertext(std::string& text, TerminalSize sz = get_terminal_size());

extern std::map<std::string, std::function<void()>> gmeng_warnings;

// internal warning method.
// displays warnings related to functions of the engine.
// do not use this method for your game. implement your own
// warning screen mechanisms.
extern void _gmeng_show_warning(std::string warning_, char* filename, int fileline, TRACEFUNC);

// internal warning method. displays warnings related to functions of the engine.
// do not use this method for your game. implement your own warning screen mechanisms.
#define gmeng_show_warning(x) _gmeng_show_warning( x, __FILE__, __LINE__ )

extern void gm_err(const int& type, const std::string& err_title, TRACEFUNC);


/// Replaces all occurences of a substring within
/// a given string to a given substring.
extern std::string str_replace(const std::string& str, const std::string& from, const std::string& to);

/// foregroundcolor text identifiers.
/// TODO: convert to `char` array.
extern std::vector<std::string> colornames;

/// backgroundcolor text identifiers.
extern std::vector<std::string> bgcolornames;

/// Formats a text with colorcodes. Converts `~<colorname>~` indexes
/// into ANSI escape code sequences.
/// Example:
/// ```
/// std::string f_string = "~g~green text ~n~normal text ~b~blue text ...";
/// std::string formatted = colorformat(f_string);
/// ```
extern std::string colorformat(const std::string& data, bool append_end_format = true);


/// Writes to the standard output with color formatting.
/// See `colorformat()` to see ANSI escape code sequencing.
#define SAY std::cout << colorformat
/// Writes to the standard output with information formatting.
/// See `colorformat()` to see ANSI escape code sequencing.
#define INF std::cout << "\033[1m" + Gmeng::colors[Gmeng::BLUE] + "(i) >> \033[0m" + Gmeng::colors[Gmeng::WHITE] + colorformat

/// Parses string variables (VERSION and BUILD).
/// str vars are accessed with the `$!__VARIABLE_NAME` syntax.
extern std::string parse_str_vars(const std::string& a);

/// Writes a parsed string to the standard output.
/// Parsed strings include string variables. See `colorformat()` for
/// ANSI escape sequencing and highlighting functionality.
#define WRITE_PARSED(x) std::cout << colorformat(parse_str_vars(x))


namespace Gmeng {
/// @since Gmeng 13.4.0-d: Terminal Graphics Renderer impementation.
/// Provides functionality to (very efficiently) draw raw framebuffers
/// of RGB pixels to the terminal screen.
///
/// This utility is built around the `Kitty Terminal Graphics Protocol`,
/// which provides the API for drawing pixels within terminals that have
/// implemented the protocol. However, Gmeng prefers the `Kitty` terminal
/// for this functionality as it's the most efficient; and, obviously, the
/// best implementation of the protocol out of the batch that supports it.
class term_graphics_renderer {
  public:
    /// Width of the renderer (in pixels, not units). This value is used for
    /// calculating the Shared Memory Size which contains the framebuffer.
    int width = 0;
    /// Height of the renderer (in pixels, not units). This value is used for
    /// calculating the Shared Memory Size which contains the framebuffer.
    int height = 0;
    /// Frame index, used for selecting the correct SHM file descriptor
    /// from term_renderer::shm_names which is more efficient than using
    /// a single object and constantly mutating it and sending update headers.
    uint64_t frame_index = 0;

    GMENG_INIT_TYPE( width, height, frame_index );

    /// Default constructor
    term_graphics_renderer() = default;
    /// Constructor with the width and height of the framebuffer. If the image's
    /// width and height changes (usually means that the Level::Display::viewpoint
    /// was adjusted), the changes must be reflected to this object's width & height.
    term_graphics_renderer(int w, int h);

    ~term_graphics_renderer();

    /// Present a framebuffer to the screen. The framebuffer in this case is
    /// almost always a vector<Unit> containing data for the next frame. Since
    /// a Unit object is much larger in size than a uint8_t object, by default
    /// `get_lvl_view` will convert and add the color parameter of any Unit object
    /// it parses over into `(uint8_t*)Camera::display_map::framebuffer`.
    void present(const uint8_t* framebuffer);
};
};

extern void gmeng_terminal_graphics_init();


/// default termios state. Used to disable rawmode.
extern struct termios orig_termios;

extern int FONT_SIZE_SET_IN_GMENG_KITTY_CONFIG;

/// Returns the width and height of a single character
/// using the current font in pixel size.
extern Objects::coord get_font_size_values();

extern Objects::coord get_terminal_size_in_pixels();

/// Disables SGR-Pixel rawmode by setting the
/// termios to the default state.
///
/// SGR-Pixel returns the pixel position of the mouse
/// within the Terminal, allowing for correct mouse clicks
/// in Gmeng's cubic_render mode, because it halves screen height.
extern void sgr_pixel_disable_rawmode();

/// Enables SGR-Pixel rawmode by setting the
/// termios to the default state.
///
/// SGR-Pixel returns the pixel position of the mouse
/// within the Terminal, allowing for correct mouse clicks
/// in Gmeng's cubic_render mode, because it halves screen height.
extern void sgr_pixel_enable_rawmode(bool state = true);

/// Enables SGR-Pixel mosue reporting
/// (chcp \033[?1016h escape code.)
extern void sgr_pixel_enablemouse();

/// Disables SGR-Pixel mosue reporting
/// (chcp \033[?1003l \033[?1016l escape code.)
extern void sgr_pixel_disablemouse();

/// Checks if the current terminal supports SGR-Pixel rawmode.
/// Requires the user to move their mouse to function. Not recommended to use.
///
/// SGR-Pixel returns the pixel position of the mouse
/// within the Terminal, allowing for correct mouse clicks
/// in Gmeng's cubic_render mode, because it halves screen height.
extern bool sgr_pixel_checksupport();

extern int attempt_set_terminal_sgr_support(bool& checker);


/// Terminal Utility for raw mode, text drawing, mouse tracking, etc
namespace Gmeng::TerminalUtil {
    extern void enable_mouse_tracking();
    extern void disable_mouse_tracking();

    extern struct termios orig_termios;
    extern bool already_set;

    extern void cleanup();

    extern void set_raw_mode(bool state);

    // not a function that can be disabled.
    // I tried.
    extern void set_non_blocking(bool state);
};

/// Gmeng : Repeats a given string `times` number of times.
/// The string is referenced but is not modified,
/// the function returns the generated string.
extern std::string repeatString(const std::string& str, int times);

/// Gmeng : Repeats any `Thing` of any type `times` number of times.
template<typename Thing>
extern std::vector<Thing> repeatThing(Thing obj, int times);


/// Gmeng : Splits a vector of any type with a user-provided `checker` function.
/// If the checker function returns true, the vector is split.
/// Returns a vector of a vector of things.
template<typename Thing>
extern std::vector<std::vector<Thing>> splitThing(std::vector<Thing>& obj, std::function<bool(Thing)> checker);


extern void repeat(int count, std::function<void()> func);
extern void repeat(int count, std::function<void(int count)> func);

extern wchar_t* repeat_wstring(wchar_t* wc, int times);

/// Repeats a wide string `times` amount of times
extern wchar_t* repeat_wstring(const wchar_t* wc, int times);

/// converts a wide string to a normal string.
extern std::string ws2s(const std::wstring& wstr);


namespace Gmeng {
    extern std::unordered_map<char, Gmeng::Blob> CHARACTER_FONT_TEXTURES;
};

struct LookupEntry {
    char key; Objects::coord value;
};

constexpr Objects::coord texture_lookup(const char);

extern Gmeng::Blob& gmeng_terminal_font_get_char(char);
extern int gmeng_terminal_font_init(char*);

extern const Objects::coord gmeng_terminal_font_calculate_text_size(std::string_view);


#ifndef GMNEG_MAX_MAP_SIZE
    #define GMENG_MAX_MAP_SIZE 65434
#endif

namespace Gmeng {
    /// DisplayMap, container for the unitmap within the
    /// camera. Units are not raw, so they can be accessed
    /// by external functions to be modified. Such changes
    /// will be reflected after `get_lvl_view` and before
    /// `emplace_lvl_camera` is called. Modifying them
    /// directly will only reflect for a single frame.
	template<std::size_t d_width = 0, std::size_t d_height = 0>
	class DisplayMap {
		public:
		/// Height of the DisplayMap.
        int __h ;
/// Width of the DisplayMap.
        int __w;
        /// Contains the undrawn unitmap within the camera.
        /// Units are not raw, so they can be accessed by
        /// external functions to be modified. Such changes
        /// will be reflected after `get_lvl_view` and before
        /// `emplace_lvl_camera` is called. Modifying them
        /// directly will only reflect for a single frame.
        std::vector<Gmeng::Unit> unitmap;
        /// Pool size of the unitmap. Differs from `CONSTANTS::UNITMAP_SIZE`.
        /// This property calculates (at compile-time) the byte size of the
        /// unitmap in accordance to the byte size of one `Gmeng::Unit` instance.
        int pool_size;
        /// Data of raw framebuffer. Used & updated if the Kitty Terminal
        /// Graphics Protocol is supported. This is used to display raw
        /// framebuffers to the terminal program's screen, which is much
        /// faster and more efficient than drawing ASCII colorcodes.
        uint8_t* framebuffer;
	};

    /// UI element blob that can be clicked/interacted.
    ///
    /// has callbacks for hover, left click and right click.
    ///
    /// Using the ui_clickable type is not done via creating
    /// instances and applying them. This is automatically formatted from
    /// a text string.
    ///
    /// Example usage:
    ///
    /// ui_element { .text="click #clickable{ <tag>, <color32_t bg>, <color32_t fg>, label of clickable } to quit." }
    /// <type> is BLOB, BUTTON or UNKNOWN. Set via 0,1,2 integers.
    /// <color32_t> is integers in color32 formatting. default colors are available 0-7.
    /// <label> is the title of the clickable entity that will show up in drawing.
    struct ui_clickable {
      public:
        enum clickable_type { BLOB, BUTTON, UNKNOWN };
        /// tag for clickable. the ID is used to differentiate clickable objects.
        unsigned int tag;
        /// type of clickable.
        clickable_type type;
        /// background color of the clickable.
        color32_t color_bg;
        /// foreground color of the clickable.
        color32_t color_fg;
        /// label of the clickable.
        std::string label;

        ui_clickable();
        ui_clickable( unsigned int tag, unsigned int type, color32_t bg, color32_t fg, std::string label );
    };

    /// UI element blob that can be emplaced to a Camera instance
    /// to be drawn. Can hold ui_clickable items such as buttons
    struct ui_element {
      public:
        /// position of the element. default is 1,1 which is the top left corner with
        /// a padding of 1 unit from the X and Y coordinates.
        Objects::coord position;
        /// active status. When set to false the Camera instance this element
        /// is emplaced unto will remove it from the std::vector of active elements.
        bool active;
        /// id for the element.
        int id = g_mkid();
        /// background color for the element
        /// default is black.
        color32_t color_bg = (uint32_t)BLACK;
        /// foreground color for the element
        /// default is white.
        color32_t color_fg = (uint32_t)WHITE;
        /// formatted text for the element. This supports ui_clickables.
        /// see doc:ui_clickable on how to incorporate ui_clickables into your
        /// ui elements.
        std::string text;


        /// expiration time for the element (in milliseconds)
        /// default (0) means no expiration until its 'active' parameter is set to false.
        unsigned int expire_after = 0;
        /// creation time_ms state for the element.
        unsigned long long created_at = 0;
        ui_element( unsigned int expire_after, std::string text, Objects::coord pos = { 0, 0 }, color32_t col_bg = (uint32_t)BLACK, color32_t col_fg = (uint32_t)WHITE );
        ui_element() = default;
    };

    /// Manages UI elements for a Camera instance.
    class Camera_UI_Manager {
      public:
        struct formatted_text_obj {
            enum what_is_it { TEXTBLOB, CLICKABLE };
            what_is_it type;
            std::string content_or_tag;
        };


        struct formatted_element {
            ui_element base;
            std::unordered_map<unsigned int, ui_clickable> clickables;
            std::vector<formatted_text_obj> text_formatted;
            Objects::coord position;
        };

        std::unordered_map<int, ui_element> elements;
        std::unordered_map<int, formatted_element> cache;


        ui_clickable parse_clickable( std::string& _text );

        struct clickablestate {
            std::vector<size_t> positions;
            ui_clickable clickable;
        };

        formatted_element format_element(ui_element& element_full);

        /// Get the final text content of the UI element as input.
        /// Requires a formatted element. Use format_element( ui_element& )
        /// to convert a raw element to a formatted element.
        std::string get_final(formatted_element& elem);

        /// Adds a UI element to the Camera.
        void add_element( ui_element& elem );

        /// updates a UI element by rewriting the cache at the specified ID.
        void update_element( ui_element& elem );
    };


/// Camera Instance of any viewport in the game engine.
    /// All displays are rendered through this class.
	template<std::size_t _w = 1, std::size_t _h = 1>
	class Camera { /// v8.2.1: Camera, v4.1.0: CameraView, v1.0.0: WorldMap
      public:
        /// width of the Camera.
        /// can be set by template parameters with Camera<5, 5>.
        /// or can be set with Camera<0, 0>.SetResolution(5, 5);
        std::size_t w = _w;
        /// height of the Camera.
        /// can be set by template parameters with Camera<5, 5>.
        /// or can be set with Camera<0, 0>.SetResolution(5, 5);
        std::size_t h = _h;
        // frame time, calculated internally
        // time in milliseconds for frame generation
        float frame_time = 0;
        // draw time, currently not internally calculated
        // time in milliseconds for frame drawing (to the output)
        float draw_time = 0;
        // entity count, set by external Level controller class.
        // will be 0 if used without external controller classes.
        uint32_t entity_count;
        // model count, set by external Level controller class.
        // will be 0 if used without external controller classes.
        uint32_t model_count;

        // all modifiers for the camera
        ModifierList modifiers = {
			{
                /// noclip modifier, disables all collision checking
                /// currently unused.
				modifier { .name="noclip",                  .value=0 },
                /// force_update disallows draw() from just writing the
                /// output in raw_unit_map, forcing a call to update() first.
				modifier { .name="force_update",            .value=0 },
                /// should be moved to the Game Event Loop.
                /// controls plugin usage.
				modifier { .name="allow_plugins",           .value=1 },
                /// deprecated, casts events through std::cerr
                /// used by old typescript eventloop.
				modifier { .name="cast_events",             .value=1 },
                /// for deprecated logging methods
                /// unused
				modifier { .name="allow_writing_plog",      .value=1 },
                /// cubic render, squishes units into squares [|] instead of
                /// writing them as 1x2 full unit characters [I].
                modifier { .name="cubic_render",            .value=1 }, // enabled since v8.2.2-d
			    /// debug render mode, enables many different rendering
                /// options, most notably model border rendering.
                modifier { .name="debug_render",            .value=1 }, // since v10.4.0-d
                /// wireframe render mode, draws wireframes on units with collision
                /// info. X for no collision, x for collision.
                modifier { .name="wireframe_render",        .value=0 },
                /// enable/disable lighting and illumination
                /// used by renderscale functions
                modifier { .name="lighting",                .value=0 },
                /// minimum light intensity for the camera, default 5
                /// increasing this brightens the entire camera view.
                modifier { .name="lighting_floor",          .value=5 },
                /// UI element controller. enables/disables HUD and other
                /// UI elements from being drawn.
                modifier { .name="allow_ui",               .value=1 },
                /// Whether FIXED_UPDATE event calls follow UPDATE event calls.
                /// FIXED_UPDATE will be called every frame when enabled.
                /// This can be disabled if in-game events only happen upon
                /// input from the player. Usually only in singleplayer games.
                modifier { .name="update_all_frames",      .value=1 },
                /// Whether the orange outline of the game view should
                /// be drawn. Applies to consoleview only.
                modifier { .name="draw_outline",           .value=1 },
                /// efficient_render mode. Various optimizations that make rendering
                /// chunks faster, but can sometimes cause issues with external game code.
                /// However, since these are almost completely nonexistent, we can get away
                /// with enabling efficient_render by default. For any edge-case that requires
                /// it to be disabled, this option is dynamic and can be disabled.
                modifier { .name="efficient_render",       .value=1 },
                /// kitty_graphics mode. This will enable using the Kitty Terminal Graphics
                /// Protocol to draw raw framebuffers to the terminal screen instantly.
                /// This is (1) faster, (2) better looking, and (3) really cool.
                /// Enabling this will not immediately cause Gmeng to use it, the value of
                /// `Gmeng::global::terminal_has_kgp_pixel_support` must be `true`.
                modifier { .name="kitty_graphics",         .value=1 },
                /// Enables the ImGui_ImplGmeng wrapper for ImGui which supports the
                /// Kitty Graphics Protocol to add ImGui to the term_graphics_renderer.
                modifier { .name="imgui", .value=1 },
            }
		};

        /// UI manager of the camera. Handles all UI elements
        /// including clickables and textblobs.
        Camera_UI_Manager ui_manager;
        /// Display map, contains the current
        /// screen data in units
		DisplayMap<_w, _h> display_map;
        /// Terminal Graphics Manager. Handles the Kitty Terminal
        /// Graphics Protocol and raw framebuffer present() methods.
        term_graphics_renderer renderer;
        // raw unit map, contains the rendered units
        // to be written to the console output
        std::vector<std::string> raw_unit_map = std::vector<std::string>( GMENG_MAX_MAP_SIZE );
        /// override map, for overriding specific units
        /// used by UI functions.
        std::map<int, std::string> overrides;
        /// extra drawlist, for adding render blobs to the
        /// camera with single-frame instances.
        std::vector< std::tuple< Objects::coord, Gmeng::Blob > > frame_draw_list;
        /// after-render drawlist, for adding UI elements
        std::vector< std::tuple< Objects::coord, Gmeng::Blob > > after_draw_list;

        GMENG_INIT_TYPE(w, h, frame_time, draw_time,
                              entity_count, model_count,
                              modifiers, ui_manager,
                              display_map, raw_unit_map,
                              overrides, frame_draw_list );

        /// calculates light illumination for a given RGB color
        /// and returns the modified color to be used instead
        LightingState calculate_illumination(
            const color32_t& color,
            const color32_t& light_color,
            int light_intensity_max,
            int light_distance,
            int max_brightness,
            int min_intensity
        ); 


        /// Sets the resolution of the camera.
		void SetResolution(std::size_t w, std::size_t h);
        /// Updates the raw_unit_map to match the display_map's
        /// units. This means rendering the camera's viewport fully.
		[[deprecated("update() is deprecated")]] void update();

        /// Temporarily sets the Unit in a position only on the raw_unit_map.
        /// This means that in the next rendered frame including this drawpoint
        /// the unit will revert to its original state.
        void temp_displacement(int __pX, int __pY, Gmeng::Unit u);

        /// Draws the raw_unit_map as a string (to be written to a terminal output stream)
        /// This method also appends an outline to the camera.
        /// Cubic rendering is accounted for automatically with this method.
        ///
        /// TODO: 13.0.0: make this function draw the screen in real time instead of
        /// appending to a string and drawing the final output. Will hopefully increase
        /// frame-rate in the console.
		std::string draw();

        /// sets the cursor's position (accounted for the frame)
		void set_curXY(int x, int y);

        /// Writes all UI elements to the camera view.
        /// Use only after a frame has been generated.
        ///
        /// Gmeng::Renderer::Display::refresh() automatically
        /// handles all required draws by default. This method
        /// should only be called by the user if you are using
        /// a custom screen refresh handler.
        void apply_ui();

        /// Adds a Blob render obect to the draw list. This method can
        /// be used to create basic shapes such as rectangles and circles
        /// that should not be treated as models or entities. Particularly
        /// useful for items like waypoints, objective markers, selection
        /// highlights and other items that are temporary.
        ///
        /// Gmeng's Easy Header ( `lib/bin/easy.h` ) provides basic
        /// geometric shape constructors for rectangles and circles.
        void add_draw( Objects::coord position, const Gmeng::Blob& blob );

        /// Adds a Blob render obect to the after-render draw list.
        /// This method can be used to draw custom UI elements.
        ///
        /// Gmeng's Easy Header ( `lib/bin/easy.h` ) provides basic
        /// geometric shape constructors for rectangles and circles.
        void add_draw_after( const Objects::coord& position, const Gmeng::Blob& blob );

        /// Adds a string to the drawlist. The text is split into each
        /// character and converted into a `Gmeng::Blob` via the font
        /// loaded with `gmeng_terminal_font_init( font_file )`.
        ///
        /// Gmeng's Text Header ( `utils/text.h` ) provides utilities
        /// for terminal-based fonts and texts. See `CHARACTER_FONT_TEXTURES`.
        /// You can add or set your custom font if you prefer. However, they
        /// are in a custom format. The default-provided font is at `assets/term-font`.
        /// See the directory's contents to understand how it is stored.
        /// Each character is a different gmeng texture object (Blob).
        void add_text( Objects::coord position, const std::string& text, uint32_t color = WHITE );

        /// checks for internal camera modifiers
		constexpr bool has_modifier(std::string_view name);

        /// updates internal camera modifiers
		[[deprecated]] void update_modifier(Gmeng::modifier& modifier, int value);

        /// sets internal camera modifiers
		[[deprecated]] void set_modifier(std::string_view name, int value);


        /// resets the cursor to the 0,0 position of the terminal
		constexpr void reset_cur();

        /// returns the 1D vector index of a unit
        /// as a 2D coordinate.
        ///
        /// eg.
        /// ```
        /// { 0, 1, 2, 3, 4,
        ///   5, 6, 7, 8, 9 }
        /// camera.width = 5
        /// get_xy(7) = { .y=1, .x=3 };
        /// ```
		Objects::coord get_xy(int __p1);

        /// draws some infographics to the screen.
        /// like the F3 menu in the minecraft engine.
        ///
        /// TERMINAL-MODE only.
        void draw_info(int x = 0, int y = 0);

        /// draws a 'Unit' object and returns it as a printable string
		std::string draw_unit(Gmeng::Unit __u, Gmeng::Unit __nu = Unit { .is_entity=1 }, bool prefer_second = false);

        /// rewrites the position given as parameter to the function
        /// from the raw_unit_map.
        ///
        /// this is useful for changes for only a single frame like
        /// a flashing or rapidly color changing unit because this
        /// method allows updating the screen without updating the
        /// entire screen buffer.
		void rewrite_mapping(const std::vector<int>& positions);

        /// clears the terminal screen buffer
		void clear_screen();

        /// @deprecated
		[[deprecated]] void rewrite_full();

	};
};

/// flips a vector vertically (used for mirroring textures & other vectors 2d grids.)
template<typename T> void flip_vector(std::vector<T>& data, int width, int height);

extern std::string execute_command(const char*);

/// Kitty Terminal : Returns the size of the terminal in pixels.
extern std::tuple<int, int> kitty_get_terminal_size_in_pixels();

/// Creates a Kitty terminal config that is
/// compatible with Gmeng's requirements.
extern void kitty_create_config(std::string fname);

/// Kitty Terminal : Checks if the application is focused.
extern int kitty_check_focus(bool set_global = true, TRACEFUNC);

/// Kitty Terminal : toggles fullscreen mode.
extern void kitty_toggle_fullscreen();

extern void kitty_close_window();

/// Kitty terminal : resizes the window in pixel format.
/// Requires an unknown amount of grace milliseconds.
extern void kitty_resize_window_pixels(int width, int height);

/// Kitty terminal : resizes the window.
/// Requires an unknown amount of grace milliseconds.
extern void kitty_resize_window(int width, int height, bool pixels = false);

/// Kitty terminal : increments the window size by X,Y pixels.
/// Requires an unknown amount of grace milliseconds.
extern void kitty_incrtl_window( int x, int y );

/// Kitty terminal : sets font size.
extern void kitty_set_font_size(int size);

/// Kitty Terminal : Increments window size by Width, Height.
extern void kitty_increment_window_size( int width = 0, int height = 0 );

/// Returns the address of any given object.
template<typename T>
extern uintptr_t _uget_addr(const T& obj);

/// Gmeng : returns whether a file with the given filename exists.
extern bool file_exists(std::string f);


/// Gmeng : returns the hex value of an integer
extern std::string _uconv_1ihx(int value);

using MouseClickCallback = std::function<void()>;

extern bool endsWith(const std::string& str, const char* suffix, unsigned suffixLen);

extern bool endsWith(const std::string& str, const char* suffix);

extern bool startsWith(const std::string& str, const char* prefix, unsigned prefixLen);

extern bool startsWith(const std::string& str, const char* prefix);

/// @deprecated sleeps the current thread for a given delay
/// and calls a callbackfunction after that delay (in milliseconds).
extern void g_setTimeout(std::function<void()> callback, int delay);

/// Gmeng : Reads a file and returns it as a string.
/// lines are split with `\n` or `\r\n` (depending on the line feed)
extern std::string g_readFile(const std::string &fileName);

/// Gmeng : Removes a group of characters from a given string.
/// All occurences of the characters in the given char[] array
/// are removed from the string. Since it is a reference, nothing is returned.
extern void g_rmChar( std::string &str, char* charsToRemove );

namespace Gmeng {
    enum CONSTANTS {
		/// integer values
        UNITMAP_SIZE = 32767 * 2,
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
			void cast_ev(int __etp, std::string __est);
			std::string gen_estr(Gmeng::event __e);
	};

    extern std::stringstream completelog;

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
};

/// Sets a Global Controller at `index` to `value`.
/// Global controllers are currently unused in the engine.
/// They may be used by game instances via accessing `Gmeng::global`.
extern void controller_set(int index, std::string value);

/// Switches the state of the global dev_console
/// variable. This will not close/open the console,
/// it will enable or disable the functionality to do so.
extern void switch_dev_console();


/// Returns the thread ID of the current thread as a hex string.
extern std::string _uthread_id(const std::thread& thread);



/// splits a text into characters but includes colorcodes in chars.
extern std::vector<std::string> split_with_ansi(const std::string& input);

/// Returns whether the character is
/// the start to an ANSI escape code sequence (`\x1B`)
extern bool is_ansi_start(char c) noexcept;

/// Returns an index to a logical index (index in a string EXCLUDING ansi escape codes)
/// as a physical index (index in a string INCLUDING ansi escape codes).
/// If the index cannot be found, `std::string::npos` is returned.
extern size_t logical_to_physical_index(const std::string& s, size_t logical_index);

/// strips all ANSI escape codes / color codes from a text.
/// Used for getting the actual length of strings without accounting
/// for colorcodes.
extern std::string strip_ansi(const std::string& input);

#define v_intl int
#define v_static_cast static_cast
#define v_sizel std::size_t
#define v_title std::string
#define v_constl const
#define v_endl std::endl
#define v_nl "\n"
#define v_rcol Gmeng::resetcolor


namespace Gmeng {
    /// converts a uint32_t integer containing
    /// an RGB value (NOT! RGBA) to a color32_t value.
    extern Gmeng::color32_t color32_from_uint32(uint32_t rgb);
};

/// @deprecated use ModifierList::get_value
/// Returns the index of a modifier with `targetName` within a vector.
extern int g_find_modifier(const std::vector<Gmeng::modifier>& modifiers, const std::string& targetName);

extern std::vector<std::string> _ulogc_gen1dvfc(int ln = 7400);

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

/// Returns the last line of a stringstream.
extern std::string get_last_line(std::stringstream& ss);

namespace Gmeng {
    /// i now realise this may not be very secure
    extern std::vector<std::thread> v_threads;
    extern std::thread create_thread(const std::function<void()>& func);
    extern void clear_threads();
    extern void join_threads();
};

extern void _gthread_catchup();
extern constexpr uint32_t _ghash(const char* data, size_t const size) noexcept;
extern wchar_t* concat_wstr(const wchar_t* str1, const wchar_t* str2);
extern wchar_t* concat_wstr(const wchar_t* str1, const std::string& str2);


extern void set_terminal_size(int width, int height);
extern void resize_terminal(int columns, int rows, bool force = false);

extern void print_windows_error_message();

extern void ansi_clear_screen();

#ifdef __APPLE__

/// (Gmeng) installs a font to the user's fonts directory
/// if the font is not already installed.
extern bool g_install_font(const std::string& fontFilePath);

/// Forces the macos terminal to use the correct
/// terminal profile (provided by gmeng) when running
/// the program.
///
/// Return values:
///    [success] 0 -> the profile is already set up, proceed
///    [error] 1 -> the current executable path too long
///    [error] 2 -> fail while copying terminal profile to /tmp
///    [success] 3 -> terminal profile launched
extern int enforce_macos_terminal_profile(const std::string& templateProfilePath, bool already_set_up);

/// Gmeng's internal setup for macOS terminals.
/// This installs the truetype font Hacker Nerd Font Mono,
/// Sets up the required Apple Terminal Profile for games
/// and ensures `make setup was successful`
extern bool gmeng_macos_terminal_setup(int argc, char* argv[], TRACEFUNC);


#endif


extern std::string boolstr(bool x);
extern void init_logc(int ms = 250);
extern void set_terminal_title(const std::string& title);

/// Gets the current working directory
extern std::string get_cwd();

/// returns the username of the current user
extern std::string get_username();

extern void apply_argument(const std::string& argument, bool& IS_USING_GMENG_APPLETERMINAL_THEMED);

/// returns true if the argument is usable, false if it
/// is an alias and has been consumed instead.
extern bool gmeng_check_argument(std::string& argument, bool& profile_c);

/// initializes gmeng. run on first line of `int main(int argc, char** argv)`.
extern void patch_argv_global(int argc, char* argv[], TRACEFUNC);

/// Restarts the current executable/program.
/// Requires `patch_argv_global()` to have been called.
/// This method runs `execvp` from the `unistd.h` library
/// to replace the current program image in the memory
/// with a new instance of the current program, in `argv[0]`.
extern int restart_program();


namespace Gmeng {
    extern bool save_texture( std::string fname, Gmeng::texture& tex, TRACEFUNC );
};

Gmeng::texture gmeng_create_highlighted_texture(const Gmeng::texture& tx, uint32_t color = Gmeng::RED);

#include <sol/sol.hpp>

namespace Gmeng {
    namespace Scripts {
        extern sol::state lua;
        extern sol::table gmeng_lua_namespace;
        extern sol::table renderer_lua_namespace;
        extern int init_sol2();
    };
};

static int sol2_init_state = Gmeng::Scripts::init_sol2();

extern void setRawMode(bool enable);

extern std::string lineinput(bool secret = false);

// generates a trajectory between two coordinates (x1,y1), (x2, y2)
// using the most efficient path towards point 2
//
// does NOT factor in collision. The trace WILL contain unit coordinates
// that are not collidable if they are part of the most efficient trajectory.
extern std::vector<Objects::coord> g_trace_trajectory(int x1, int y1, int x2, int y2);

namespace Gmeng {
    // applicable events for the game loop
    //
    // note that these events aren't what happens in a game itself, rather its interactions with the engine.
    enum Event {
        /// Initialization event. Called once when `do_event_loop` is ready.
        INIT,
        /// RAW update, ran every frame. NOT time-delta based.
        /// Used for processing rendering & real-time changes.
        ///
        /// Since this event is not fixed on a timeline, using
        /// it for input processing for actions like movement
        /// via `getkey()` will yield unstable results.
        ///
        /// Event handlers for physics and other time-based
        /// processes should use the FIXED_UPDATE event instead.
        UPDATE,
        /// FIXED update, time-delta based. This event is ran
        /// every `1000 / (level::display::fixed_update_fps)` milliseconds.
        ///
        /// The default timetable is 60FPS, so by default this
        /// event will be fired every ~16.6667ms. However, before
        /// calling`do_event_loop()`, users can set the
        /// `level.display.fixed_update_fps` value to their desired
        /// FPS for FIXED_UPDATE and then run the event loop to
        /// change how fast it fires.
        FIXED_UPDATE,
        /// RESET event. This happens when a core process of Gmeng
        /// is manipulated to hard fail, causing a RESET to be performed.
        RESET,
        /// RELOAD event, unused internally but scripts & the program
        /// can call this event to simulate a RELOAD event within games.
        RELOAD,
        /// EXIT event. Called when `ev->cancel()` is executed,
        /// `ev->cancelled` is equal to true, the `exit` command is called,
        /// the program receives the `SIGINT` signal from the terminal or
        /// if the application is running in SDL-Mode, when the window closes.
        EXIT,

        /// Mouse click start events
        MOUSE_CLICK_LEFT_START,
        MOUSE_CLICK_RIGHT_START,
        MOUSE_CLICK_MIDDLE_START,

        /// Mouse click end events
        MOUSE_CLICK_LEFT_END,
        MOUSE_CLICK_RIGHT_END,
        MOUSE_CLICK_MIDDLE_END,
        MOUSE_CLICK_END_ANY,

        /// Mouse scroll events
        MOUSE_SCROLL_UP,
        MOUSE_SCROLL_DOWN,
        MOUSE_MOVE, /// with 1006 raw input mode, we can capture mouse move events.

        /// Key events
        KEYPRESS,
        KEYREPEAT,
        KEYRELEASE,

        /// Unknown event
        UNKNOWN,
        /// helper for the event name receiver, unused
        EVENT_COUNT
    };

    extern const std::string event_names[EVENT_COUNT];

    /// returns the name of a Gmeng::Event as a string
    extern const std::string& get_event_name(const Event& event);    
    /// returns a list of every event in a vector as a string
    extern std::string list_events(std::vector<Event>& events);

    /// Gmeng Event Info, contains
    /// variables regarding an event_loop event.
    typedef struct EventInfo {
        /// The name of the Event
        Event EVENT;
        /// If it is a KEYPRESS event, this determines the keycode.
        /// TERMINAL-ONLY: Will not work with SDL.
        char KEYPRESS_CODE;
        /// For SDL-based events, so the symbol can be used instead of the charcode
        int KEYPRESS_SYM = -1;
        /// If it is a mouse event, the X position of the mouse.
        int MOUSE_X_POS = -1;
        /// If it is a mouse event, the Y position of the mouse.
        int MOUSE_Y_POS = -1;
        /// SDL-Only, mouse drag relative position X
        int MOUSE_REL_X_POS = -1;
        /// SDL-Only, mouse drag relative position Y
        int MOUSE_REL_Y_POS = -1;

        /// Raw Mouse Position, the value can be pixels if SGR-Pixel
        /// is supported by the terminal emulator the app is being ran on.
        Objects::coord RAW_MOUSE_POS = { 0, 0 };

        /// Prevents any other event hook from executing.
        bool prevent_further = false;
        /// Prevents default behaviour
        /// from executing
        bool prevent_default = false;
        /// Alternative usage.
        /// usually triggered by holding shift.
        /// one example is using shift + scroll for
        /// left-right movement instead of usual down-up movement.
        bool alternative = false;
        /// Keypress WITH ctrl keydown
        /// SDL-Only
        bool ctrl = false;
        /// Keypress WITH alt keydown
        /// SDL-Only
        bool alt = false;
        /// optional EventLoop object reference
        Gmeng::EventLoop* event_loop = nullptr;

    } EventInfo;

    static EventInfo NO_EVENT_INFO = EventInfo { Gmeng::UNKNOWN, 0, -1, -1, false };
    static EventInfo UPDATE_INFO   = EventInfo { Gmeng::UPDATE,  0, -1, -1, false };
    static EventInfo INIT_INFO     = EventInfo { Gmeng::INIT,    0, -1, -1, false };
};

class GmengImGuiTextureEditor {
  private:
    Gmeng::texture* tex_ptr = nullptr;
  public:
    std::string current_filename = "$!__NO_FILENAME";
    bool loaded = false;
    Gmeng::texture& tex = Gmeng::notxtr;
    Gmeng::Unit current_brush;
    float brush_color_float[3] = { 1.0f, 1.0f, 1.0f }; // For ImGui ColorEdit3
    std::set<uint32_t> texture_palette;

    float zoom = 57.2f;
    bool is_open = true;

    GmengImGuiTextureEditor(const std::string& default_filename = "custom.gt");
    GmengImGuiTextureEditor(Gmeng::texture& _tex, Gmeng::texture* tex_ptr);

    void ResizeTexture(std::size_t, std::size_t);
    void Save(const std::string& fname);
    void Load(const std::string& fname);
    void Draw();
};


namespace Gmeng {
    /// v10.1.0 - Unit object that has a position.
    struct Positioned_Unit {
        Unit unit;
        Objects::coord position;
    };
    /// v10.1.0
    struct Overlay {
        std::vector<Positioned_Unit> units;
    };

    extern color32_t apply_lighting(const LightingState& state);    

    /// EventHook handler function type
    using handler_function_type = std::function<void(Gmeng::Level*, EventInfo*)>;

    /// EventHook handler type for event_loop
    struct EventHook {
      public:
        int id = g_mkid(); std::vector<Event> events;
        handler_function_type handler;
        bool locked;

    };

    /// Gmeng EventLoop Object.
    /// EventLoop allows for a continuous running with very in-depth
    /// default behaviour as well as error handling, so the engine can
    /// do the heavy-lifting of rendering, running, optimizing and handling
    /// a game instance in Gmeng.
    ///
    /// Only one EventLoop object can be ran per gmeng instance,
    /// so no two games can be run at the same time with the same executable.
    typedef struct EventLoop {
      private:
          void init_default_event_hooks();
      public:
        /// ID of the GameLoop. Isn't useful within a single program but
        /// it can be used to communicate the instance on multiplayer
        /// instances to check if the required event loop is set-up on
        /// both clients, as a simple validation check in a server.
        int id;
        /// Level of the EventLoop. `do_event_loop` manages this level
        /// instance and emits events based on it.
        Gmeng::Level* level;
        /// Log stream of the level. also accessable via `GAME_LOGSTREAM`
        /// but this is neccessary for NOBLE scripts & plugins because they
        /// can't access the memory of the program they are hooked into,
        /// and since GAME_LOGSTREAM lives in the stack of the program they
        /// need a reference to it, via the `EventLoop::logstream`.
        std::vector<std::string>* logstream;
        /// log string of the level. also accessable via `GAME_LOGSTREAM_STR`
        /// but this is neccessary for NOBLE scripts & plugins because they
        /// can't access the memory of the program they are hooked into,
        /// and since GAME_LOGSTREAM_STR lives in the stack of the program they
        /// need a reference to it, via the `EventLoop::logstream_str`.
        std::stringstream* logstream_str; 
        /// EventLoop Modifiers.
        ModifierList modifiers = { {
            modifier { "allow_console", 1 },
            modifier { "server_passkey", 738867 } /// Should be changed for better password-protection in custom servers.
        } };

        /// Processes, used for registering event calls for the
        /// next tick of the event loop. Called with `UPDATE` event
        std::vector<EventHook> processes;

        /// Event Hooks of the Loop.
        std::unordered_map<Event, std::vector<EventHook>> hooks;
        /// Default Event Hooks of the Loop. Can be cancelled by
        /// external hooks to prevent default behaviour.
        std::unordered_map<Event, std::vector<EventHook>> defaults;

        /// Modifier of whether the EventLoop is cancelled or not.
        /// If cancelled is `true`, the game exits.
        bool cancelled = false;

        /// For External SDL Screen utility. Default to false.
        /// Changing this with code will not modify the SDL
        /// Window state, it is used for calcuations only.
        ///
        /// Setting this to `false` while an SDL window is open
        /// will freeze it, and you will have to force exit
        /// the window.
        bool uses_sdl = false;

        /// Total count of remote connections.
        /// This is used by default-provided script:
        /// 'server.dylib'.
        int RCON_REMOTE_COUNT = 0;

        /// type declaration for utility states
        struct UtilStateHolder {
            /// gmeng-provided developer console state
            bool* dev_console_open;
            /// gmeng-provided vgm editor state
            bool* vgm_open;
            /// gmeng-provided level inspector state
            bool* level_inspector_open;
        };

        /// holds states for the developer console,
        /// vgm editor and the level inspector.
        UtilStateHolder states;

        /// Frame times stored as an array of floats
        std::array<float, 10> frame_time_graph;

        GMENG_INIT_TYPE( id, level, logstream, logstream_str,
                         modifiers, processes, hooks, defaults,
                         cancelled, uses_sdl, RCON_REMOTE_COUNT,
                         states, frame_time_graph );

        /// EventLoop Constructor, Parses external EventHooks if any is provided.
        EventLoop( std::unordered_map<Event, std::vector<EventHook>> );
        EventLoop() = default;

        ~EventLoop();

        void next_tick(handler_function_type handler);

        void progress_tick();

        /// Registers an Event Hook to the EventLoop.
        /// Event Hooks can have multiple events to listen for.
        /// This is useful in cases where the interaction can be
        /// handled by the same event listener, like mouse interactions
        /// ({ MOUSE_CLICK_LEFT, MOUSE_CLICK_RIGHT }).
        void add_hook(std::vector<Event> events, handler_function_type handler);

        /// Emits an Event of type 'ev' with the specified EventInfo
        void call_event( const Event& ev, EventInfo& info );


        /// ImGui Texture editor map.
        std::map<std::string, std::unique_ptr<GmengImGuiTextureEditor>> editors;

        /// Ends the EventLoop, setting this->cancelled to true
        void cancel();

    } EventLoop;

    /// Main Gmeng EventLoop, for accessibility from everywhere.
    extern EventLoop* main_event_loop;


    typedef struct EventLoop_Controller_State {
        Event last_event = INIT;
        bool console_open = false;
    } EventLoop_Controller_State;
};

namespace Gmeng {
    /// returns position of a 2d pointer in a vector as a 1d index to the vector
    /// example:
    /// { 0, 1, 2,
    ///   3, 4, 5 }
    /// width = 3,
    /// pointer = (1, 1)
    /// y (1) * width (3) + x (1) => pos(4)
    /// vector[4] = 4
    template <typename number_type = int>
    extern number_type _vcreate_vu2d_delta_xy(int x, int y, int width);

    namespace Renderer {
        /// Gmeng class for Models. Models are any standalone object
        /// within a render instance that isn't treated as an entity.
        /// Models are part of a 'chunk' which make up a Level instance.
        class Model {
            private:
              Objects::coord get_pointXY(int pos);
            public:
              texture highlighted_texture = { Blob(0, 0, {}), false, "#$!_NONE" };
              /// Whether or not the model is highlighted. Gmeng
              /// internally draws a box around the model if this
              /// parameter is set to true. Useful for debugging.
              bool highlighted = false;
              /// Activity status of the model. If set to false,
              /// the model is treated as inactive and it will
              /// not be drawn in the renderscale. Any effects
              /// the model might have on collision and lighting
              /// will also not be taken into account during calculation.
              bool active = true;
              /// Width of the model. This should always be equal to
              /// `Model::texture.width`, as the texture is what's
              /// taken into account when drawing a Model.
              std::size_t width;
              /// Height of the model. This should always be equal to
              /// `Model::texture.height`, as the texture is what's
              /// taken into account when drawing a Model.
              std::size_t height;
              /// Size of the model. This should always be equal to
              /// `Model::width * Model::height`. Internally managed.
              std::size_t size;
              /// Position of the model within its parent `chunk`
              /// container. This is almost always equal to the
              /// model's position within the Level, but with
              /// levels that have multiple chunks, it's not guaranteed.
              drawpoint position;
              /// Name of the model. Useful for identifying specific
              /// models that may be using the same texture as another
              /// model within the same level.
              std::string name;
              /// Texture of the model. The texture controls the
              /// model's `width, height, size` values internally.
              /// This is the image that is drawn as a Model when
              /// `get_renderscale` accesses this model's parent chunk.
              Gmeng::texture texture;
              /// ID of the model.
              int id;

              GMENG_INIT_TYPE( highlighted, active, width, height,
                               size, position, name, texture, id );
              /// Resets the texture into the default null-layer with the same height
              /// as the previously attached texture.
              ///
              /// @since 4.1-glvl
              void reset_texture();

              const Gmeng::texture& get_sprite(bool force_highlight = false);

              /// Resizes the Model (and its texture)
              void resize( const std::size_t& new_width, const std::size_t& new_height );

              /// Resizes the Model (and its texture)
              void resize( const float& scale );

              /// Attaches a given texture to the model.
              ///
              /// @since 4.1-glvl
              void attach_texture(const Gmeng::texture& __t);

              /// Loads a texture from a file.
              ///
              /// @since 4.1-glvl
              void load_texture(std::string __tf);

              /// sets the border of the model.
              /// Models do not have borders by default.
              /// this method can be used for debug_render rendering modes.
              ///
              /// @since 10.4.0-d
              void set_border(color_t border_color);
        };
        

        extern int dp_distance(const drawpoint& a, const drawpoint& b);

        struct gnmdl {
            Model m;
            v_title txname;
        };

        /// parses a string.gmdl file to a gmdl
        gnmdl parse_gmdl(std::string _indx);        

        // compiles Renderer::Model objects into a Gmeng::Unit vector
        extern std::vector<Unit> draw_model(const Model& __m);

    };

    struct VectorView {
        std::map<int, std::vector<Gmeng::Unit>> vectors; std::size_t width = 1; std::size_t height = 1;
    };
    struct RenderBuffer {
        std::vector<Gmeng::Unit> units; Renderer::drawpoint pos; bool linear = false;
    };
    enum LinearRenderBufferPositionController {
        SIDE_RIGHT  = 0, SIDE_LEFT = 1
    };
    enum HorizontalRenderBufferPositionController {
        SIDE_BOTTOM = 0, SIDE_TOP  = 1
    };

    extern const Renderer::Model nomdl;

    template<typename v_type>
    struct vd_item {
        v_intl id; v_type data;
    };

    template<typename v_type>
    struct v_dictl {
        public:
            std::vector<Gmeng::vd_item<v_type>> values;
            /// returns size of the dictionary
            int size(bool diff = 0);
            /// returns value at the specified index of the dictionary
            v_type indx(v_intl pos);
            /// emplaces a value at the end of the dictionary
            void addb_l(v_type val);
            /// returns the vector of the dictionary
            std::vector<Gmeng::vd_item<v_type>>& v_getrelative();
    };

    extern Renderer::Model vd_find_model(v_dictl<Renderer::Model> dict, v_title name);
    extern texture& vd_find_texture(v_dictl<texture>& dict, v_title name);
    
    namespace vgm_defaults {
        extern v_dictl<Renderer::Model> vg_rdmodels;
        extern v_dictl<texture>         vg_textures;
    };

    /// reads a folder's VGM texture & model files into Gmeng::vgm_defaults::vg_rdmodels & vg_textures
    /// for ease-of-access to textures without having to read them from the disc.
    extern int _uread_into_vgm(const v_title& folder);

    /// writes values in Gmeng::vgm_defaults::vg_rdmodels & vg_textures into vectors v_mdls and v_txtrs
    extern void _uwritevgm_to_maps(std::map<v_title, Gmeng::Renderer::Model>& v_mdls, std::map<v_title, Gmeng::texture>& v_txtrs);

    /// Proximity Info (used for calculate_proximity() calls)
    struct Proximity_Info {
        unsigned int proximity;
        Renderer::viewpoint path;
    };

    /// Calculates the proximity between two objects
    /// given their position and size values.
    ///
    /// The calculated value is the gap between
    /// the first object and the second object.
    ///
    /// posA, sizeA for first object
    extern Proximity_Info calculate_proximity(
        const Renderer::drawpoint& posA, const Renderer::drawpoint& sizeA,
        const Renderer::drawpoint& posB, const Renderer::drawpoint& sizeB
    );

    /// Returns the shortest grid path with every immediate step needed
    /// to be taken to reach a viewpoint's end point from the start point.
    ///
    /// Based on Bresenam's line algorithm.
    extern std::vector<Renderer::drawpoint> shortest_path(const Renderer::viewpoint& view);


    /// chromatized chunks are rendered chunks of a level
    /// that share the same viewpoint.start.y coordinates.
    /// bound by level's base_template header width & height.
    /// example:
    /// level's base_template header: { width=9, height=2 }
    /// level->chunks: [render chunk (Gmeng::r_chunk)]
    ///   { "1", "2", "3",
    ///     "7", "8", "9",
    ///     "d", "e", "f", },
    ///
    ///   { "4", "5", "6",
    ///     "a", "b", "c",
    ///     "g", "h", "i", }
    /// parsed into _vp2d_scalar_size and _vconcatenate_lvl_chunks(&p)
    /// chromatized_chunks: [chormatized chunk (__CHROMATIZED_CHUNK_CONTROLLER_VIEWPOINT__)]
    ///   { "1", "2", "3", "4", "5", "6", "7", "8", "9" },
    ///   { "a", "b", "c", "d", "e", "f", "g", "h", "i" }
    /// width = 9units, height: 2rows
    struct __CHROMATIZED_CHUNK_CONTROLLER_VIEWPOINT__ {
        std::vector<Unit> data;
        Renderer::viewpoint vp;
    };

    /// returns the width of a viewpoint
    extern int _cc1d_scalar_size(Renderer::viewpoint vp);    

    /// mirror: reverses a vector
    template<typename __vtype__>
    extern void mirror(std::vector<__vtype__>& __v, int cc = 0);    

    /// mirror: returns a reversed version of a vector
    template<typename __vtype__>
    extern std::vector<__vtype__> mirror(std::vector<__vtype__> __v, bool returntyped = true);

    /// concatenates all chunks within a level, according for a 2d space
    /// of which boundaries are set by the level's header base_template.
    /// returned as a vector<vector<Unit>>, which consist of Gmeng::Unit rows
    /// example:
    /// { "xxxxxxx",
    ///   "xxxxxxx",
    ///   "xxxoxxx", } -> VECTOR(VECTOR<Unit>, 3);
    extern std::vector<std::vector<Unit>> _vconcatenate_lvl_chunks(Level& lvl, bool _cubic_render = true, TRACEFUNC);

    /// Returns the renderscale of a level.
    ///
    /// @before {
    ///     This method concatenates all chunks in the level according to their scalar viewpoints
    ///     to prevent cumilative-index drawing which may result in a wrong image.
    ///     PERFORMANCE-HEAVY especially in bigger levels.
    ///     DO NOT! use this method in loops. It will cause lag and stutterring. This is not due to a
    ///     poorly optimized method. It is intended to be called after loading a gm4_0 framework glvl.
    ///     It will trace_chunk_vector(&lvl) and render it completely. It should be used as a base image.
    ///     Only use this when a chunk's models, textures are updated. Player and entity movement are handled
    ///     automatically, so there is no need to put this method in any loops.
    /// }
    ///
    /// @after {
    ///     Since 10.0.0, this method is sufficiently optimized. Draw times will not be affected,
    ///     and it is safe to use inside event loops. to draw the screen.
    /// }
    extern std::vector<std::vector<Unit>> _vget_renderscale2dpartial_scalar(Level& level_t, bool cubic_render = true);

    /// Returns the renderscale of a level.
    ///
    /// @before {
    ///     This method concatenates all chunks in the level according to their scalar viewpoints
    ///     to prevent cumilative-index drawing which may result in a wrong image.
    ///     PERFORMANCE-HEAVY especially in bigger levels.
    ///     DO NOT! use this method in loops. It will cause lag and stutterring. This is not due to a
    ///     poorly optimized method. It is intended to be called after loading a gm4_0 framework glvl.
    ///     It will trace_chunk_vector(&lvl) and render it completely. It should be used as a base image.
    ///     Only use this when a chunk's models, textures are updated. Player and entity movement are handled
    ///     automatically, so there is no need to put this method in any loops.
    /// }
    ///
    /// @after {
    ///     Since 10.0.0, this method is sufficiently optimized. Draw times will not be affected,
    ///     and it is safe to use inside event loops. to draw the screen.
    /// }
    extern std::vector<std::vector<Unit>> get_renderscale(Level& level_t, bool cubic_render = true);

    /// Draws a line of units
    /// vector<Unit> -> "[][][][]"
    extern std::string draw_line_units(std::vector<Unit> line);

    /// returns the camera of the current level, with drawpoints included in level_t->display.viewpoint
    /// This utility is a replacement for cam_view_generate, chunk_view and spliced_chunk systems.
    ///
    /// @since 7.2.0 @continuous 8.1.0 @since {cubic_render} 8.2.0 @since {debug_render} 10.4.0
    ///
    /// use `get_renderscale` to receive a concatenated vector<Unit> object to be parsed by this method.
    /// Give the output of this method to `emplace_lvl_camera` and it will set up your camera.
    ///
    /// This utility supports chunk splicing, cubic rendering (with 1x1 square, smaller units
    /// instead of 2x1 rectangle units) for much more size and way better visuals, debug rendering,
    /// continuous and seemless chunk movement, viewpoints.
    extern std::string get_lvl_view(Level& level_t, std::vector<std::vector<Unit>>& trimmed_units, bool cubic_render = true);

    /// emplaces unit data to the camera.
    /// This method requires a Gmeng::Level& and a viable rendered_viewpoint as parameters.
    /// to create a rendered_viewpoint use Gmeng::_vget_renderscale2dpartial_scalar(level) and put its return value to get_lvl_view(level, value)
    extern void emplace_lvl_camera(Level& level_t, std::string& cam_data);
};

namespace Gmeng {
    /// Default Event Hooks & handlers of Gmeng.
    /// Individual hooks defined in this dictionary
    /// can be disabled by external event hooks via
    /// (EventInfo*) info->prevent_default(true).
    extern std::unordered_map<Event, std::vector<EventHook>> DEFAULT_EVENT_HOOKS;
};

std::deque<std::string> gmeng_log_get_last_lines(int n = 5);

/// variables in-game usable in the developer console/
/// these variables are only integers (currently)
///
/// variables can point to other variables
/// using the '*' token at the start of the variable name
/// and the index of the variable it is pointing to/
///
/// example:
///
/// { "errorlevel", 0 },
/// { "hello_world", 0 },
/// { "*hw", 1 }, // but the value of 'hw' would point to 'hello_world' as hello world is variables[1].
/// hence the hw's value of 1.
extern std::vector< std::tuple<std::string, int> > variables;

#ifndef GMENG_COMPILING_SCRIPT
// EXTERNS FOR NOBLE SCRIPTS
// all NOBLE scripts must use these methods for their functionality.
// scripts can not be executed from different entrypoint functions,
// but can have multiple of the gmeng-provided entries for different
// types of execution such as a command based initialization entry
// and a periodic control entry.


// NOBLE SCRIPT HELPER FOR GMENG
//
// Command-based, executed once
extern "C" int gmeng_script_command( Gmeng::EventLoop* );
// NOBLE SCRIPT HELPER FOR GMENG
//
// Periodic, executed with event UPDATE
extern "C" int gmeng_script_periodic( Gmeng::EventLoop* );

#endif


template <typename T>
struct function_return_type;

template <typename R, typename... Args>
struct function_return_type<R(Args...)> {
    using type = R;
};

template <typename T>
using function_return_type_t = typename function_return_type<T>::type;


namespace Gmeng {
    template<typename T>
    class CommandSet {
      public:
        struct Command {
            std::string name;
            std::function<T> executor;
        };

        std::map<std::string, Command> commands;

        template<typename... Args>
        function_return_type_t<T> run( std::string command, Args&&... args);

        CommandSet() = default;
        CommandSet( std::vector<std::tuple<std::string, std::function<T>>> _commands ) {
            for ( auto command : _commands ) {
                Command new_cmd{ std::get<0>(command), std::get<1>(command) };
                this->commands[ new_cmd.name ] = new_cmd;
            };
        };
    };
};



#include <asio/asio.hpp>

namespace Gmeng {
    struct network_message {
        int id;
        int response_of = -1;
        std::string content;
    };

    /// serializes a network message
    extern std::string network_serialize_message( const std::string& message, int message_id, int response_of = -1 );

    extern network_message network_parse_message( const std::string& message );

    class GameServer;
    extern Gmeng::GameServer* game_server;

    /// Since 13.2.0: Gmeng Networking with Asio
    /// Game Server for Multiplayer game instances.
    class GameServer {
      public:
        /// Client ID type ( aka unsigned int )
        using client_id = unsigned int;
        /// Client instance for the Server.
        /// Contains its client_id, IP address,
        /// connection date and num_packets.
        struct ServerClient : public std::enable_shared_from_this<ServerClient> {
            /// Socket with the client.
            std::shared_ptr<asio::ip::tcp::socket> socket;
            /// Number of packets sent by the client.
            unsigned int num_packets = 0;
            /// Time (in date milliseconds) of connection by the client.
            long long connected_at = GET_TIME();
            /// ID of the client. Unique for all clients.
            /// See `g_mkid()` for ID creation logic.
            client_id id = -1;
            /// IP address (in string form) of the client.
            std::string address = "unknown";

            ServerClient() = default;
            ServerClient( decltype(socket) _socket, decltype(num_packets) _packets,
                          decltype(connected_at) _connected_at, decltype(id) _id,
                          decltype(address) _address) : socket(_socket), num_packets(_packets),
                          connected_at( _connected_at ), id( _id ), address(_address) {};
            /// Sends a message to the client.
            /// set `response_of` to a valid message sent by the client to send a response,
            /// instead of a brand new message. This will fire the `response` callback of that message (client-side).
            void send_message( const std::string& message, int response_of = -1, int id = g_mkid() );

            std::string info_string();
        };
        /// Message from a connected client.
        /// Contains client information as well its message.
        struct ClientMessage {
            /// ID of the message.
            unsigned int id = g_mkid();
            /// Client information.
            ServerClient* client;
            /// contents of the message.
            std::string content = "";
            /// method to respond to the message.
            void respond( std::string reply_text );
        };
        /// TCP server port of the server. Gmeng internally sets this to 7388.
        /// See `GMENG_DEFAULT_PORT` in `networking.cc` for more information.
        unsigned int port = 7388;
        /// maximum number of allowed clients into
        /// the GameServer instance. default is 10.
        unsigned int max_clients = 8;
        /// Asio IO context of the server.
        /// Handles connectivity as well as the socket streaming.
        asio::io_context context;
        /// Asio TCP connection acceptor of the server.
        /// Accepts incoming clients and manages them.
        asio::ip::tcp::acceptor acceptor;
        /// Whether the server is running. When set to false,
        /// the server will close all active connections and exit.
        bool active = true;
        /// default in-game chat stream.
        std::stringstream chat;
        /// Clients connected to the server.
        std::map<client_id, ServerClient> clients;

        GameServer( int _port = 7388, int _max_clients = 10 )
        : port ( _port ), max_clients( _max_clients ),
        acceptor( context, asio::ip::tcp::endpoint( asio::ip::tcp::v4(), port ) ) {
            __functree_call__( Gmeng::GameServer::constructor );
            /// Multiple server instances in a single
            /// program is not supported, error out and exit.
            if ( Gmeng::game_server != nullptr ) {
                gm_log("gmeng::game_server is not nullptr, exitting...");
                throw std::runtime_error("a Gmeng::GameServer instance is already present.");
            };

            gm_log("created a game server instance for port "$(this->port)".");
            /// set the current server to this instance
            Gmeng::game_server = this;
        };

        /// Launches the server. This will create a while loop
        /// with a callback to the parameter `user_callback`. Best
        /// to run in its own thread, and use the main program
        /// thread only for the game updates.
        void run( const std::function<void( GameServer*, ClientMessage& )>& user_callback );
    };

    struct ServerConnection;

    namespace network_protocol {
        using std::string, std::vector;
        extern CommandSet<int(ServerConnection*, vector<string>)> commands;
    };

    struct ServerConnection {
        std::shared_ptr<asio::io_context> context;
        std::shared_ptr<asio::ip::tcp::socket> socket;
        bool connected = false;
        unsigned int client_id = 0;

        std::function<void(network_message&)> message_handler = nullptr;


        /// global message handler. this function will handle all messages received
        /// by the client. this includes responses to messages sent by the client as well.
        void handle_message( decltype(message_handler) _message_handler );

        std::map<int, decltype(message_handler)> response_callbacks;

        /// Sends a message instantly. Doesn't block.
        /// If on_response is provided, it fires the next time the server sends data.
        void write(const std::string& message, decltype(message_handler) on_response = nullptr);

        /// Handles Gmeng websocket data protocol messages.
        void handle_protocol( const network_message& message );

        /// creates a message loop. This is a blocking function, so
        /// it is recommended to run this method in a seperate thread.
        /// See `Gmeng::_ucreate_thread` for information about threading with Gmeng.
        void poll();

        void disconnect();
        
    };

    /// Since 13.2.0: Gmeng Networking with Asio
    /// Game Client for Multiplayer game instances.
    class GameClient {
      public:
        /// Connects to a Gmeng GameServer instance at the provided ip address and port.
        /// WARNING: IP_ADDRESS MUST BE AN IP ADDRESS! DOMAINS ARE NOT ACCEPTED.
        static ServerConnection connect(const std::string& ip_address, unsigned int port = 7388, TRACEFUNC);
    };
};


/// Runs the developer console for Gmeng.
/// This utility is for Terminal-mode only.
/// It should not be used with Gmeng SDL.
///
/// Kitty Terminal Graphics Protocol will be used if available.
extern void gmeng_dev_console(Gmeng::EventLoop* ev, Gmeng::EventInfo* info);

/// Checks and displays warnings for various terminal and shell programs.
/// This function is called by `do_event_loop` in the beginning of a game
/// event loop. It is not required to implicitly call it.
extern void check_terminal_and_shell( Gmeng::EventLoop* ev );

#ifndef _WIN32
#if GMENG_SDL != true

extern void gmeng_handle_sigint(int);
extern void gmeng_handle_sigwinch(int);
extern void gmeng_write_stack_trace();
extern void gmeng_handle_sigsegv(int signal);
extern void gmeng_handle_buserror(int signal);

enum class cursor_type {
    DEFAULT,        // DEFAULT
    POINTER,        // CLICKING HAND
    CROSSHAIR,      // PRECISION CROSS
    TEXT,           // TEXT I-BEAM
    WAIT,           // LOADING / SPINNING WHEEL
    HELP,           // ARROW WITH QUESTION MARK
    NOTALLOWED,     // CIRCLE WITH A SLASH
    ERESIZE,        // HORIZONTAL RESIZE ARROW
    NERESIZE,
    NWRESIZE,
    NRESIZE,
    SERESIZE,
    SWRESIZE,
    SRESIZE,
    WRESIZE,
    EWRESIZE,       // HORIZONTAL RESIZE ARROWS
    NSRESIZE,       // VERTICAL RESIZE ARROWS
    NESWRESIZE,
    NWSERESIZE,
    ZOOM_IN,
    ZOOM_OUT,
    ALIAS,
    COPY,
    NODROP,
    GRAB,
    GRABBING,
    CELL,           // PRECISION CELL
    VERTICAL_TEXT,  // VERTICAL TEXT
    MOVE,           // MOVE VECTOR

};

/// Gmeng : Function to set the mouse cursor icon using the enum
extern void gmeng_set_cursor(const cursor_type& shape);

namespace Gmeng {
    extern void set_pointer_icon(const cursor_type& shape);
    extern void reset_pointer_icon();
};

/// Define the Implementation if its not set to default (true)
#ifndef GMENG_IMGUI_IMPLEMENTATION
    #define GMENG_IMGUI_IMPLEMENTATION 1
#endif


/// Dear ImGui, Implementation for Gmeng
/// (Supported only for applications using the Kitty Terminal Graphics Protocol)

#if GMENG_IMGUI_IMPLEMENTATION == 1

struct rgb_texture {
    uint8_t* pixels;
    int width;
    int height;
};

#endif

namespace Gmeng::Renderer {
        /// 'Skybox' implementation in Gmeng. Basically, this 'base'
        /// is the underlying texture of a `Gmeng::Level` instance.
        /// All chunks within a level are drawn on top of this texture
        /// and this also defines the boundaries of a Level.
        struct LevelBase {
            /// texture of the level base (skybox)
            Gmeng::texture lvl_template;
            /// Width of the level base as well as the maximum width
            /// of the parent level instance.
            std::size_t width;
            /// Height of the level base as well as the maximum height
            /// of the parent level instance.
            std::size_t height;

            void resize( float scale );

            void resize( const std::size_t& n_width, const std::size_t& n_height );

            GMENG_INIT_TYPE( lvl_template, width, height );
        };
};

enum SIDE {
    TOP,    // Fades downwards from the top edge
    BOTTOM, // Fades upwards from the bottom edge
    LEFT,   // Fades rightwards from the left edge
    RIGHT,  // Fades leftwards from the right edge
};

namespace Gmeng {
    class FrameProfiler {
      private:
        using clock = std::chrono::high_resolution_clock;
        using timepoint = std::chrono::time_point<clock>;

        timepoint start_time;

      public:
        std::chrono::duration<float> duration = std::chrono::milliseconds(0);
        /// Called before frame creation
        void start_profile();

        /// Called after frame creation
        float end_profile();
    };
};
template<typename T>
struct subarray_2d {
  private:
    T* main_array = nullptr;
  public:
    Gmeng::Renderer::viewpoint main_size;
    Gmeng::Renderer::viewpoint view;

    size_t view_size;
    size_t stride;
    size_t view_stride;

    subarray_2d(T* arr, const Gmeng::Renderer::viewpoint& _main_size, const Gmeng::Renderer::viewpoint& _box);
    subarray_2d() = default;

    bool exists();

    T& operator[](size_t index);

    const Gmeng::Renderer::drawpoint point(int index) const;

    size_t operator()(size_t index);

    T& at( size_t index );
};

namespace Gmeng::Renderer {
        extern void subimage_copy(
            const uint8_t* src_image,
            int src_stride_width,
            const Gmeng::Renderer::viewpoint& vp,
            uint8_t* dest_image
        );

        // type decl
        class Display {
            public:
                /// Main camera of the display. this utility  is how a level object is rendered or seen,
                /// but the camera does not have positions since it is not an object. The camera renders units
                /// and draws to the screen only. Rendering is handled  by get_renderscale(), get_lvl_view()
                /// and emplace_lvl_camera() on the terminal and by get_renderscale() by itself on SDL mode.
                Camera<0, 0> camera;
                /// The viewpoint of the display. this decides the part of the level that will be visible
                /// on the camera. This is not tied to the screen resolution, so it must be synced accordingly.
                /// This option is not serialized, so it must be set at the beginning of every game instance.
                viewpoint viewpoint;
                /// The width of the display. Not synced with the viewpoint.
                /// Setting this variable by itself is not recommended. Use
                /// set_resolution() instead, as it will also update the camera.
                std::size_t width;
                /// The maximum width that the display can have. Set by the
                /// Level instance managing this display. The value will be equal
                /// to the level's base_texture width.
                std::size_t max_width;
                /// The height of the display. Not synced with the viewpoint.
                /// Setting this variable by itself is not recommended. Use
                /// set_resolution() instead, as it will also update the camera.
                std::size_t height;
                /// THe maximum height that the display can have. Set by the
                /// Level instance managing this display. The value will be equal
                /// to the level's base_texture height.
                std::size_t max_height;
                /// Units rendered on the display. used for camera syncing internally.
                /// This vector will usually be empty, it is not for usage in games.
                /// use either Camera::display_map::unitmap or Level::renderscale.
                std::vector<Unit> rendered_units;
                /// Frame Profiler of the display, calculates frame and draw time
                /// for the get_lvl_view and get_renderscale functions. Used for
                /// sending a FIXED_UPDATE event.
                FrameProfiler profiler;
                /// FPS of the FIXED_UPDATE event call.
                /// wrn: this object is not serialized into a .glvl file, must
                /// be set from the game code. The default value is 120FPS.
                unsigned int fixed_update_fps = 60;

                GMENG_INIT_TYPE( camera, viewpoint, width, max_width,
                                 height, max_height, rendered_units, profiler );

                /// enables / disables cursor visibility on the terminal.
                void set_cursor_visibility(bool state);

                /// Sets the resolution of the display.
                /// This does not affect the viewpoint, only the width/height
                /// of the display and camera. the size of the viewpoint may
                /// cause problems if the resolution and it are not synced.
                void set_resolution(std::size_t width, std::size_t height);

                /// Generates and draws the newest frame to the screen.
                /// Default draw for both engines (SDL & Terminal-based)
                ///
                /// TODO: Also add SDL default draw support (currently does nothing)
                void refresh(Gmeng::Level*, std::function<void(Gmeng::Level*)>);

                struct DisplayController {
                    const Gmeng::Renderer::viewpoint box;
                    uint8_t* original;
                    subarray_2d<uint8_t> content;

                    DisplayController(const Renderer::viewpoint& _box);

                    void backdrop( const Renderer::drawpoint& offset, std::function<void(DisplayController*)> controller, bool) const;

                     DisplayController* drop_shadow( const uint32_t& color, float opacity = 0.5f, size_t offset_x = 1, size_t offset_y = 1 );

                     DisplayController* blur(float radius);

                     DisplayController* blend( float blend_amount = 0.5f, SIDE dir = SIDE::BOTTOM );

                     uint8_t brightness_extract( uint8_t v, float brightness = 1.0f ) const;

                     DisplayController* brightness( float brightness );
                };
                /// filters and other options for a given viewpoint within the display
                 void box(const Gmeng::Renderer::viewpoint& box,
                                std::function<void(DisplayController*)> controller);

                /// Moves the display's viewpoint to the specified new viewpoint.
                void move_to(const Gmeng::Renderer::viewpoint& __vp);

                /// Moves the display's viewpoint by the specified X and Y
                /// values. The values are added to the existing viewpoint.
                ///
                /// THIS DOES NOT OVERRIDE THE VIEWPOINT! It merely will
                /// add the specified X and Y deltas to the viewpoint.
                ///
                /// For example:
                ///
                /// using move_viewpoint_by(1, 1) will move a viewpoint of
                /// values: { start = { 50, 50 }, end = { 75, 75 } } to
                /// moved: { start = { 51, 51, }, end = { 76, 76 } }.
                ///
                /// The values can be negative as well.
                ///
                /// TODO: move this to Gmeng::Renderer::viewpoint::move_by.
                void move_viewpoint_by(int by_x = 0, int by_y = 0);
        };
};

template <typename T>
class GridMap {
public:
    using EntityID = uint32_t;
    using EntityPtr = std::shared_ptr<T>;

private:
    struct EntityData {
        float x = 0, y = 0;
        float proximity = 0;
        EntityPtr ptr = nullptr;
        int cellX = 0, cellY = 0;
        bool initialized = false;
    };

    std::unordered_map<EntityID, EntityData> entities;
    std::unordered_map<int64_t, std::vector<EntityID>> grid;

    int cellSize;

    int64_t cell_key(int cx, int cy) const; 

public:
    GridMap(int cell_size = 32);

    // ----------------------------------------------------------
    // REMOVE ENTITY
    // ----------------------------------------------------------
    void remove_entity(EntityID id);

    // ----------------------------------------------------------
    // UPDATE ENTITY (CALL EVERY FRAME)
    // ----------------------------------------------------------
    void update_entity(
        EntityID id,
        float x, float y,
        float interaction_proximity,
        EntityPtr ptr
    ); 

    // ----------------------------------------------------------
    // GET NEARBY ENTITIES BASED ON PROXIMITY
    // ----------------------------------------------------------
    std::vector<EntityPtr> get_nearby_entities(EntityID id); 

    // ----------------------------------------------------------
    // BUILD INTERACTION MAP: ID → vector<shared_ptr<T>>
    // ----------------------------------------------------------
    std::unordered_map<EntityPtr, std::vector<EntityPtr>> build_interaction_map(); 
};

namespace Gmeng {
    // chunk of a `Gmeng::Level` instance. similar to how minecraft handles chunks.
    // in `Gmeng::Level::render_chunk()` a `std::vector<Gmeng::r_chunk> chunks` is loaded.
    // these chunks contain { Gmeng::Renderer::viewpoint vp; } metadata. The chunk's
    // `std::vector<Gmeng::Renderer::Model>` models object is recieved and compiled into a unitmap.
    struct chunk {
        /// Viewpoint of the chunk. Defines the part of the Level that this
        /// chunk is responsible for containing Model and Entity data for.
        Renderer::viewpoint vp;
        /// Models within the chunk.
        std::vector<Renderer::Model> models;
        /// Container for the rendered units of the chunk.
        /// Used by Level::v_render_chunk or Level::render_chunk,
        /// stores default initialization of the vector.
        std::vector<Unit> rendered_units;

        GMENG_INIT_TYPE( vp, models, rendered_units );
    };

    // levelinfo (parsed into Gmeng::Level::load_level after Gmeng::parse_glvl())
    struct LevelInfo {
        Renderer::LevelBase base;
        std::string name;
        std::vector<int> display_res;
        std::vector<chunk> chunks;
        std::string description;
    };

    /// Level type of Gmeng.
    /// This is more a 'level manager' than a level itself,
    /// but nontheless it holds the display and camera that
    /// initialize the cameraview and the level object.
    ///
    /// @since 4.1_glvl
    /// @continuous_build 8.0.0
    class Level {
        private:
            // compiles a chunk into a vector<Unit> unitmap for a Camera instance to render.
            // this method can also be used externally to render any chunk, it does not require
            // for the chunk to  be a member of the Level class.
            //
            // currently used in get_renderscale, get_lvl_view & emplace_lvl_camera.
            //
            // Does not render entities!
            std::vector<Unit> render_chunk(chunk& chunk);

            // retrieves the chunk at the given id from
            // level->chunks.at(id) and returns it.
            //
            // @since 4.1-glvl
            chunk& get_chunk(int id);

            // Replaces the chunk at level->chunks.at(id)
            // to the given chunk.
            //
            // @since 4.1-glvl
            void set_chunk(int id, chunk& chunk);

            /// @deprecated We don't use Objects::G_Player as the player anymore
            ///
            /// calculates the viewpoint of the camera depending on the player object,
            /// for 4.1-glvl framework applications only. Kept for backwards compatibility
            Renderer::viewpoint calculate_camera_viewpoint();
            /// for displacement_cache
            struct displacement_cache_entry {
                Renderer::Model* model; std::vector<Renderer::drawpoint> displacements;
            };
        public:
            /// stores every displacement of the level's models
            /// mapped by the models' ids. Used for shadows &
            /// ray tracing in Gmeng::get_renderscale()
            std::map< int, displacement_cache_entry > displacement_cache;
            /// Map of light sources for each LightSource entity,
            /// mapped by LightSource->entity_id. cached & generated
            /// by Gmeng::get_renderscale().
            std::map< int, std::shared_ptr<LightSource> > light_sources;
            /// Contains a map of illumination and lighting info
            /// for each drawpoint in the level. The minimum light
            /// intensity is set by the camera (level.display.camera)
            /// or (Gmeng::Camera). The minimum light intensity is a
            /// modifier and can be serialized and saved to level
            /// files or any stdc++ outstream.
            ///
            /// Lighting cache is initialized by Gmeng::get_renderscale and
            /// stored in Gmeng::Level::lighting_cache, but lighting states
            /// for every drawpoint is calculated by Gmeng::Camera. via
            /// Gmeng::Camera::calculate_illumination( color32_t color,
            /// int light_intensity_max, int distance, int min_intensity).
            ///
            /// Lighting can be disabled by setting the modifier on
            /// Gmeng::Camera::modifiers( "lighting", 0 ) with the method
            /// Gmeng::Camera::set_modifier( "lighting", 0 ).
            ///
            /// The minimum lighting level (min_intensity) can also be set
            /// with Gmeng::Camera::set_modifier( "lighting_floor", 0 ).
            std::unordered_map< uint32_t, LightingState > lighting_cache;
            /// Contains a copy of each model included in the level.
            /// Does not contain a copy of each individual model's derivatories,
            /// only models that weren't initialized before.
            std::map<int, Renderer::Model*> modelmap;
            /// Available renderscale for the level.
            /// Not updated internally, is set by get_renderscale(),
            /// _vconcatenate_lvl_chunks() or _vget_renderscale2dpartial_scalar().
            ///
            /// For ease-of-access in preview / debugging utilities without having to
            /// recompile the renderscale from the start by using this shared object.
            std::vector<std::vector<Unit>> renderscale;
            /// Base texture for the Level.
            /// This object is treated as a skybox.
            ///
            /// It covers all chunks and is the first layer
            /// (so the backmost layer) in every chunk as well.
            Renderer::LevelBase base;
            /// Display for the level. Contains the Camera
            /// and the display properties like the viewpoint,
            /// camera width, height and resolution as well
            /// as the camera ModifierList.
            Renderer::Display display;
            /// Chunk vector of the level. Contains every
            /// chunk registered to the level.
            ///
            /// Chunks contain models and
            /// are spliced with get_renderscale when
            /// drawing the level display.
            std::vector<chunk> chunks;
            /// Entities in the level. Contains
            /// every entity registered t4o the level.
            ///
            /// Entities are not bound with chunks.
            std::vector<std::shared_ptr<EntityBase>> entities;
            /// The Level's interaction grid map.
            /// The level is divided into 32x32 cells that
            /// contain buckets of entities, which makes
            /// checking for proximity between entities
            /// easier
            GridMap<Gmeng::EntityBase> entity_grid;
            /// Description of the level.
            ///
            /// can be set with 4.1 text-based GLVL files
            /// with desc="<description>" on the file headers.
            std::string desc;
            /// Name of the level.
            ///
            /// can be set by 4.1 text-based GLVL files
            /// with name="<name>" on the file headers.
            std::string name;

            /// Caching modifier for the Level::base object,
            /// the level template is only drawn once (unless
            /// a `redo_levelbase` command is called) so we
            /// cache it in _vconcatenate_lvl_chunks().
            bool base_init = false;
            /// Cached rows of the base object.
            std::vector<std::vector<Unit>> CACHED_BASE;

            GMENG_INIT_TYPE( displacement_cache, light_sources, lighting_cache,
                             modelmap, renderscale, base, display, chunks,
                             entities, entity_grid, desc, name, base_init );

            /// Loads a chunk from the given param,
            /// and returns the id that the chunk is assigned to.
            ///
            /// can be called with get_chunk(id) or level->chunks.at(id).
            /// can be cast to void, (void)load_chunk(...). The id value
            /// is not crucial to keep as long as no further changes will
            /// be made to the chunk after its addition.
            ///
            /// @since 4.1-glvl
            int load_chunk(chunk chunk);

            /// Returns the rendered output of a chunk
            /// at the given id.
            ///
            /// @since 4.1-glvl
            std::vector<Unit> get_rendered_chunk(int id);

            /// Renders a given chunk with the internal chunk
            /// renderer for the level. does not add to chunk map.
            ///
            /// @since 7.0.0-d
            std::vector<Unit> v_render_chunk(chunk& chunk);

            /// Returns every model that covers a specified drawpoint.
            /// For example,
            /// if find_models_at( 10, 10 ) is called and there is;
            ///     Model( { 0,0 -> 15, 15 } ),
            ///     Model( { 5,5 -> 10, 10 } ),
            /// these models will be returned as pointers.
            ///
            /// This method, by default, will ignore a model if the
            /// search drawpoint is covered by it, but is a transparent unit.
            /// Set `including_transparent` to true to include them.
            ///
            /// This method ignores inactive models.
            std::vector<Renderer::Model*> find_models_at( Renderer::drawpoint dp, bool including_transparent = false );
    };    
};

/// Terminal-Based Event Loop handler. Default-provided by Gmeng.
/// Since 8.0.0, this is the correct way for a Gmeng game to be ran.
/// `do_event_loop` will automatically check the environment in which
/// Gmeng is being run.
///
/// See `gmeng.org/do_event_loop` for more information.
extern int do_event_loop(Gmeng::EventLoop* ev);

namespace Gmeng::Util {
    /// @since 10.1.0
    /// TODO: does not support cubic rendering, add option
    extern std::string draw_texture_string(Gmeng::texture txt);

    /// fills a texture with a single color
    extern void texture_fill(texture& tx, color_t color);

    /// replaces the colors of a texture (color1 -> color2)
    extern void texture_replace_color(texture& tx, color32_t color1, color32_t color2);
    extern int calculate_proximity(Renderer::drawpoint pos1, Renderer::drawpoint pos2);
    extern void level_set_skybox(Level* lvl, texture& tx);

    extern Renderer::Model model_create(int width, int height, Renderer::drawpoint pos);
    extern Renderer::Model model_from_txtr(texture& texture, Renderer::drawpoint pos);    

    extern texture default_texture_search(std::string name);
    extern Renderer::Model default_model_search(std::string name);

    extern void viewpoint_go_right(Renderer::viewpoint& vp, int times);
    extern void viewpoint_go_left(Renderer::viewpoint& vp, int times);
    extern void viewpoint_go_up(Renderer::viewpoint& vp, int times);
    extern void viewpoint_go_down(Renderer::viewpoint& vp, int times);
};

/// Serializes a Unit object, and writes it to a stream.
extern void serialize_unit(const Gmeng::Unit& unit, std::ostream& stream);
/// Reads a Unit object from a stream.
extern void deserialize_unit(Gmeng::Unit& unit, std::istream& stream);

/// Writes a Texture object to a stream.
extern void serialize_texture(const Gmeng::texture& tex, std::ostream& stream);
/// Reads a Texture object from a stream.
extern void deserialize_texture(Gmeng::texture& tex, std::istream& stream);

/// Writes a Modifier object to a stream.
extern void serialize_modifier(const Gmeng::modifier& mod, std::ostream& out);
/// Reads a Modifier object from a stream.
extern void deserialize_modifier(Gmeng::modifier& mod, std::istream& in);

/// Writes a modifier list object to a stream.
extern void serialize_modifier_list(const Gmeng::ModifierList& list, std::ostream& out);
/// Reads a modifier list object from a stream.
extern void deserialize_modifier_list(Gmeng::ModifierList& list, std::istream& in);

/// returns the name of a variable's type
template<typename T>
std::string getTypeName(const T& var);

extern void serialize_drawpoint(const Gmeng::Renderer::drawpoint& dp, std::ostream& out);
extern void deserialize_drawpoint(Gmeng::Renderer::drawpoint& dp, std::istream& in);

extern void serialize_viewpoint(const Gmeng::Renderer::viewpoint& vp, std::ostream& out);
extern void deserialize_viewpoint(Gmeng::Renderer::viewpoint& vp, std::istream& in);

/// Writes a Model object to a stream.
extern void serialize_model(const Gmeng::Renderer::Model& model, std::ostream& out);
/// Reads a Model object from a stream.
extern void deserialize_model(Gmeng::Renderer::Model& model, std::istream& in);

/// Writes a Chunk object to a stream.
extern void serialize_chunk(const Gmeng::chunk& ch, std::ostream& out);
/// Reads a Chunk object from a stream.
extern void deserialize_chunk(Gmeng::chunk& ch, std::istream& in);

extern void serialize_entity(const std::shared_ptr<Gmeng::EntityBase>& entity, std::ostream& out);
extern std::shared_ptr<Gmeng::EntityBase> deserialize_entity(std::istream& in);

extern void serialize_level_base(const Gmeng::Renderer::LevelBase& level, std::ostream& out);
extern void deserialize_level_base(Gmeng::Renderer::LevelBase& level, std::istream& in);

extern float versionToFloat(const std::string& version);

extern void serialize_level(const Gmeng::Level& level, std::ostream& out);
extern void deserialize_level(Gmeng::Level& level, std::istream& in);

/// Writes out data of a Level object to a file, can be reloaded with read_level_data().
extern void write_level_data(std::string filename, Gmeng::Level& level);
/// Reads data of a Level object from a file.
extern void read_level_data(std::string filename, Gmeng::Level& level);

/// Writes out data of a Level object to a stream, which can later be reloaded with read_level_data().
extern void write_level_data(std::ostream& out, Gmeng::Level& level);
/// Reads data of a Level object from a stream.
extern void read_level_data(std::istream& inf, Gmeng::Level& level);

#define GMENG_SERIALIZATION_INIT 1

#endif

#endif

#endif
