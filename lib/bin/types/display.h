#pragma once

#include "../src/objects.cpp"

#include "../utils/filters.h"
#include "../utils/subset.h"
#include "../utils/time.h"
#include "../utils/setup_class.h"
#include "../utils/functree.h"
#include "../utils/viewpoint.h"

#include "camera.h"
#include "unit.h"

#include <vector>

namespace Gmeng {
    /// type decl
    class Level;
};

namespace Gmeng::Renderer {
        inline void subimage_copy(
            const uint8_t* src_image,
            int src_stride_width,
            const Gmeng::Renderer::viewpoint& vp,
            uint8_t* dest_image
        ) {
            const int bytes_per_pixel = 4;
        
            int crop_x = vp.start.x;
            int crop_y = vp.start.y;
            int crop_width = vp.end.x - vp.start.x;
            int crop_height = vp.end.y - vp.start.y;
        
            if (crop_width <= 0 || crop_height <= 0 ||
                crop_x <= 0 || crop_y <= 0) {
                return; 
            }
        
            const int row_bytes_to_copy = crop_width * bytes_per_pixel;
        
            #pragma omp parallel for schedule(static)
            for (int y = 0; y < crop_height; ++y) {
                int src_row_index = (crop_y + y) * src_stride_width + crop_x;
                const uint8_t* src_ptr = src_image + (src_row_index * bytes_per_pixel);
        
                uint8_t* dest_ptr = dest_image + (y * row_bytes_to_copy);
        
                std::memcpy(dest_ptr, src_ptr, row_bytes_to_copy);
            }
        };
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
                inline void set_cursor_visibility(bool state) {
                    if (!state) std::cout << "\e[?25l"; // hide
                    else std::cout << "\e[?25h"; // show
                };

                /// Sets the resolution of the display.
                /// This does not affect the viewpoint, only the width/height
                /// of the display and camera. the size of the viewpoint may
                /// cause problems if the resolution and it are not synced.
                inline void set_resolution(std::size_t width, std::size_t height) {
                    __functree_call__(Gmeng::Renderer::Display::set_resolution);
                    this->width = width; this->height = height;
                    this->camera.SetResolution(this->width, this->height);
                };

                /// Generates and draws the newest frame to the screen.
                /// Default draw for both engines (SDL & Terminal-based)
                ///
                /// TODO: Also add SDL default draw support (currently does nothing)
                inline void refresh(Gmeng::Level*, std::function<void(Gmeng::Level*)>);

                struct DisplayController {
                    const Gmeng::Renderer::viewpoint box;
                    uint8_t* original;
                    subarray_2d<uint8_t> content;

                    DisplayController(const Renderer::viewpoint& _box) : box(_box) {
                        this->original = new uint8_t[ vp_size1D(box)*4 ]();
                    };

                    void backdrop( const Renderer::drawpoint& offset, std::function<void(DisplayController*)> controller, bool) const;

                    inline DisplayController* drop_shadow( const uint32_t& color, float opacity = 0.5f, size_t offset_x = 1, size_t offset_y = 1 ) {
                        this->backdrop({ (int)offset_x, (int)offset_y },
                        [this, &color, &opacity, &offset_x, &offset_y](DisplayController* dc) -> void {
                            ::dropshadow( dc->content,
                                          vp_size( dc->box ),
                                          this->box,
                                          color, opacity, offset_x, offset_y );
                        }, true);

                        return this;
                    };

                    inline DisplayController* blur(float radius) {
                        auto size = vp_size(box);
                        ::gaussian_blur( content, size, { 0, 0 }, size, radius);
                        return this;
                    };

                    inline DisplayController* blend( float blend_amount = 0.5f, SIDE dir = SIDE::BOTTOM ) {
                        ::blend(    this->content, this->original,
                                    vp_width(box),
                                   vp_height(box),
                                dir,
                                        0.0f,
                            blend_amount
                               );
                        return this;
                    };

                    inline constexpr uint8_t brightness_extract( uint8_t v, float brightness = 1.0f ) const {
                        return (int)std::clamp( v * brightness, 0.0f, 255.0f );
                    };

                    inline DisplayController* brightness( float brightness ) {
                        #pragma omp parallel for schedule(static)
                        for ( int i = 0; i < vp_size1D(box)*4; i++ )
                            content[i] = brightness_extract(content[i], brightness);
                        return this;
                    };
                };
                /// filters and other options for a given viewpoint within the display
                inline void box(const Gmeng::Renderer::viewpoint& box,
                                std::function<void(DisplayController*)> controller) {

                    DisplayController dc_box(box);

                    int vp_h = vp_height(box);
                    int vp_w = vp_width(box);

                    Renderer::subimage_copy( camera.display_map.framebuffer,
                                      vp_width(viewpoint),
                                                    box,
                                            dc_box.original);

                    dc_box.content = subarray_2d<uint8_t>(
                        camera.display_map.framebuffer,
                        viewpoint * 4,
                        {
                            {
                                box.start.x * 4,
                                box.start.y * 4
                            },
                            {
                                box.end.x * 4,
                                box.end.y * 4
                            }
                        }
                    );

                    controller(&dc_box);

                    delete[] dc_box.original;
                };

                /// Moves the display's viewpoint to the specified new viewpoint.
                inline void move_to(const Gmeng::Renderer::viewpoint& __vp) {
                    __functree_call__(Gmeng::Renderer::Display::move_to);
                    this->viewpoint = __vp;
                };

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
                inline void move_viewpoint_by(int by_x = 0, int by_y = 0) {

                    int vp_width_end_x = this->viewpoint.end.x + by_x;
                    int vp_width_start_x = this->viewpoint.start.x + by_x;

                    /// X position of the viewpoint are smaller than is possible.
                    if ( vp_width_end_x < 0 || vp_width_start_x < 0 ) return;

                    /// X position of the viewpoint is larger than the allowed positions.
                    if ( vp_width_end_x > this->max_width ||
                         vp_width_start_x > this->max_width ) return;

                    int vp_height_end_y = this->viewpoint.end.y + by_y;
                    int vp_height_start_y = this->viewpoint.start.y + by_y;

                    /// X position of the viewpoint are smaller than is possible.
                    if ( vp_height_end_y < 0 || vp_height_end_y < 0 ) return;

                    /// Y position of the viewpoint is larger than the allowed positions.
                    if ( vp_height_end_y > this->max_height ||
                         vp_height_start_y > this->max_height ) return;

                    /// the X delta of the new viewpoint is smaller than the display width
                    if ( vp_width_end_x - vp_width_start_x != this->width ) return;
                    /// the Y delta of the new viewpoint is smaller than the display height
                    if ( vp_height_end_y - vp_height_start_y != this->height ) return;

                    this->viewpoint = { { vp_width_start_x, vp_height_start_y },
                                        { vp_width_end_x,     vp_height_end_y }};
                };
        };

};

#ifndef GMENG_NO_SOL

#include "../utils/scripts.h"
static int __gmeng_define_lua_display_method = ([]() -> int {
    using namespace Gmeng;
    using namespace Renderer;
    using namespace Scripts;

    renderer_lua_namespace.new_usertype<Display>("Display",
            "camera", &Display::camera,
            "viewpoint", &Display::viewpoint,
            "width", &Display::width,
            "height", &Display::height,
            "max_width", &Display::max_width,
            "max_height", &Display::max_height,
            "profiler", &Display::profiler,
            "fixed_update_fps", &Display::fixed_update_fps
    );

    return 0;
})();
#endif

