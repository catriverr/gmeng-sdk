#pragma once
#include <string>

#define __GMENG_OBJECTINIT__ true

namespace Gmeng {


    /// X and Y position Coordinate object.
	struct Coordinate {
		int x = 0; int y = 0;
	};

    /// started work on implementing rendering
    /// options for SDL2 screens.
    /// @since 10.2.0-d
    enum Renderer_Type { CONSOLE, EXTERNAL };
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

            const drawpoint operator*( size_t val ) const {
                return { this->x*(int)val, this->y*(int)val };
            };

            const std::string to_string() {
                return "( " + std::to_string(this->x) + ", " + std::to_string(this->y) + " )";
            };
        };

        /// viewpoint, 2 drawpoints with a start and end
        struct viewpoint {
          public:
            drawpoint start;
            drawpoint end;

            const viewpoint operator*( size_t val ) const {
                return { start * val, end * val };
            };

            /// Checker for a drawpoint. Checks if a drawpoint is within
            /// the boundaries of the viewpoint. p must be a constant,
            /// it can be dependant on or a subpoint of a viewpoint.
            /// E.g.: point(0,0) of viewpoint({ 5, 5 }, { 10, 10 })
            /// can not be used with this operator, you should first
            /// call viewpoint[ subpoint_or_dependant ] and pass its
            /// return value to the viewpoint() operator().
            constexpr const bool operator()(const Renderer::drawpoint& p) const {
                return ( p.x >= start.x && p.x <= end.x &&
                         p.y >= start.y && p.y <= end.y );
            };

            /// returns the true position of a subpoint within the viewpoint.
            /// For example:
            ///     let V = viewpoint({ 5, 5 }, { 10, 10 })
            ///     V[{ 0, 0 }] = { 5,   5 }
            ///     V[{ 5, 5 }] = { 10, 10 }
            ///
            /// THIS DOES NOT PERFORM BOUNDS CHECKING.
            constexpr const Renderer::drawpoint operator[](const Renderer::drawpoint& p) const {
                return { p.x + start.x, p.y + start.y };
            };

            inline constexpr void move_by( int by_x, int by_y ) {
                int vp_width_start_x  = this->start.x+by_x,
                    vp_height_start_y = this->start.y+by_y,
                    vp_width_end_x    = this->end.x+by_x,
                    vp_height_end_y   = this->end.y+by_y;

                *this = { { vp_width_start_x, vp_height_start_y },
                          { vp_width_end_x,     vp_height_end_y } };
            };
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
