#pragma once
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


/// base object types
#include "src/objects.cpp"
/// class initialization
#include "types/camera.h"
#include "utils/setup_class.h"
/// Gmeng::global utility
#include "utils/global.h"
/// Gmeng::Assert utility
#include "utils/assert.h"
/// gm_log() logging utility
#include "utils/log.h"
/// terminal size utility
#include "utils/ioctl.h"
/// SGR-Pixel mouse reporting utility
#include "utils/sgr_pixel.h"

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


#include "utils/tracefunc.h"


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


#ifdef __GMENG_OBJECTINIT__


#include "utils/functree.h"
#include "utils/kitty.h"
#include "types/modifier.h"


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

#include "utils/repeat.h"

/// Gmeng : returns the hex value of an integer
static std::string _uconv_1ihx(int value) {
    //__functree_call__(_conv_1ihx);
    std::stringstream stream;
    stream << "0x" << std::hex << value;
    return stream.str();
}

#include "types/vector.h"
#include "types/color32.h"
#include "types/unit.h"
#include "types/texture.h"

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

#include "src/version.h"

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
    /// Entire logs of the game, engine, dependencies.
    static std::stringstream completelog;





    /// type decl.
    class Level;

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

#include "utils/ansi.h"


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
inline Gmeng::color32_t Gmeng::color32_from_uint32(uint32_t rgb) {
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

#include "utils/colorformat.h"



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
    static std::thread create_thread(const std::function<void()>& func) {
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




static void set_terminal_size(int width, int height) {
    // The ANSI sequence to resize the window is \033[8;{height};{width}t
    std::cout << "\033[8;" << height << ";" << width << "t" << std::flush;
}

static void resize_terminal(int columns, int rows, bool force = false) {
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
    else if ( term != nullptr && std::string(term) == ("xterm-kitty") ) {
        if (force) kitty_resize_window(columns,rows);
    };
}

#include "utils/warning.h"


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
static bool g_install_font(const std::string& fontFilePath) {
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
static bool gmeng_macos_terminal_setup(int argc, char* argv[], TRACEFUNC) {
    __annot__(gmeng_macos_terminal_setup, "sets up Gmeng for MacOS & Apple Terminal");
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

#include "utils/startup.h"

#define __GMENG_INIT__ true /// initialized first because the source files check this value before initialization

/// Gmeng LUA scripts utility
#include "utils/scripts.h"

#include "src/textures.cpp"
#include "utils/text.h"
#include "src/gmeng.cpp"

#if _WIN32 == false
#include "utils/network.cpp"
#ifndef GMENG_NO_CURSES
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

#include "utils/serialization.cpp"

namespace g = Gmeng;
namespace gm = Gmeng;
namespace gmeng = Gmeng;

#endif
