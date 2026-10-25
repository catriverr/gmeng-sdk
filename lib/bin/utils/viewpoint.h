#pragma once

#include "../utils/log.h"
#include "../src/objects.cpp"
#include <vector>

namespace Gmeng {


    inline bool viewpoint_includes(const Renderer::viewpoint& a, const Renderer::viewpoint& b, bool retval) {
        return ( (a.start.x <= b.end.x && a.end.x >= b.start.x) &&
                 (a.start.y <= b.end.y && a.end.y >= b.start.y) );
    };

    /// checks if a viewpoint contains a sub-viewpoint
    /// this code is horrifying to edit, we should just hope that it will never cause issues
    inline std::vector<Renderer::drawpoint> viewpoint_includes(const Renderer::viewpoint &v_control, const Renderer::viewpoint &v_check) {
        std::vector<Renderer::drawpoint> __shared__;
        // Check if v_checks start point is within control
        if (v_check.start.x >= v_control.start.x && v_check.start.y >= v_control.start.y &&
            v_check.start.x <= v_control.end.x && v_check.start.y <= v_control.end.y) {
            __shared__.push_back(v_check.start);
        }

        // Check if v_checks end point is within control
        if (v_check.end.x >= v_control.start.x && v_check.end.y >= v_control.start.y &&
            v_check.end.x <= v_control.end.x && v_check.end.y <= v_control.end.y) {
            __shared__.push_back(v_check.end);
         }

        // Check if v_checks start and end points overlap control
        if (v_check.start.x <= v_control.end.x && v_check.end.x >= v_control.start.x &&
            v_check.start.y <= v_control.end.y && v_check.end.y >= v_control.start.y) {
            // Determine shared drawpoints within the overlapping region
            int sharedStartX = std::max(v_check.start.x, v_control.start.x);
            int sharedStartY = std::max(v_check.start.y, v_control.start.y);
            int sharedEndX = std::min(v_check.end.x, v_control.end.x);
            int sharedEndY = std::min(v_check.end.y, v_control.end.y);

            __iterator_outer__: for (int x = sharedStartX; x <= sharedEndX; ++x) {
                __iterator_inner__: for (int y = sharedStartY; y <= sharedEndY; ++y) {
                    __shared__.push_back({x, y});
                }
            };
        };

        return __shared__;
    };

    /// checks if a viewpoint includes a drawpoint.
    inline bool viewpoint_includes_dp(Renderer::viewpoint vp, Renderer::drawpoint dp) {
        //__functree_call__(Gmeng::viewpoint_includes_dp);
        return (dp.x >= vp.start.x && dp.x <= vp.end.x &&
                dp.y >= vp.start.y && dp.y <= vp.end.y);
    };


    /// returns delta of a viewpoint's X coordinate
    template <typename number_type = int>
    inline number_type _vcreate_vp2d_deltax(Renderer::viewpoint vp) {
        return v_static_cast<number_type>( (vp.end.x - vp.start.x) );
    };

    /// returns delta of a viewpoint's Y coordinate
    template <typename number_type = int>
    inline number_type _vcreate_vp2d_deltay(Renderer::viewpoint vp) {
        return v_static_cast<number_type>( (vp.end.y - vp.start.y) );
    };

    template <typename... Args>
    auto vp_width(Args&&... args) -> decltype(_vcreate_vp2d_deltax(std::forward<Args>(args)...)) {
        return _vcreate_vp2d_deltax(std::forward<Args>(args)...);
    };

    template <typename... Args>
    auto constexpr vp_height(Args&&... args) -> decltype(_vcreate_vp2d_deltay(std::forward<Args>(args)...)) {
        return _vcreate_vp2d_deltay(std::forward<Args>(args)...);
    };

    inline const Renderer::drawpoint vp_size(const Renderer::viewpoint& vp) {
        return {
            vp.end.x - vp.start.x,
            vp.end.y - vp.start.y
        };
    };

    inline const int constexpr vp_size1D(const Renderer::viewpoint& vp) {
        return (vp.end.x - vp.start.x) * ( vp.end.y - vp.start.y );
    };

    namespace Renderer {
        /// returns placement coordinates for a viewpoint within a map sizeof drawpoint(x,y)
        /// this method will modify the viewpoint to ensure that the viewpoint is within the
        /// map's boundaries if its start_xy is < 0 or end_xy > map_xy.
        inline std::vector<drawpoint> get_displacement(viewpoint& view, const drawpoint& map) {
            /// normalize the viewpoint to the bounds of the map drawpoint
            view.start.x = std::max(view.start.x, 0); view.end.x = std::min(view.end.x, map.x - 1);
            view.start.y = std::max(view.start.y, 0); view.end.y = std::min(view.end.y, map.y - 1);

            int width = view.end.x - view.start.x + 1;
            int height = view.end.y - view.start.y + 1;

            if (width <= 0 || height <= 0) {
                return {};
            }

            std::vector<drawpoint> result(static_cast<size_t>(width) * height);

            size_t i = 0;
            for (int y = view.start.y; y <= view.end.y; ++y)
                for (int x = view.start.x; x <= view.end.x; ++x)
                    result[i++] = {x, y};

            return result;
        };

        static viewpoint c_npos = viewpoint {
            .start = drawpoint { .x=-1, .y=-1 },
            .end   = drawpoint { .x=-1, .y=-1 },
        };

        /// returns drawpoint object as log string
        inline std::string conv_dp(drawpoint p) {
            //__functree_call__(Gmeng::Renderer::conv_dp);
            return "{ x:" + v_str(p.x) + ", y:" + v_str(p.y) + " }";
        };

        inline std::string conv_vp(viewpoint vp) {
            return "{ (x=" + v_str(vp.start.x) + ",y=" + v_str(vp.start.y) + "), " +
                     "(x=" + v_str(vp.end.x)   + ",y=" + v_str(vp.end.y) + ") }";
        };

        /// returns size of a drawpoint in total amount of units (NOT FOR X,Y POSITIONS)
        /// for coordinate sizes, use trace_1dp(msize, mapwidth)
        /// this method asumes that the given drawpoint is not a position, but
        /// the total size of a canvas.
        /// For example:
        /// drawpoint { 5,5 } represents an object with the width of 5 and height of 5,
        /// so Gmeng::Renderer::getsize returns 25 for this object.
        /// this means that this drawpoint does not represent the coordinate 5,5;
        /// however an object with the size of 5,5.
        inline std::size_t getsize(drawpoint c) {
            //__functree_call__(Gmeng::Renderer::getsize);
            return (
                        c.x < 1 ?
                        ( c.y ) :
                        ( c.x * c.y )
                   );
        };

        // returns placement coordinates for each coordinate of object at __p with size of __s in mapsize of __ws
        inline std::vector<Renderer::drawpoint> get_placement(drawpoint __p, drawpoint __s, drawpoint __ws, bool _vlinear_render = false) {
            //__functree_call__(Gmeng::Renderer::get_placement);
            gm_log("gm::v_renderer -> get_placement : pvalues = 1: " + conv_dp(__p) + " 2: " + conv_dp(__s) + " 3: " + conv_dp(__ws));
            if (getsize(__p) > getsize(__ws) || getsize({ __s.x-1, __s.y-1 }) > getsize(__ws)) throw std::invalid_argument("placement parameters invalid");
            std::vector<Gmeng::Renderer::drawpoint> vec(getsize(__s));

            int g_i = 0;
            if (_vlinear_render) {
                if ((__p.x + __s.x <= __ws.x) && (__p.y + __s.y <= __ws.y)) {
                    for (int i = 0; i < __s.x; ++i) {
                        for (int j = 0; j < __s.y; ++j) {
                            Gmeng::Renderer::drawpoint point = {.x=__p.x + i, .y=__p.y + j};
                            vec[g_i++] = (point);
                        };
                    };
                } else { std::cerr << "Gmeng::Renderer::get_placement: e_obj: out of boundaries: __getsize(__p, __s, __ws);" << std::endl; };
            } else {
                //if ((__p.x + __s.x <= __ws.x) && (__p.y + __s.y <= __ws.y)) { /// horizontal render
                    for (int j = 0; j < __s.y; ++j) {
                        for (int i = 0; i < __s.x; ++i) {
                            Gmeng::Renderer::drawpoint point = {.x=__p.x + i, .y=__p.y + j};
                            vec[g_i++] = (point);
                        }
                    }
                //} else { std::cerr << "Gmeng::Renderer::get_placement: e_obj: out of boundaries: __getsize(__p, __s, __ws);" << std::endl; };
            };
            ASSERT("pref.log", p_no);
            return vec;
        };


    };
};
