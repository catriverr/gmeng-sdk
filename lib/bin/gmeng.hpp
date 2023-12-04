#include <exception>
#include <iostream>
#include <fstream>
#include <iterator>
#include <thread>
#include <chrono>
#include <vector>
#include <string>
#include "utils/textures.cpp"
#include "objects.cpp"
#include <future>
#include <functional>
#include <random>
#include <map>
#include <algorithm>
#include <sstream>
#ifdef __GMENG_OBJECTINIT__

#define v_str std::to_string

static void gm_nlog(std::string msg) {
    #ifndef __GMENG_ALLOW_LOG__
        return;
    #endif
    #if __GMENG_ALLOW_LOG__ == true
        std::cerr << msg;
    #endif
};

static void gm_log(std::string msg, bool use_endl = true) {
    #ifndef __GMENG_ALLOW_LOG__
        return;
    #endif
    #if __GMENG_ALLOW_LOG__ == true
        std::cerr << "gm:0 *logger >> " + msg + (use_endl ? "\n" : "");
    #endif
};

std::string _uconv_1ihx(int value) {
    std::stringstream stream;
    stream << "0x" << std::hex << value;
    return stream.str();
}

static int g_mkid() {
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
    std::this_thread::sleep_for(std::chrono::milliseconds(delay));
    callback();
}

static std::string g_readFile(const string &fileName)
{
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return std::string(bytes.data(), fileSize);
};

static void g_rmChar( string &str, char* charsToRemove ) {
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
		std::string name; int id;
		std::vector<std::string> params;
	};
	class EventHandler {
		public:
		// __etp = event type
		// __est = event script
			inline void cast_ev(int __etp, std::string __est) {
                if (__etp < 8500 || __etp > 8599) throw std::invalid_argument("Gmeng::EventHandler->cast_ev(int __etp <- right here | " + std::to_string(__etp) + " is not a valid event type"); 
                if (__etp == Gmeng::CONSTANTS::C_InputEvent) std::cerr << __est;
				if (__etp == Gmeng::CONSTANTS::C_PlugEvent) std::cerr << __est;
				else if (__etp == Gmeng::CONSTANTS::C_LogEvent) std::cerr << "[gm0:core/__log] " + __est;
				return;
			};
			inline std::string gen_estr(Gmeng::event __e) {
                if (__e.id == Gmeng::CONSTANTS::IE_Type0) return std::string ("[gm0:core/__MOUSECLICK__]");
				return std::string ("[gm0:core/__EVCAST] " +
				std::to_string(__e.id) +
				" " + __e.name + " " +
				g_joinStr(__e.params, "!:"));
			};
	};
	struct Coordinate {
		int x; int y;
	};
	std::string colors[] = {
		"\x1B[39m", "\x1B[34m", "\x1B[32m", "\x1B[36m", "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m", "\x1B[37m"
	};
	std::string colorids[] = { "7", "4", "2", "6", "1", "5", "3", "0" };
	std::string resetcolor = "\033[22m\033[0m"; std::string boldcolor = "\033[1m";
	const char c_unit[4] = "\u2588";
	const char c_outer_unit[4] = "\u2584";
	const char c_outer_unit_floor[4] = "\u2580";
	struct Unit {
		public:
			int color; bool collidable;
			bool is_entity = false; bool is_player = false; bool special = false; int special_clr = 0;
			Objects::G_Entity entity={}; Objects::G_Player player={}; std::string special_c_unit = "";
	};


	struct modifier {
		public:
			int value; std::string name;
	};
	struct ModifierList {
		public:
			std::vector<modifier> values;
	};
	struct RendererOptions {
		public:
		bool countObjects; bool useTitle;
		std::string title;
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
			for (int i = 0; i < _width * _height; i++) {
				this->display.unitmap[i] = unitmap[i];
			};
		};
	};
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
	return Gmeng::Unit{.collidable=collidable,.color=color};
};
inline Gmeng::Unit g_spike(int color = 0, int bgcolor = 0, bool big = false) {
	return Gmeng::Unit{.collidable=false,.color=color,.special=true,.special_c_unit=(big ? "X" : "x"),.special_clr=bgcolor};
};
inline int g_find_modifier(const std::vector<Gmeng::modifier>& modifiers, const std::string& targetName) {
    for (size_t i = 0; i < modifiers.size(); ++i) { if (modifiers[i].name == targetName) { return static_cast<int>(i); }; };
    return -1;
}

#define __GMENG_INIT__ true /// initialized first because the source files check this value before initialization
#include "def/gmeng.cpp"
#include "def/renderer.cpp"
#include "utils/envs/map.hpp"
#include "utils/termui.cpp"

namespace g = Gmeng;
namespace gm = Gmeng;
namespace gmeng = Gmeng;
#endif
