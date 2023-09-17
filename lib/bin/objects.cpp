#include <iostream>


#ifndef __GMENG_OBJECTINIT__
#define __GMENG_OBJECTINIT__ true

namespace Objects {
	class G_Object {
		int id; bool collidable;
	};
	struct coord { int x; int y; };
	struct G_Entity {
		int entityId; int textureId; int colorId;
		bool textured = false; bool colored = false; coord coords;
		inline void setColor(int id) { this->colorId = id; };
	};
	struct G_Player : G_Entity {
		int entityId; int textureId = 0; int colorId = 1;
		bool textured = false; bool colored = true;
		std::string c_ent_tag = "o";
	};
	struct G_Wall {
		int textureId;
	};
};

#endif
