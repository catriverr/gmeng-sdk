#pragma once
#include <string>

#define __GMENG_OBJECTINIT__ true

namespace Gmeng {
    namespace Renderer {
        /// position in a viewpoint
        struct drawpoint {
            int x; int y;

            /// drawpoint comparison
            bool operator<(const drawpoint& other) const {
                if (y != other.y) return y < other.y;
                return x < other.x;
            };
            /// drawpoint equals
            bool operator==(const drawpoint&) const = default;
            bool operator!=(const drawpoint&) const = default;

        };
    };
};

namespace Objects {
	class G_Object {
		int id = 0; bool collidable = false;
	};
	struct coord {
        int x; int y;

        bool operator==(const coord& other) const = default;
        operator Gmeng::Renderer::drawpoint() const {
            return { x, y };
        }
    };
	struct G_Entity {
		int entityId = 0; int textureId = 0; int colorId = 0;
		bool textured = false; bool colored = false; coord coords = { .x=0, .y=0 };
		inline void setColor(int id) { this->colorId = id; };
	};
	struct G_Player : G_Entity {
		int entityId = 0; int textureId = 0; int colorId = 1;
		bool textured = false; bool colored = true;
		std::string c_ent_tag = "o";
	};
	struct G_Wall {
		int textureId = 0;
	};
};
