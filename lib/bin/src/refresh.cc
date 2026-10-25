#pragma once

#include "../utils/console.h"

inline void Gmeng::Renderer::Display::refresh(Gmeng::Level* level, std::function<void(Gmeng::Level*)> after_render_callback = nullptr) {
    #if GMENG_SDL != true

        /// ImGui_ImplGmeng state
        bool imgui_enabled = this->camera.modifiers.get_value("imgui") == 1;
        /// Generate a priority drawable scale of the level
        /// (renderscale ignores objects that are too far out
        ///  of view from the display's viewpoint/resolution)
        level->renderscale = Gmeng::get_renderscale(*level);
        /// converts the renderscale into a text blob so it can be
        /// drawn as raw text by the camera to the Terminal screen
        std::string lvl_view = Gmeng::get_lvl_view(*level, level->renderscale);

        if ( after_render_callback != nullptr ) after_render_callback( level );

        /// After rendering the game view, we still have some draw
        /// commands for stuff like UI elements. If the list is not
        /// empty, we loop through it to draw them to the buffer.
        if ( !this->camera.after_draw_list.empty() ) {

            int vph = vp_height( this->viewpoint );
            for ( auto& draw : camera.after_draw_list ) {
                auto& pos = std::get<0>(draw);
                auto& img  = std::get<1>(draw);

                for (int y = 0; y < img.height; y++)
                    #pragma omp simd
                    for (int x = 0; x < img.width; x++) {
                        int pos = (y * vph + x )* 4;
                        auto& un = img.units[y * img.width + x];
                        camera.display_map.framebuffer[ pos + 0 ] = ( un.color >> 16 ) & 0xFF;
                        camera.display_map.framebuffer[ pos + 1 ] = ( un.color >>  8 ) & 0xFF;
                        camera.display_map.framebuffer[ pos + 2 ] = ( un.color       ) & 0xFF;
                    };
            };
            this->camera.after_draw_list.clear();
        };

        /// KGP_PIXEL_SUPPORT: Kitty Terminal Graphics Protocol
        /// if the terminal supports the protocol, we can render
        /// the level view as a framebuffer of RGB values instead.
        /// `get_lvl_view()` creates the buffer if support for it
        /// is available, so we can use that buffer to draw instantly.
        if ( Gmeng::global.terminal_has_kgp_pixel_support        &&
             this->camera.has_modifier("efficient_render") &&
             this->camera.has_modifier("kitty_graphics") ) {
            /// Resize the renderer with the viewpoint's width / height
            this->camera.renderer.width  = Gmeng::vp_width ( this->viewpoint );
            this->camera.renderer.height = Gmeng::vp_height( this->viewpoint );

            if ( dev_console_open )
                gmeng_kgp_console(main_event_loop, &NO_EVENT_INFO);

            if ( imgui_enabled ) {
                for ( auto& t_e : main_event_loop->editors ) {
                    if (!t_e.second->loaded) t_e.second->Load( t_e.second->current_filename );
                    if (t_e.second->is_open) t_e.second->Draw();
                };

                ImGui::Render();
                ImGui_ImplKitty_RenderDrawData(
                    ImGui::GetDrawData(),
                    this->camera.display_map.framebuffer,
                    this->camera.renderer.width, this->camera.renderer.height
                );
            };

            /// if the application is not focused, do not display the view,
            /// as it will max out the cpu AND will cause the computer to crash
            /// or the program to seg-fault. The cause for this is because kitty
            /// normally will deallocate the shared memory space given to it which
            /// contains the uint8_t array for that frame, but if it is not focused
            /// it does not draw the image and subsequently does not deallocate its
            /// memory. This will cause a memory leak in Gmeng's source code. The
            /// solution is to ignore refresh() calls if the app is not focused.
            if ( !Gmeng::global.app_focused ) return;

            /// Present the image using the protocol. The camera instance already
            /// has a terminal_graphics_manager, so we can just send present().
            this->camera.renderer.present( this->camera.display_map.framebuffer );
            /// For draw_time calculation. drawing the frame takes less time
            /// than generating the frame in the Kitty Graphics Protocol.
            /// Draw time includes the frame_time and is the best way
            /// to measure the Frames Per Second via 1000/draw_time.
            this->camera.draw_time = 1000 * this->camera.frame_time;
        } else {
            /// Places the level view into the camera so its draw()
            /// function consumes the data and can write it to the screeen.
            Gmeng::emplace_lvl_camera(*level, lvl_view);

            /// For draw_time calculation. drawing the frame takes longer
            /// than generating the frame in the terminal since the raw
            /// text contains a large chunk of colorcodes and escape sequences.
            /// draw_time includes the frame_time as well.
            long long draw_time_b = GET_TIME();

            /// checks draw_info for displaying debug information to the screen.
            if (level->display.camera.modifiers.get_value("draw_info") == 1)
                level->display.camera.draw_info(vp_width(level->display.viewpoint)+2, 0);

            /// Update and emplace UI elements to the camera view.
            level->display.camera.apply_ui();
            /// seeks to the 0,0 position of the terminal screen
            /// and writes the new frame. This will override the
            /// previously displayed frame, but if there was a change
            /// of resolution between frames this means that artifacts
            /// of the previous frame will be visible on the screen.
            ///
            /// Camera::clear_screen() can be called in such cases, but
            /// since resolution changes are not called by the engine,
            /// there is no need for an internal system to detect them
            /// and clear the screen accordingly, as it's the user's responsibility.
            std::string drawn =
                Gmeng::HOME + level->display.camera.draw() + Gmeng::resetcolor;
            /// write to STDOUT directly (was using std::cout before, idiot me)
            /// this is way more efficient.
            write( STDOUT_FILENO, drawn.c_str(), drawn.size() );
            /// Set the actual draw time
            level->display.camera.draw_time = GET_TIME() - draw_time_b;
        };
    #endif
};
