#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

#ifdef __GMENG_INIT__
#ifndef __GMENG_MAP_INIT__

#define stob(str) (str == std::string("true") || str.substr(1) == std::string("true"))
#define cpps(str) ( std::string(str) )
using namespace std;

namespace Gmeng {
	struct PlayerData {
		int startDX = 0; int startDY = 0;
		bool colored = true; int colorId;
		std::string c_ent_tag = "o";
	};
	struct WorldData {
		public: 
			std::size_t _w; std::size_t _h;
			std::string name; std::string description;
			Gmeng::PlayerData player;
	};
	struct ModifiedWorldData {
		public: 
			const std::size_t _w; const std::size_t _h;
			std::string name; std::string description;
			Gmeng::PlayerData player;
	};
	namespace MapParser {
		inline Gmeng::ModifiedWorldData ParseWorldInfo(std::string w_cntnt, std::string p_cntnt) {
			Gmeng::WorldData dat; std::vector<std::string> Wvalues = g_splitStr(w_cntnt, "\n"); std::vector<std::string> Pvalues = g_splitStr(p_cntnt, "\n");
			for ( auto val : Wvalues) {
				std::vector<string> datas = g_splitStr(val, "="); std::string vname = datas[0]; std::string vval = datas[1];
				if ( vname == "name" ) dat.name = vval;
				if ( vname == "description" ) dat.description = vval;
				if ( vname == "width" ) dat._w = std::stoi(vval);
				if ( vname == "height") dat._h = std::stoi(vval);
			};
			for ( auto val : Pvalues) {
				std::vector<string> datas = g_splitStr(val, "="); std::string vname = datas[0]; std::string vval = datas[1];
				if ( vname == "startDX" ) dat.player.startDX = std::stoi(vval);
				if ( vname == "startDY" ) dat.player.startDY = std::stoi(vval);
				if ( vname == "colored" ) dat.player.colored = stob(vval);
				if ( vname == "colorId" ) dat.player.colorId = std::stoi(vval);
			}
			auto WPDat = Gmeng::ModifiedWorldData {
				._w = dat._w, ._h = dat._h,
				.name=dat.name,
				.description=dat.description,
				.player=dat.player
			};
			return WPDat;
		};

		template <std::size_t __wd, std::size_t __hg>
		inline Gmeng::CameraView<__wd, __hg> LoadAsset(std::string __f, const std::size_t _w1 = 0, const std::size_t _h1 = 0) {
			Gmeng::CameraView<__wd, __hg> map;
			std::size_t _w = __wd; std::size_t _h = __hg;
			if (_w1 != 0 && _h1 != 0) { map.SetResolution(_w1, _h1); _w = _w1; _h = _h1; };
			std::string cntnt = g_readFile(__f);
			std::vector<std::string> lines = g_splitStr(cntnt, "\n"); // each line represents a single pixel on the WorldMap. Greatest design choice I know.
			if (lines.size() < _w * _h || lines.size() > _w * _h) throw std::invalid_argument(__f + " is smaller than the allocated size of the world map [map.gm UNMATCH! map.cfg]");
			int i = 0;
			for (auto lndx : lines) {
				Gmeng::Unit cur_unit;
				int posX; int posY;
				if (startsWith(lndx, ";")) continue;
				// example lndx:
				// POSITION CLRID COLLISION IS_ENTITY IS_PLAYER SPECIAL SPECIAL_CLR SPECIAL_UNIT
				//   2,5      2     false     false     false     true     5            o
				std::vector<std::string> params = g_splitStr(lndx, " ");
				int indx = 0;
				for (auto param : params) {
					if ( indx ==  0  ) { std::vector<std::string> pos = g_splitStr(param, ","); posX = std::stoi(pos[0]); posY = std::stoi(pos[1]);  };
					if ( indx == 0+1 ) { cur_unit.color = std::stoi(param);  };
					if ( indx == 1+1 ) { cur_unit.collidable = stob(param);  };
					if ( indx == 2+1 ) { cur_unit.is_entity = stob(param);   };
					if ( indx == 3+1 ) { cur_unit.is_player = stob(param);   };
					if ( indx == 4+1 ) { cur_unit.special = stob(param);     };
					if ( indx == 5+1 ) { cur_unit.special_clr = stoi(param); };
					if ( indx == 6+1 ) { cur_unit.special_c_unit = param;    };
					indx++;
				};
				if (indx < 7) throw std::invalid_argument("unitmap at "+__f+" invalid"); 
				map.display_map.unitmap[i] = cur_unit;
				i++;
			};
			return map;
		};
		
		inline Gmeng::ModifiedWorldData GetInfo(std::string __wf, std::string __pf) {
			std::string w_cntnt = g_readFile(__wf); std::string p_cntnt = g_readFile(__pf);
			Gmeng::ModifiedWorldData __pd = Gmeng::MapParser::ParseWorldInfo(w_cntnt, p_cntnt);
			return __pd;
		};
	};
};

#define __GMENG_MAP_INIT__ true
#endif
#endif
