#pragma once
#include <codecvt>
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
#include "src/objects.cpp"
#include <filesystem>
#if _WIN32 == false
#include <sys/ioctl.h> //ioctl() and TIOCGWINSZ
#endif
#include <unistd.h> // for STDOUT_FILENO


#ifdef __GMENG_OBJECTINIT__

#define vl_get_name(x) #x
#define vl_filename(path) (strrchr(path, '/') ? strrchr(path, '/') + 1 : path)


namespace Gmeng {
    static std::map<std::string, std::string> func_annotations;
    static std::ofstream funclog("gmeng-functree.log");
    static bool functree_init = false;
    static bool functree_enabled = true;
    static std::vector<std::string> func_last(5000);
};

// annotates a function, like information about a function.
static void _func_annot(const char* func, const char* info) {
    Gmeng::func_annotations.emplace(func, std::string(info));
};

#define __annot__(func, info)      _func_annot(vl_get_name(func), info)
#define __info__                   __annot__
#define __annotation__             __annot__

#define __functree_init__() if (!Gmeng::functree_init) Gmeng::funclog << "-- cleared previous log --\n~~GMENG_FUNCTREE~~\n*** This file is used for diagnostics ***\n", Gmeng::functree_init = true

static void _functree_vl(char* file, int line, const char* func) {
    if (!Gmeng::functree_enabled) return;
    if (!Gmeng::functree_init) __functree_init__();
    std::string func_annot = "";
    auto v = Gmeng::func_annotations.find(func);
    if (v != Gmeng::func_annotations.end()) func_annot = "\t\t/// " + v->second;
    std::string dat = vl_filename(file) + std::string(":") + std::to_string(line) + " >> " + func + func_annot;
    Gmeng::funclog << dat << std::endl;
    if (Gmeng::func_last.size() >= Gmeng::func_last.max_size()) Gmeng::func_last.clear();
    Gmeng::func_last.push_back(dat);
};

#define __functree_call__(file, line, func) _functree_vl(file, line, vl_get_name(func))


#define v_str std::to_string

template<typename T>
static uintptr_t _uget_addr(const T& obj) {
    //__functree_call__(__FILE__, __LINE__, _uget_addr);
    return (reinterpret_cast<uintptr_t>(&obj));
};

static bool file_exists(std::string f) {
    __functree_call__(__FILE__, __LINE__, file_exists);
    return std::filesystem::is_directory(f) || std::filesystem::exists(f);
}

static std::string repeatString(const std::string& str, int times) {
    //__functree_call__(__FILE__, __LINE__, repeatString);
    std::string result = "";
    for (int i = 0; i < times; i++) {
        result += str;
    }
    return result;
}

template<typename Thing>
static std::vector<Thing> repeatThing(Thing obj, int times) {
    std::vector<Thing> Things;
    for (int i = 0; i < times; i++) Things.push_back(obj);
    return Things;
};

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

static std::string _uconv_1ihx(int value) {
    //__functree_call__(__FILE__, __LINE__, _conv_1ihx);
    std::stringstream stream;
    stream << "0x" << std::hex << value;
    return stream.str();
}

static int g_mkid() {
    //__functree_call__(__FILE__, __LINE__, g_mkid);
    std::random_device rd; // random device to seed the generator
    std::mt19937 gen(rd()); // mersenne twister 19937 generator
    std::uniform_int_distribution<int> distribution(1000000, 9999999); // 7-digit range

    return distribution(gen);
}

#define stob(str) (str == std::string("true") || str.substr(1) == std::string("true"))
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

static void g_setTimeout(std::function<void()> callback, int delay) {
    __functree_call__(__FILE__, __LINE__, g_setTimeout);
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    callback();
}

static std::string g_readFile(const string &fileName)
{
    __functree_call__(__FILE__, __LINE__, g_readFile);
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return std::string(bytes.data(), fileSize);
};

static void g_rmChar( string &str, char* charsToRemove ) {
    __functree_call__(__FILE__, __LINE__, g_rmChar);
   for ( unsigned int i = 0; i < strlen(charsToRemove); ++i ) {
      str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end() );
   }
}

static std::string g_joinStr(std::vector<std::string> v, std::string delimiter) {
	std::string result;
	for ( auto i : v) { result += i + delimiter; };
	return result;
};

static std::vector<std::string> g_splitStr(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

namespace Gmeng {
    /// current version of the engine.
    /// "-d" suffix means the version is a developer version, high unstability level
    /// "-b" suffix means the version is a beta version, low unstability level but unpolished
    /// "-c" suffix means the version is a coroded version, low to medium unstability level but specific methods will not perform as expected
    static std::string version = "8.2.1-b";
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
	enum CONSTANTS {
		/// integer values
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
	struct event {
		std::string name = ""; int id = 0;
		std::vector<std::string> params = {};
	};
	class EventHandler {
		public:
		// __etp = event type
		// __est = event script
			inline void cast_ev(int __etp, std::string __est) {
                __functree_call__(__FILE__, __LINE__, Gmeng::EventHandler::cast_ev);
                if (__etp < 8500 || __etp > 8599) throw std::invalid_argument("Gmeng::EventHandler->cast_ev(int __etp <- right here | " + std::to_string(__etp) + " is not a valid event type"); 
                if (__etp == Gmeng::CONSTANTS::C_InputEvent) std::cerr << __est;
				if (__etp == Gmeng::CONSTANTS::C_PlugEvent) std::cerr << __est;
				else if (__etp == Gmeng::CONSTANTS::C_LogEvent) std::cerr << "[gm0:core/__log] " + __est;
				return;
			};
			inline std::string gen_estr(Gmeng::event __e) {
                __functree_call__(__FILE__, __LINE__, Gmeng::EventHandler::gen_estr);
                if (__e.id == Gmeng::CONSTANTS::IE_Type0) return std::string ("[gm0:core/__MOUSECLICK__]");
				return std::string ("[gm0:core/__EVCAST] " +
				std::to_string(__e.id) +
				" " + __e.name + " " +
				g_joinStr(__e.params, "!:"));
			};
	};
	struct Coordinate {
		int x = 0; int y = 0;
	};
    static std::stringstream logstream;
    static std::stringstream completelog;
	static std::string colors[] = {
		"\x1B[37m", "\x1B[34m", "\x1B[32m", "\x1B[36m", "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m", "\x1B[37m"
	};
    static std::string bgcolors[] = {
        "\x1B[47m", "\x1B[44m", "\x1B[42m", "\x1B[46m", "\x1B[41m", "\x1B[45m", "\x1B[43m", "\x1B[40m", "\x1B[47m"
    };
    static std::string bgcolors_bright[] = { // match the 'bold' foreground colors (bright)
        "\x1B[107m", "\x1B[104m", "\x1B[102m", "\x1B[106m", "\x1B[101m", "\x1B[105m", "\x1B[103m", "\x1B[40m", "\x1B[107m"
    };
	static std::string colorids[] = { "7", "4", "2", "6", "1", "5", "3", "0" };
	static std::string resetcolor = "\033[22m\033[0m"; static std::string boldcolor = "\033[1m";
	const char c_unit[4] = "\u2588";
	const char c_outer_unit[4] = "\u2584";
	const char c_outer_unit_floor[4] = "\u2580";
	struct Unit {
		public:
			int color = 1; bool collidable = true; bool is_player = false; bool is_entity = false;
            Objects::G_Player player={}; bool transparent = false; bool special = false; int special_clr = 0;
			Objects::G_Entity entity={}; std::string special_c_unit = "";
	};
    struct texture {
        std::size_t width = 0; std::size_t height = 0; bool collidable;
        std::vector<Gmeng::Unit> units; std::string name;
    };
    extern void set_texturemap(Gmeng::texture &g_tx, Gmeng::texture &g_tx2);
    extern Gmeng::texture LoadTexture(std::string __fname);
	struct modifier {
		public:
			std::string name = "v_unallocated_modifier"; int value = 0;
	};
	struct ModifierList {
		public:
			std::vector<modifier> values = {};
	};
	struct RendererOptions {
		public:
		bool countObjects = false; bool useTitle = false;
		std::string title = "";
	};
	template<std::size_t d_width, std::size_t d_height>
	class DisplayMap {
		public:
		int __h = d_width; int __w = d_height;
		Gmeng::Unit unitmap[32767] = {}; int pool_size = (sizeof unitmap / 8);
	};
	template<std::size_t _width, std::size_t _height>
	class G_Renderer {
		public:
		std::size_t width = _width; std::size_t height = _height;
		int totalObjects; Gmeng::DisplayMap<_width, _height> display; Gmeng::RendererOptions options;
		inline void setUnits(Unit unitmap[_width * _height]) {
            __functree_call__(__FILE__, __LINE__, Gmeng::G_Renderer::setUnits);
			for (int i = 0; i < _width * _height; i++) {
				this->display.unitmap[i] = unitmap[i];
			};
		};
	};
    typedef struct {
        std::vector<int> indexes;
        std::vector<std::string> containers;
        bool dev_console;
        bool debugger;
        bool log_stdout;
        bool dev_mode;
        bool dont_hold_back;
        bool shush;
    } __global_object__;
    /// static__ , global_controllers__
    static __global_object__ global;
    static std::ofstream outfile("gmeng.log");
};

inline void controller_set(int index, std::string value) {
    __functree_call__(__FILE__, __LINE__, controller_set);
    Gmeng::global.indexes.push_back(index);
    Gmeng::global.containers.push_back(value);
};
inline void switch_dev_console() {
    __functree_call__(__FILE__, __LINE__, switch_dev_console);
    Gmeng::global.dev_console = !Gmeng::global.dev_console;
};

inline std::string _uget_thread() {
    /// __functree_call__(__FILE__, __LINE__, _uget_thread);
    static std::atomic<int> counter(0);
    thread_local int threadId = counter.fetch_add(1);
    return std::to_string(threadId);
};

inline std::string _uthread_id(const std::thread& thread) {
    /// __functree_call__(__FILE__, __LINE__, _uthread_id);
    std::thread::id threadId = thread.get_id();
    size_t hashValue = std::hash<std::thread::id>{}(threadId);
    return std::to_string(hashValue);
};

#define v_intl int
#define v_static_cast static_cast
#define v_sizel std::size_t
#define v_title std::string
#define v_constl const
#define v_endl std::endl
#define v_nl "\n"
#define v_rcol Gmeng::resetcolor

static void gm_err(v_intl type, v_title err_title) {
    __functree_call__(__FILE__, __LINE__, gm_err);
    switch (type) {
        case 0: // v_gm_err case 0: continue running program
            std::cerr << Gmeng::colors[4] << "gm:0 *error >> " << err_title << v_endl << v_rcol;
            break;
        case 1: // v_gm_err case 1: exception (stop execution)
            throw std::invalid_argument(Gmeng::colors[4] + "gm:0 *error >> " + err_title + v_nl + v_rcol);
            break;
    };
};

inline Gmeng::Unit g_unit(int color = 0, bool collidable = false) {
	return Gmeng::Unit{.color=color, .collidable=collidable};
};
inline Gmeng::Unit g_spike(int color = 0, int bgcolor = 0, bool big = false) {
	return Gmeng::Unit{.color=color,.collidable=false,.special=true,.special_clr=bgcolor,.special_c_unit=(big ? "X" : "x")};
};
inline int g_find_modifier(const std::vector<Gmeng::modifier>& modifiers, const std::string& targetName) {
    __functree_call__(__FILE__, __LINE__, g_find_modifier);
    for (size_t i = 0; i < modifiers.size(); ++i) { if (modifiers[i].name == targetName) { return static_cast<int>(i); }; };
    return -1;
}

inline std::vector<std::string> _ulogc_gen1dvfc(int ln = 7400) {
    __functree_call__(__FILE__, __LINE__, _ulogc_gen1dvfc);
    std::vector<std::string> vector;
    for (int i = 0; i < ln; i++) {
        vector.push_back("\x0b");
    };
    return vector;
};

/// writes to a log file (&name) with content (&content)
static void __gmeng_write_log__(const std::string& name, const std::string& content, bool append = true) {
    Gmeng::outfile << content;
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

#define FILENAME (std::string(__FILE__).substr(std::string(__FILE__).rfind("/") + 1)).c_str()

#define __gmeng_attribute__() ({ \
    std::ostringstream oss; \
    oss << FILENAME << ":" << __LINE__; \
    oss.str(); \
})

static std::string str_replace(const std::string& str, const std::string& from, const std::string& to) {
    // __functree_call__(__FILE__, __LINE__, str_replace);
    std::string result = str; std::size_t start_pos = 0;
    while ((start_pos = result.find(from, start_pos)) != std::string::npos) {
        result.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return result;
};

static std::vector<std::string> colornames = {
    "w", "b", "g", "c", "r", "p", "y", "0", "_", "o", "1"
    // white blue green cyan red pink yellow black
};

static std::vector<std::string> bgcolornames = {
    "Bw", "Bb", "Bg", "Bc", "Br", "Bp", "By", "B0", "Bw", "B1"
    // white blue green cyan red pink yellow black
};

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

#define SAY std::cout << colorformat
#define INF std::cout << "\033[1m" + Gmeng::colors[Gmeng::BLUE] + "(i) >> \033[0m" + Gmeng::colors[Gmeng::WHITE] + colorformat


static void _gm_log(const char* file, int line, std::string _msg, bool use_endl = true) {
    #ifndef __GMENG_ALLOW_LOG__
        __gmeng_write_log__("gmeng.log", "logging is disallowed");
        return;
    #endif
    #if __GMENG_ALLOW_LOG__ == true
        std::string msg = std::string(file) + ":" + v_str(line) + " | " + _msg;
        #if __GMENG_LOG_TO_COUT__ == true
            if (Gmeng::global.log_stdout) std::cout << msg << std::endl;
        #endif
        std::string _uthread = _uget_thread();
        std::string __vl_log_message__ =  "gm:" + _uthread + " *logger >> " + msg + (use_endl ? "\n" : "");
        Gmeng::logstream << __vl_log_message__;
        __gmeng_write_log__("gmeng.log", __vl_log_message__);
        if (Gmeng::global.dev_console) _utext(Gmeng::logc, __vl_log_message__);
        #if __GMENG_DRAW_AFTER_LOG__ == true
            if (Gmeng::global.dev_console) _udraw_display(Gmeng::logc);
        #endif
    #endif
};

static void gm_log(const char* file, int line, std::string _msg, bool use_endl = true) {
    if (Gmeng::global.shush) return;
    _gm_log(file, line, _msg, use_endl);
};

static void gm_log(std::string _msg, bool use_endl = true) {
    if (Gmeng::global.shush) return;
    _gm_log(":",0,_msg,use_endl);
};

static void gm_slog(Gmeng::color_t color, std::string title, std::string text) {
    if (Gmeng::global.shush) return;
    gm_log(":", 0, Gmeng::colors[color] + title + " " + Gmeng::colors[Gmeng::WHITE] + text);
};

namespace Gmeng {
    static std::vector<std::thread> v_threads;
    static std::thread _ucreate_thread(std::function<void()> func) {
        __functree_call__(__FILE__, __LINE__, Gmeng::_ucreate_thread);
        return (Gmeng::v_threads.emplace_back(func)).detach(), std::move(Gmeng::v_threads.back());
    };
    static void _uclear_threads() {
        __functree_call__(__FILE__, __LINE__, Gmeng::_uclear_threads);
        v_threads.erase(std::remove_if(v_threads.begin(), v_threads.end(), [](const std::thread& t) { return !t.joinable(); }), v_threads.end());
    };
    static void _ujoin_threads () {
        __functree_call__(__FILE__, __LINE__, Gmeng::_ujoin_threads);
        for (auto& thread : Gmeng::v_threads) { gm_log(FILENAME, __LINE__, "Gmeng::_ujoin_threads -> gm:v_thread, _ucreate_thread() -> T_MEMADDR: " + _uconv_1ihx(_uget_addr(&thread)) + " - MAIN THREAD ID: " + _uget_thread() + " - T_THREAD_ID: " + _uthread_id(thread)); try { if (thread.joinable()) thread.join(); _uclear_threads(); } catch (std::exception& e) { std::cerr << (Gmeng::colors[4] + "_ujoin_threads() -> *error :: could not join thread, skipping..."); gm_log(FILENAME, __LINE__, " :::: error cause -> " + std::string(e.what())); }; };
    };
}

static void _gupdate_logc_intvl(int ms = 250) {
    __functree_call__(__FILE__, __LINE__, _gupdate_logc_intvl);
    #if __GMENG_ALLOW_LOG__ == false
        return;
    #endif
    __gmeng_write_log__("gmeng.log", "-- cleared previous log --\n", false);
    __gmeng_write_log__("gmeng.log", "Gmeng: Go-To Console Game Engine.\nSPAWN(1) = v_success\ncontroller_t of termui/_udisplay_of(GMENG, window) handed over to: controller_t(gmeng::threads::get(0))\n");
    if (!Gmeng::global.shush) Gmeng::_ucreate_thread([&]() {
            __functree_call__(__FILE__, __LINE__, _glog_thread_create);
        for ( ;; ) {
            if (!Gmeng::global.dev_console) continue;
            if (Gmeng::logstream.str().length() > Gmeng::logc.v_drawpoints.size()) {
                Gmeng::completelog << Gmeng::logstream.str();
                Gmeng::logstream.str(""); /// flush sstream
               _uflush_display(Gmeng::logc, 5);
               gm_log(FILENAME, __LINE__, "t_display *job_flush -> flushed display at gm:thread" + _uget_thread() + " (detached from gm:thread0 / generated from gm:thread0) ; display memory address: " + _uconv_1ihx(_uget_addr(&Gmeng::logc)));
            };
            _udraw_display(Gmeng::logc);
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
        }
        /// close log file
        Gmeng::outfile.close();
    });
};

static void _gthread_catchup() {
    __annotation__(_gthread_catchup, "Gmeng::_uthread catchup function, attaches to all threads and clears them.");
    __functree_call__(__FILE__, __LINE__, _gthread_catchup);
    gm_log(FILENAME, __LINE__, "_gthread_catchup() -> waiting for " + v_str(Gmeng::v_threads.size()) + " threads to catch-up to thread:" + (_uget_thread()));
    Gmeng::_ujoin_threads();
};

static constexpr uint32_t _ghash(const char* data, size_t const size) noexcept {
    uint32_t hash = 5381;
    for(const char *c = data; c < data + size; ++c) hash = ((hash << 5) + hash) + (unsigned char) *c;
    return hash;
}


static wchar_t* concat_wstr(const wchar_t* str1, const wchar_t* str2) {
    size_t len1 = wcslen(str1); size_t len2 = wcslen(str2);
    wchar_t* concatenated = new wchar_t[len1 + len2 + 1];
    wcscpy(concatenated, str1); wcscat(concatenated, str2);
    return concatenated;
};

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

static std::string ws2s(const std::wstring& wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
}

///// __controller_satisfy__
///// OS Check for windows
static void __explain_why_i_cannot_run_to_dumbass_using_windows() {
    __annot__(__explain_why_i_cannot_run_to_dumbass_using_windows, "explains to a user using windows why windows cannot run gmeng.");
    __functree_call__(__FILE__, __LINE__, __explain_why_i_cannot_run_to_dumbass_using_windows);
    std::cout << Gmeng::colors[4] << "libgmeng-abi: __excuse__" << std::endl;
    std::cout << "INTERNAL: __gmeng_platform__, __gmeng_threading__, __stdlib__, __libc++-abi__, __std_com_apple_main_pthread__" << std::endl;
    std::cout << "Gmeng is not available in a core-platform other than darwin ( apple_kernel )." << std::endl;
    std::cout << "current_platform: win32 ( WINDOWS_NT )" << std::endl;
    std::cout << "__gmeng_halt_execution__( CAUSE( gmeng::global.v_exceptions->__find__( \"controller.platform\" ) ) && CAUSE( \"__environment_not_suitable__\" ) )" << std::endl;
    exit(1);
};

static void patch_argv_global(int argc, char* argv[]) {
    __annot__(patch_argv_global, "patches the Gmeng::global variable with the command-line arguments.");
    __functree_call__(__FILE__, __LINE__, patch_argv_global);
    #if _WIN32
        __explain_why_i_cannot_run_to_dumbass_using_windows();
        return;
    #endif
#if _WIN32 == false
    for (int i = 0; i < argc; i++) {
        char *v_arg = argv[i];
        std::string argument (v_arg);
        if ( argument == "-help" || argument == "/help" || argument == "--help" || argument == "/?" || argument == "-?" ) {
            __functree_call__(__FILE__, __LINE__, __gmeng__help__menu__);
            struct winsize size;
            ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);
            int times = size.ws_col-11;
            __gmeng_write_log__("gmeng.log", "command-line argument requested help menu\n");
            SAY("~Br~\x0F~h~\x0F~y~GMENG " + (Gmeng::version) + "~n~ | " + Gmeng::colors[6] + "Terminal-Based 2D Game Engine~n~ | Help Menu\n");
            SAY("~_~~st~" + repeatString("-", times+11) + "~n~\n");
            SAY("~h~~r~Gmeng~n~ is a standalone terminal-based game engine, utilizing ~y~pthread~n~ and the ~b~C++ Standard library~n~.\n");
            SAY("~_~Currently, Gmeng is only available to MacOS and Linux users while on its active development phase.\n");
            SAY("~_~You may contribute to porting ~r~Gmeng~_~ to windows: ~b~https://github.com/catriverr/gmeng-sdk\n");
            SAY("~b~~i~PARAMETERS ~st~" + repeatString("-", times) + "~n~\n");
            SAY("\t    ~y~gmeng ~p~-devc \t\t~_~enables developer console\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-no-devc\t\t~_~disables developer console\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-devmode\t\t~_~enables developer diagnostics\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-log-to-cout\t\t~_~streams logs to stdout\t\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-debugger -debug\t~_~enables extensive debug logs\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-shut-the-fuck-up\t~_~silences all logging\t\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("\t    ~y~gmeng ~p~-tell-me-everything\t~_~enables all logging methods\t~r~(DEFAULT=~p~false~r~)\n");
            SAY("~b~" + repeatString(" ", 11) + "~st~" + repeatString("-", times) + "~n~\n");
            exit(0);
        };
        if ( argument == "-devc" ) Gmeng::global.dev_console = true;
        if ( argument == "-no-devc" || argument == "-shut-the-fuck-up" ) Gmeng::global.dev_console = false;
        if ( argument == "-shut-the-fuck-up" ) Gmeng::global.shush = true;
        if ( argument == "-debugger" || argument == "-debug" || argument == "--debugger" ) Gmeng::global.debugger = true;
        if ( argument == "-log-to-cout" || argument == "-lc" ) Gmeng::global.log_stdout = true;
        if ( argument == "-devmode" ) Gmeng::global.dev_mode = true;
        if ( argument == "-tell-me-everything" ) Gmeng::global.shush = false, Gmeng::global.dev_mode = true, Gmeng::global.dev_console = true, Gmeng::global.dont_hold_back = true, Gmeng::global.debugger = true;
    };
#endif
};


#define __GMENG_INIT__ true /// initialized first because the source files check this value before initialization
#include "src/textures.cpp"
#include "src/gmeng.cpp"
#include "src/renderer.cpp"
#include "types/map.h"
#if _WIN32 == false
#include "utils/network.cpp"
#include "types/interface.h"
#include "utils/interface.cpp"
#endif
namespace g = Gmeng;
namespace gm = Gmeng;
namespace gmeng = Gmeng;
#endif
