#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include <string>
#include "utils/textures.cpp"
#include "objects.cpp"

#ifdef __GMENG_OBJECTINIT__
#define stob(str) (str == std::string("true") || str.substr(1) == std::string("true"))
#define cpps(str) ( std::string(str) )
using namespace std;

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

std::string g_readFile(const string &fileName)
{
    ifstream ifs(fileName.c_str(), ios::in | ios::binary | ios::ate);

    ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, ios::beg);

    vector<char> bytes(fileSize);
    ifs.read(bytes.data(), fileSize);

    return std::string(bytes.data(), fileSize);
};

void g_rmChar( string &str, char* charsToRemove ) {
   for ( unsigned int i = 0; i < strlen(charsToRemove); ++i ) {
      str.erase( remove(str.begin(), str.end(), charsToRemove[i]), str.end() );
   }
}

std::string g_joinStr(std::vector<std::string> v, std::string delimiter) {
	std::string result;
	for ( auto i : v) { result += i + delimiter; };
	return result;
};

std::vector<std::string> g_splitStr(std::string s, std::string delimiter) {
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
		C_LogEvent = 8534, LE_Type0 = 8535, LE_Type1 = 8536,
		C_PlugEvent = 8544, PE_Type0 = 8545, PE_Type1 = 8546
	};
	struct event {
		std::string name; int id;
		std::vector<std::string> params; 
	};
	class EventHandler {
		public:
			inline void cast_ev(int __eid, std::string __est) {
				if (__eid == Gmeng::CONSTANTS::C_PlugEvent) std::cerr << __est; 
				else if (__eid == Gmeng::CONSTANTS::C_LogEvent) std::cout << __est;
				return;
			};
			inline std::string gen_estr(Gmeng::event __e) {
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
		"\x1B[39m", "\x1B[34m", "\x1B[32m", "\x1B[36m", "\x1B[31m", "\x1B[35m", "\x1B[33m", "\x1B[30m"
	};
	std::string resetcolor = "\033[0m";
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
		int __h = d_width; int __w = d_height; int pool_size = (sizeof unitmap / 8);
		Gmeng::Unit unitmap[32767] = {};
	};
	template<std::size_t _width, std::size_t _height>
	class Renderer {
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
#define __GMENG_INIT__ true
#include "def/gmeng.cpp"
#include "utils/envs/map.hpp"
#endif