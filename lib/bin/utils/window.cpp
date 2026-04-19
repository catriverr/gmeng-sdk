#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <filesystem>

#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <vector>
#include <iostream>
#include <map>

#include "../gmeng.h"
#include "../src/renderer.cpp"

#include "../types/window.h"

/// had to modify this bitch
/// disregard vim errors here
/// this code compiles
/// Coc is a pile of shit that
/// nobody should get sucked into
#include "../../../include/imgui/backends/imgui_impl_sdl2.h"
#include "../../../include/imgui/backends/imgui_impl_sdlrenderer2.h"
#include "../../../include/imgui/imgui.h"
#include "SDL_render.h"
#include "../types/IconsFontAwesome7.h"

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)

#define FILE_DIRECTORY(file) \
    (std::string(STR(file)).substr(0, std::string(STR(file)).find_last_of("/\\")))
// Returns true if selection changed, current item index updated by reference
bool ImGui_CenteredSelection(
    const char* label_id,         // ImGui unique ID label (for widget identity)
    int* current_item,            // current selected index (input & output)
    const char* const* items,     // array of item strings
    int item_count,
    float total_width = 0.0f      // optional fixed total width (0 = auto calc)
) {
    if (item_count <= 0) return false;
    ImGui::PushID(label_id);

    // Calculate widths
    ImVec2 arrow_size = ImGui::CalcTextSize(" <");
    arrow_size.y += 5;
    ImVec2 label_size = ImGui::CalcTextSize(items[*current_item]);
    if (total_width <= 0.0f) {
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        total_width = arrow_size.x + label_size.x + arrow_size.x + spacing * 2;
    }

    ImVec2 avail = ImGui::GetContentRegionAvail();
    float start_x = ImGui::GetCursorPosX() + (avail.x - total_width) * 0.5f;
    ImGui::SetCursorPosX(start_x);

    bool changed = false;

    // Prev arrow
    if (ImGui::Button("<", arrow_size)) {
        current_item--;
        if (*current_item < 0) *current_item = item_count-1;
        changed = true;
    }
    ImGui::SameLine();

    // Label text
    ImGui::TextUnformatted(items[*current_item]);
    ImGui::SameLine();

    // Next arrow
    if (ImGui::Button(">", arrow_size)) {
        current_item++;
        changed = true;
    }

    ImGui::PopID();
    return changed;
}

bool ImGui_ClickableRect(const char* str_id, ImVec2 p_min, ImVec2 p_max, ImU32 color, ImU32 hover_color, ImU32 click_color)
{
    // Calculate the size required for the invisible button
    ImVec2 size = ImVec2(p_max.x - p_min.x, p_max.y - p_min.y);

    // Save the current cursor position so we don't disrupt the normal UI flow
    ImVec2 backup_pos = ImGui::GetCursorScreenPos();

    // Set absolute position for our custom widget (p_min is our top-left starting point)
    ImGui::SetCursorScreenPos(p_min);

    // Create an invisible button to handle interaction (hover, click, active states)
    ImGui::PushID(str_id);
    bool clicked = ImGui::InvisibleButton("##clickable_rect", size);
    ImGui::PopID();

    // Check interaction states
    bool is_hovered = ImGui::IsItemHovered();
    bool is_active = ImGui::IsItemActive(); // True while the mouse button is held down

    // Determine the color based on the current state
    ImU32 current_color = color;
    if (is_active)
    {
        current_color = click_color;
    }
    else if (is_hovered)
    {
        current_color = hover_color;
    }

    // Draw the rectangle using the provided min and max bounds directly
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(p_min, p_max, current_color);

    // Restore the cursor position for standard ImGui widgets that follow
    ImGui::SetCursorScreenPos(backup_pos);

    return clicked;
}

bool ImGui_SelectableImage(ImTextureID texture, ImVec2 size, bool selected = false, ImU32 highlightColor = IM_COL32(255, 255, 0, 255), float thickness = 2.0f) {
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Image(texture, size);

    bool hovered = ImGui::IsItemHovered();
    if (hovered || selected) {
        ImU32 color = selected ? highlightColor : IM_COL32(180, 180, 180, 255); // Yellow if selected, gray if just hovered
        ImGui::GetWindowDrawList()->AddRect(
            pos,
            ImVec2(pos.x + size.x, pos.y + size.y),
            color,
            0.0f,
            0,
            thickness
        );
    }

    return hovered;
}
void ImGui_CenteredText(const char* text, ImFont* font = nullptr) {
    ImVec2 text_size = ImGui::CalcTextSize(text);
    float window_width = ImGui::GetContentRegionAvail().x;
    float text_x = (window_width - text_size.x) * 0.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + text_x);
    if (font) ImGui::PushFont(font);
    ImGui::TextUnformatted(text);
    if (font) ImGui::PopFont();
}

struct GmengImGuiScripts {
  public:
    bool open = 0;
    std::string selected_file = "";
    inline void Draw() {
        ImGui::SetNextItemOpen(false);
        ImGui::SetNextWindowPos({0, 0});
        ImGui::SetNextWindowSize({ 960, 540 });
        ImGui::Begin("gmeng script manager", &open, ImGuiWindowFlags_NoResize);

        ImGui_CenteredText(selected_file.length() > 0 ? selected_file.c_str() : "select a script file to view its contents");


            ImGui::BeginChild("script_list", { 340,0 }, true);
                ImGui::SeparatorText("scripts/src/**");
                for (auto f : filesystem::directory_iterator("scripts/src/")) {
                    auto file_icon = f.is_directory() ? ICON_FA_FOLDER : ICON_FA_FILE_CODE;
                    auto label = (file_icon + f.path().generic_string());

                    if (f.path().generic_string() != selected_file) {
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 0, 0, 0, 0 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 40, 40, 170, 200 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 40, 40, 170, 255 ));
                    };

                    if (f.is_directory()) ImGui::BeginDisabled();
                    if (ImGui::Button(label.c_str()) && selected_file != f.path().generic_string()) {
                        selected_file = f.path().generic_string();
                        ImGui::PopStyleColor(3);
                    };
                    if (f.is_directory()) ImGui::EndDisabled();
                    if ( selected_file != f.path().generic_string() ) ImGui::PopStyleColor(3);
                    ImGui::Separator();
                }

            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::Separator();
        ImGui::End();
    };
};

struct GmengImGuiVGM {
    Gmeng::EventLoop* ev;
    SDL_Renderer* renderer;

    GmengImGuiVGM( Gmeng::EventLoop* _ev, SDL_Renderer* _renderer ) :
        ev( _ev ), renderer( _renderer )
    { __functree_call__(GmengImGuiVGM::create); };

    ~GmengImGuiVGM() {
        for (auto& val : this->texture_map) {
            SDL_DestroyTexture(val.second.txtr);
        };
    };

    struct VGMItem {
        SDL_Texture* txtr;
        Gmeng::texture g_txtr;
        Gmeng::Renderer::Model model;
        std::shared_ptr<Gmeng::EntityBase> entity;

        bool selected = false;
        bool is_entity = false;
    };

    std::map<std::string, VGMItem> texture_map;

    bool popup_open = false;

    VGMItem* selected = nullptr;

    char inputBuffer[256] = "";
    std::string bufferString = "";

    char filterBuffer[256] = "";

    int selected_chunk = 0;

    inline void Draw() {

        /// vgm was updated, renew this cache
        if ( texture_map.size() != Gmeng::vgm_defaults::vg_textures.size() ) {
            gm_log("renewing old cache("$(texture_map.size())") from vgm("$(Gmeng::vgm_defaults::vg_textures.size())")");
            /// Append a functree call for this event
            __functree_call__(GmengImGuiVGM::update_cache);
            /// Free the previous cache, otherwise this will
            /// cause a memory leak (eventually)
            for (auto& val : this->texture_map) {
                /// This technically doesn't remove the value,
                /// just frees its SDL_Texture* object making
                /// it a nullptr but we will insert_or_assign
                /// to it so it's not a problem.
                SDL_DestroyTexture(val.second.txtr);
            };
            /// append/write the new VGM data to the cache
            for ( auto txtr : Gmeng::vgm_defaults::vg_textures.v_getrelative() ) {
                gm_log("loading VGM content " + v_str(txtr.id) + ":" + txtr.data.name);
                /// Convert to sImage using the
                /// size definition of the texture
                Gmeng::sImage img = { (int)txtr.data.width, (int)txtr.data.height };
                /// append pixel data to the sImage
                /// from the unit info on the texture
                for ( auto un : txtr.data.units )
                    img.content.push_back( (Gmeng::color_t)un.color );
                /// Convert to SDL_Texture (hardware-heavy)
                /// this part can cause memory leaks if used
                /// improperly.
                SDL_Texture* sdl_texture = make_texture(this->renderer,
                    /// Stretch to 100x100 for better visualisation in the screen
                    stretchImage( img, 100, 100 )
                );
                /// Add/assign to texture_map cache
                this->texture_map[ txtr.data.name ] = { sdl_texture, txtr.data };
            };
        };

        ImGui::SetNextWindowPos({ 1220, 540 });
        ImGui::SetNextWindowSize(ImVec2( 700, 375 ));
        ImGui::Begin("gmeng vgm", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui_CenteredText("VisualCache Graphics Manager");

        ImGui::Separator();


        /// VGM load utility only available if vgm_load is set to true
        if ( this->ev->level->display.camera.modifiers.get_value("vgm_load") == 1 ) {

            ImGui::SetNextItemWidth( 445 );
            ImGui::InputText("vgm directory", inputBuffer, IM_ARRAYSIZE(inputBuffer));
            ImGui::SameLine();
            if (ImGui::Button("load")) {
                bufferString = inputBuffer;
                inputBuffer[0] = '\0'; // reset string

                if (filesystem::exists( bufferString )) {
                    if (filesystem::is_directory( bufferString )) {
                        Gmeng::_uread_into_vgm(bufferString);
                        gm_log("VGM load request to " + bufferString + " complete");
                    } else {
                        gm_log("invalid VGM load request: ENOTDIRECTORY " + bufferString);
                    };
                } else {
                    gm_log("invalid VGM load request: ENOENT " + bufferString);
                };
            };

            ImGui::Separator();
        };


        ImGui_CenteredText(("VGM models: "
                           $(Gmeng::vgm_defaults::vg_rdmodels.size())
                           " | VGM textures: "$(Gmeng::vgm_defaults::vg_textures.size())"").c_str());
        ImGui_CenteredText(("Cached objects: "$(this->texture_map.size())"").c_str());

        ImGui::Separator();

        ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x-98 );
        ImGui::InputText("search", filterBuffer, IM_ARRAYSIZE(filterBuffer));
        std::string searchstr = filterBuffer;

        ImGui::SameLine();

        if (ImGui::Button("clear")) {
            filterBuffer[0] = '\0'; /// clear char* buffer
        };

        ImGui::Separator();


        ImGui::PushStyleColor( ImGuiCol_ChildBg, IM_COL32( 143, 182, 190, 255 ) );
        ImGui::BeginChild("previews", {
                    ImGui::GetContentRegionAvail().x,
                    ImGui::GetContentRegionAvail().y-65
                },
                ImGuiChildFlags_Border, ImGuiWindowFlags_HorizontalScrollbar );

        ImVec2 txtr_size (100, 100);

        int i = 0;
        ImGui::SameLine();
        for ( auto& sdl_texture : this->texture_map ) {
            if (i != 0 && i % 5 == 0) ImGui::NewLine();
            std::string name = sdl_texture.first;

            if ( !searchstr.empty() && name.find( searchstr ) == name.npos ) {
                /// search string is not in name, skip
                continue;
            };

            SDL_Texture* txtr = sdl_texture.second.txtr;
            /// Done twice for good looking
            /// images that are centered
            ImGui::SameLine();
            ImGui::SameLine();

            bool is_selected = sdl_texture.second.selected;

            if (ImGui_SelectableImage((ImTextureID)txtr, txtr_size, is_selected)) {
                if (ImGui::IsMouseClicked(0)) {
                    sdl_texture.second.selected = !sdl_texture.second.selected;
                    if (sdl_texture.second.selected) this->selected = &sdl_texture.second;
                };

                ImVec2 mousepos = ImGui::GetMousePos();
                ImGui::SetNextWindowPos(mousepos);

                ImGui::BeginTooltip();

                ImGui_CenteredText( (name + (sdl_texture.second.g_txtr.collidable ? " (no_collision)" : "")).c_str() );
                ImGui::Separator();
                ImGui_CenteredText(("width: "$(sdl_texture.second.g_txtr.width)", height: "$(sdl_texture.second.g_txtr.height)"").c_str());

                ImGui::EndTooltip();
            };


            i++;
        };

        ImGui::PopStyleColor();
        ImGui::EndChild();

        int selected_count = 0;
        std::vector< Gmeng::texture > textures;

        for ( auto& v : this->texture_map ) {
            if (v.second.selected) selected_count++, textures.push_back(v.second.g_txtr);
        };

        ImGui::Text(( v_str(selected_count) + " selected" ).c_str());

        ImGui::SameLine();

        if (ImGui::Button("add to level")) {
            ImGui::OpenPopup("chunk selection");
            popup_open = true;
        };

        if (popup_open)
            ImGui::PushStyleColor( ImGuiCol_PopupBg, IM_COL32( 40, 40, 40, 255 ) ),
            ImGui::SetNextWindowSize( ImVec2( 300, 150 ) );
        if (ImGui::BeginPopupModal("chunk selection")) {

            if (selected_chunk < 0) selected_chunk = this->ev->level->chunks.size()-1;
            if (selected_chunk >= this->ev->level->chunks.size()) selected_chunk = 0;

            auto m_chunk = this->ev->level->chunks.at(selected_chunk);

            ImGui::Separator();

            ImGui_CenteredText("Select a chunk to place vgm objects to");
            ImGui::SliderInt("chunk id", &selected_chunk, 0, ev->level->chunks.size()-1);
            ImGui::Separator();
            ImGui_CenteredText(std::string("viewpoint: " + v_str(m_chunk.vp.start.x) + ", " +
                                             v_str(m_chunk.vp.start.y) + " - " +
                                             v_str(m_chunk.vp.end.x) + ", " +
                                             v_str(m_chunk.vp.end.y)).c_str() );
            ImGui_CenteredText(("size: "$( Gmeng::vp_width( m_chunk.vp ) )"x" + v_str( Gmeng::vp_height( m_chunk.vp ))).c_str());
            ImGui::Separator();

            if ( ImGui::Button("cancel") ) {
                popup_open = false;
                ImGui::CloseCurrentPopup();
            };

            ImGui::SameLine();

            if ( ImGui::Button("add") ) {
                popup_open = false;
                ImGui::CloseCurrentPopup();

                int i = 0;
                for ( auto addition : textures ) {
                    Gmeng::Renderer::Model _usemodel;
                    _usemodel.attach_texture( addition );
                    _usemodel.position = { 0, 0 };
                    _usemodel.id = g_mkid() + i;

                    this->ev->level->chunks.at( selected_chunk ).models.push_back( _usemodel );
                    i++;
                };
            };

            ImGui::PopStyleColor();
            ImGui::EndPopup();

            textures.clear();
        };

        ImGui::End();
    };
};

struct ImGuiTextureSelectStatus {
    std::vector<Gmeng::texture> textures;
    bool finished = false;
    bool cancelled = false;
};

static bool textureselect_redo = true;
static std::vector< GmengImGuiVGM::VGMItem > gmeng_imgui_txtr_items;
static char imguitxtrsearchbuffer[256] = "";
static int imgui_txtr_select_count = 0;



static ImGuiTextureSelectStatus ImGui_TextureSelectionMenu(SDL_Renderer* renderer, std::string popupid, int max_items = 1) {
    if (textureselect_redo) {
        for ( auto& item : gmeng_imgui_txtr_items ) {
            SDL_DestroyTexture(item.txtr);
        };

        if ( !gmeng_imgui_txtr_items.empty() ) gmeng_imgui_txtr_items.clear();

        for ( auto& txtr : Gmeng::vgm_defaults::vg_textures.v_getrelative() ) {
            Gmeng::sImage source = { (int)txtr.data.width, (int)txtr.data.height };
            for ( auto un : txtr.data.units ) source.content.push_back( (Gmeng::color_t) un.color );

            Gmeng::sImage stretched = stretchImage( source, 100, 100 );

            gmeng_imgui_txtr_items.push_back(
                GmengImGuiVGM::VGMItem {
                make_texture( renderer, stretched ), txtr.data,
                Gmeng::nomdl, nullptr, false, false
            });
        };
        textureselect_redo = false;
    };

    if (ImGui::IsPopupOpen( popupid.c_str() ))
        ImGui::SetNextWindowSize( { 600, 400 } );
    if (ImGui::BeginPopupModal( popupid.c_str() )) {

        std::string searchstr = imguitxtrsearchbuffer;

        ImGui_CenteredText("select a texture");
        ImGui::Separator();

        ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x-50 );
        ImGui::InputText("search", imguitxtrsearchbuffer, IM_ARRAYSIZE( imguitxtrsearchbuffer ));
        ImGui::Separator();


        ImGui::PushStyleColor( ImGuiCol_ChildBg, IM_COL32( 143, 182, 190, 255 ) );
        ImGui::BeginChild("texture_viewport", {
                    ImGui::GetContentRegionAvail().x,
                    250
                }, true );

        ImVec2 txtr_size (100, 100);

        int i = 0;
        ImGui::SameLine();

        for ( auto& sdl_texture : gmeng_imgui_txtr_items ) {
            if (i != 0 && i % 5 == 0) ImGui::NewLine();
            std::string name = sdl_texture.g_txtr.name;

            if ( !searchstr.empty() && name.find( searchstr ) == name.npos ) {
                /// search string is not in name, skip
                continue;
            };

            SDL_Texture* txtr = sdl_texture.txtr;
            /// Done twice for good looking
            /// images that are centered
            ImGui::SameLine();
            ImGui::SameLine();

            bool is_selected = sdl_texture.selected;

            if (ImGui_SelectableImage((ImTextureID)txtr, txtr_size, is_selected)) {
                if (ImGui::IsMouseClicked(0)) {
                    if ( imgui_txtr_select_count < max_items ) {
                        sdl_texture.selected = true;
                        if (sdl_texture.selected) imgui_txtr_select_count++;
                        else imgui_txtr_select_count--;
                    } else if ( imgui_txtr_select_count >= max_items && sdl_texture.selected ) {
                        sdl_texture.selected = false;
                        imgui_txtr_select_count--;
                    };

                };

                ImVec2 mousepos = ImGui::GetMousePos();
                ImGui::SetNextWindowPos(mousepos);

                ImGui::BeginTooltip();

                ImGui_CenteredText( (name + (sdl_texture.g_txtr.collidable ? " (no_collision)" : "")).c_str() );
                ImGui::Separator();
                ImGui_CenteredText(("width: "$(sdl_texture.g_txtr.width)", height: "$(sdl_texture.g_txtr.height)"").c_str());

                ImGui::EndTooltip();
            };


            i++;
        };

        ImGui::PopStyleColor();
        ImGui::EndChild();


        ImGui::Spacing();
        ImGui::Separator();

        ImGui::Text((v_str(imgui_txtr_select_count) + " item(s) selected").c_str());
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 204, 36, 29, 255 ));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 244, 73, 52, 255 ));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 197, 79, 69, 255 ));

        if (ImGui::Button("cancel")) {
            ImGui::CloseCurrentPopup();
            ImGui::PopStyleColor(3);
            ImGui::EndPopup();
            return { { Gmeng::notxtr }, false, true };
        };

        ImGui::PopStyleColor(3);

        ImGui::SameLine();


        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 151, 150, 26, 255 ));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 184, 187, 38, 255 ));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 159, 192, 124, 255 ));

        if ( ImGui::Button("choose") ) {
            std::vector< Gmeng::texture > chosen;

            for ( auto texture : gmeng_imgui_txtr_items )
                if ( texture.selected ) chosen.push_back( texture.g_txtr );

            ImGui::CloseCurrentPopup();
            ImGui::PopStyleColor(3);
            ImGui::EndPopup();
            return { chosen, true, false };
        };

        ImGui::PopStyleColor(3);


        ImGui::EndPopup();
    };

    return { { Gmeng::notxtr }, false, false };
};


struct GmengImGuiInfoWindow {
    Gmeng::EventLoop* ev;
    GmengImGuiInfoWindow( Gmeng::EventLoop* _ev ) : ev(_ev) {
        __functree_call__(GmengImGuiInfoWindow::create);
    };

    SDL_Texture* preview_texture = nullptr;
    int preview_width = 0;
    int preview_height = 0;
    size_t preview_hash = 0;

    size_t hash_pixels(const std::vector<uint32_t>& pixels) {
        size_t h = 0;
        for (auto val : pixels) {
            h ^= std::hash<int>{}(val) + 0x9e3779b9 + (h << 6) + (h >> 2);
        }
        return h;
    }

    int graphg_ = 0;

    void Draw( SDL_Renderer* renderer ) {

        int w_, h_;
        SDL_GetRendererOutputSize(renderer, &w_, &h_);

        ImGui::SetNextWindowPos({ 960, 0 });
        ImGui::SetNextWindowSize({ 260, 540 });
        ImGui::Begin("game info", nullptr, ImGuiWindowFlags_NoResize);

        ImGui_CenteredText(std::string("gmeng " + Gmeng::version + " - build " + GMENG_BUILD_NO).c_str());

        ImGui::Separator();

        ImGui_CenteredText(std::string("frame: "$(ev->level->display.camera.frame_time)
                                "ms | draw: " + v_str(ev->level->display.camera.draw_time) +
                                "ms | FPS: "$(1000 / ev->level->display.camera.draw_time)"").c_str());
        ImGui::Separator();
        ev->frame_time_graph.at(graphg_ % 10) = ev->level->display.camera.frame_time;
        ImGui::SetNextItemWidth(245);
        ImGui::PlotHistogram("##", ev->frame_time_graph.data(), 10);

        ImGui_CenteredText(std::string("entity count: "$(ev->level->display.camera.entity_count)" | model count: " + v_str(ev->level->display.camera.model_count)).c_str());

        ImGui::Separator();

        ImGui_CenteredText("viewpoint");

        ImGui::SetNextItemWidth(85);
        ImGui::InputInt("x1    ", &ev->level->display.viewpoint.start.x);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(85);
        ImGui::InputInt("y1",&ev->level->display.viewpoint.start.y);

        ImGui::SetNextItemWidth(85);
        ImGui::InputInt("x2    ", &ev->level->display.viewpoint.end.x);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(85);
        ImGui::InputInt("y2", &ev->level->display.viewpoint.end.y);


        ImGui::Separator();

        ImGui_CenteredText("modifiers");

        ImVec4 col_red( 255, 0, 0, 255 );
        ImVec4 col_green( 0, 255, 0, 255 );
        ImVec4 col_yellow( 255, 255, 47, 255);

        for ( auto& modifier : ev->level->display.camera.modifiers.values ) {
            auto col = modifier.value == 0 ? col_red : ( modifier.value == 1 ? col_green : col_yellow );
            ImGui::TextColored( col, (ICON_FA_CUBE + modifier.name).c_str() );
            ImGui::SameLine();
            ImGui::SetNextItemWidth(260-ImGui::CalcTextSize(modifier.name.c_str()).x);
            ImGui::InputInt(("##" + modifier.name).c_str(), &modifier.value, 0, 0);
        };
        ImGui::Text(""); // end last SameLine

        ImGui::Separator();

        SDL_Texture* obj;

        ImGui::SetNextItemOpen(true);
        if (ImGui::CollapsingHeader("preview" )) {
            Gmeng::sImage img;
            img.width = ev->level->base.width;
            img.height = ev->level->base.height;
            img.content.clear();

            for (auto& un : ev->level->renderscale)
                if (!un.is_entity)
                    img.content.push_back((Gmeng::color_t) un.color);

            Gmeng::sImage preview = stretchImage(img, ImGui::GetWindowWidth() - 16, 200);

            ImGui_CenteredText("renderscale preview");

            size_t new_hash = hash_pixels(preview.content);

            // Check if texture needs to be recreated
            if (!preview_texture ||
                preview_width != preview.width ||
                preview_height != preview.height ||
                preview_hash != new_hash) {

                if (preview_texture) {
                    SDL_DestroyTexture(preview_texture);
                    preview_texture = nullptr;
                }

                preview_texture = make_texture(renderer, preview);
                preview_width = preview.width;
                preview_height = preview.height;
                preview_hash = new_hash;
            }

            if (preview_texture) {
                ImGui::Image((ImTextureID)preview_texture, ImVec2(preview_width, preview_height));
            }

            ImGui::Separator();
        };


        ImGui::End();
        graphg_++;
    };
};


struct GmengImGuiLog {
    const std::stringstream& external_stream;

    ImGuiTextFilter Filter;
    ImVector<int> LineOffsets;   // Line breaks offsets in current buffer
    bool ScrollToBottom;

    // Cache current string contents to avoid re-parsing every frame
    std::string cached_str;
    size_t last_size = 0;

    GmengImGuiLog(const std::stringstream& stream)
        : external_stream(stream), ScrollToBottom(false)
    {
        __functree_call__(GmengImGuiLog::create);
        RebuildLineOffsets();
    }

    void RebuildLineOffsets()
    {
        LineOffsets.clear();
        for (int i = 0; i < (int)cached_str.size(); i++)
            if (cached_str[i] == '\n')
                LineOffsets.push_back(i);
    }

    void UpdateBuffer()
    {
        std::string current = external_stream.str();
        if (current.size() != last_size)
        {
            cached_str = current;
            last_size = current.size();
            RebuildLineOffsets();
            ScrollToBottom = true;
        }
    }

    void Draw(const char* title, ImVec2 pos = { 0, 0 }, ImVec2 dsize = { 480, 375 }, bool* p_opened = nullptr)
    {
        UpdateBuffer();

        ImGui::SetNextWindowPos( pos );
        ImGui::SetNextWindowSize(dsize);
        ImGui::Begin(title, p_opened, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Clear"))
        {
            // Clear external stream by resetting it
            // Unfortunately std::stringstream has no clear, so we do this:
            const_cast<std::stringstream&>(external_stream).str("");
            const_cast<std::stringstream&>(external_stream).clear();
            cached_str.clear();
            LineOffsets.clear();
            last_size = 0;
        }

        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);
        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,0), false,
        ImGuiChildFlags_None | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,1));
        if (copy) ImGui::LogToClipboard();

        const char* buf_begin = cached_str.c_str();

        if (Filter.IsActive())
        {
            const char* line = buf_begin;
            int line_no = 0;
            while (line < buf_begin + cached_str.size())
            {
                const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : buf_begin + cached_str.size();

                if (Filter.PassFilter(line, line_end))
                    ImGui::TextUnformatted(line, line_end);

                if (line_end == buf_begin + cached_str.size())
                    break;

                line = line_end + 1;
                line_no++;
            }
        }
        else
        {
            ImGui::TextUnformatted(buf_begin);
        }

        if (ScrollToBottom)
            ImGui::SetScrollHereY(1.0f);

        ScrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::End();
    }
};



struct ScaledFontResult {
    TTF_Font* font = nullptr;
    float scale = 1.0f;
};


// Holds preloaded fonts for Gmeng
// for text scaling and drawing features
class G_Window_FontSelector {
public:
    void addFont(int size, TTF_Font* font) {
        fontMap[size] = font;
        sizes.push_back(size);
        std::sort(sizes.begin(), sizes.end());
    }

    /// Returns the best font object to use to draw
    /// a font at size \param requestedSize and the
    /// scaling factor
    ScaledFontResult getBestFont(int requestedSize) {
        ScaledFontResult result;
        if (sizes.empty()) return result;

        // Find smallest font size >= requestedSize (to scale down)
        int chosenSize = -1;
        for (int sz : sizes) {
            if (sz >= requestedSize) {
                chosenSize = sz;
                break; // sizes is sorted ascending, first >= requestedSize is smallest such font
            }
        }

        if (chosenSize == -1) {
            // No font >= requestedSize, pick the largest available font to scale up
            chosenSize = sizes.back();
        }

        TTF_Font* chosenFont = fontMap[chosenSize];
        float scaleFactor = static_cast<float>(requestedSize) / chosenSize;

        result.font = chosenFont;
        result.scale = scaleFactor;
        return result;
    }

private:
    std::map<int, TTF_Font*> fontMap;
    std::vector<int> sizes;
};


struct GmengImGuiConsole
{
    const std::stringstream& external_stream;   // Reference to your external stream

    ImGuiTextFilter Filter;
    ImVector<int> LineOffsets;   // Line breaks offsets in current buffer
    bool ScrollToBottom;

    // Cache current string contents to avoid re-parsing every frame
    std::string cached_str;
    size_t last_size = 0;

    GmengImGuiConsole(const std::stringstream& stream)
        : external_stream(stream), ScrollToBottom(false)
    {
        __functree_call__(GmengImGuiConsole::create);
        RebuildLineOffsets();
    }

    void RebuildLineOffsets()
    {
        LineOffsets.clear();
        for (int i = 0; i < (int)cached_str.size(); i++)
            if (cached_str[i] == '\n')
                LineOffsets.push_back(i);
    }

    void UpdateBuffer()
    {
        std::string current = external_stream.str();
        if (current.size() != last_size)
        {
            cached_str = current;
            last_size = current.size();
            RebuildLineOffsets();
            ScrollToBottom = true;
        }
    }

    void Draw(const char* title, Gmeng::EventLoop* ev, bool* p_opened = nullptr)
    {
        UpdateBuffer();

        ImGui::SetNextWindowPos({ 0, 540 });
        ImGui::SetNextWindowSize(ImVec2(480,375), ImGuiCond_FirstUseEver);
        ImGui::Begin(title, p_opened, ImGuiWindowFlags_NoResize |
                                                    ImGuiWindowFlags_NoCollapse);

        if (ImGui::Button("Clear"))
        {
            // Clear external stream by resetting it
            // Unfortunately std::stringstream has no clear, so we do this:
            const_cast<std::stringstream&>(external_stream).str("");
            const_cast<std::stringstream&>(external_stream).clear();
            cached_str.clear();
            LineOffsets.clear();
            last_size = 0;
        }

        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);


        ImGui::Separator();
        ImGui::BeginChild("scrolling", ImVec2(0,290), false,
        ImGuiChildFlags_None | ImGuiChildFlags_AlwaysUseWindowPadding | ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,1));
        if (copy) ImGui::LogToClipboard();

        const char* buf_begin = cached_str.c_str();

        if (Filter.IsActive())
        {
            const char* line = buf_begin;
            int line_no = 0;
            while (line < buf_begin + cached_str.size())
            {
                const char* line_end = (line_no < LineOffsets.Size) ? buf_begin + LineOffsets[line_no] : buf_begin + cached_str.size();

                if (Filter.PassFilter(line, line_end))
                    ImGui::TextUnformatted(line, line_end);

                if (line_end == buf_begin + cached_str.size())
                    break;

                line = line_end + 1;
                line_no++;
            }
        }
        else
        {
            ImGui::TextUnformatted(buf_begin);
        }

        if (ScrollToBottom)
            ImGui::SetScrollHereY(1.0f);

        ScrollToBottom = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::Separator();
        /// Command input
        char buffer[256] = "";
        ImGui::SetNextItemWidth(550);
        bool keep_focus = false;

        if (keep_focus)
            ImGui::SetKeyboardFocusHere();

        if (ImGui::InputText("run command", buffer, IM_ARRAYSIZE(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
        {
            int result = gmeng_run_dev_command(ev, std::string(buffer));
            if (result != 0) GAME_LOG("command returned nonzero " + v_str(result));
        } else {
            keep_focus = ImGui::IsItemActive();  // Update focus state
        };
        ImGui::End();
    }
};



namespace Gmeng {
    /// GameWindow Utility for SDL-Based EventLoops in Gmeng.
    /// MUST USE `make use-external <...params>` FOR THIS UTILITY TO
    /// BE INCLUDED IN YOUR APPLICATION.
    class GameWindow {
        SDL_Texture* cached_texture = nullptr;
        int cached_width = 0;
        int cached_height = 0;
        size_t cached_hash = 0;

        std::vector<uint32_t> pixelBuffer;  // Converted pixel data (RGBA packed)

        size_t hash_pixels(const std::vector<uint32_t>& pixels) {
            size_t h = 0;
            for (int val : pixels) {
                h ^= std::hash<int>{}(val) + 0x9e3779b9 + (h << 6) + (h >> 2);
            }
            return h;
        }
    public:

        int frame_width;
        int frame_height;
        // initializer for an SDL Window.
        GameWindow(const char* title =
                   std::string("GMENG " + (Gmeng::version) + " BUILD " + GMENG_BUILD_NO + " | " + Gmeng::global.raw_arguments).c_str(), int width = 800, int height = 800)
            : window(nullptr), renderer(nullptr), width(width), height(height), frame_width(width), frame_height(height) {

            __functree_call__(gmeng_external::__optional_utils__::libsdl2::GameWindow::__constructor__::GameWindow);

            if (IS_SET PREF("pref.max_screens_reached")) {
                gm_log("wont proceed with Window creation, max screens reached");
                return;
            };

            if (SDL_Init(SDL_INIT_VIDEO) < 0) {
                SDL_Log("SDL could not initialize! SDL_Error: %s", SDL_GetError());
                return;
            };

            if (TTF_Init() != 0) {
                throw std::runtime_error("TTF_Init Error: " + std::string(TTF_GetError()));
            };

            window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
                                      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
            if (!window) {
                SDL_Log("Window could not be created! SDL_Error: %s", SDL_GetError());
                SDL_Quit();
                return;
            };

            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer) {
                SDL_Log("Renderer could not be created! SDL_Error: %s", SDL_GetError());
                SDL_DestroyWindow(window);
                SDL_Quit();
                return;
            };

            std::string font_path = "assets/press_start.ttf";
            std::vector<int> font_sizes_to_open = {
                12, 18, 32, 64
            };

            for ( int size : font_sizes_to_open ) {
                TTF_Font* font = TTF_OpenFont(font_path.c_str(), size);
                if (!font) {
                    gm_log("ERROR Opening TTF Font: " + std::string(TTF_GetError()));
                };
                // pushes the font
                fonts.addFont(size, font);
            };

            // gmeng.log logger
            default_windows.push_back(
                GmengImGuiLog(Gmeng::logstream)
            );
            // Functree logger
            default_windows.push_back(
                GmengImGuiLog(Gmeng::functree_calls)
            );
        };

        ~GameWindow() {
            __functree_call__(gmeng_external::__optional_utils__::libsdl2::GameWindow::__destroy__);
            TTF_Quit();
            if (renderer) SDL_DestroyRenderer(renderer);
            if (window) SDL_DestroyWindow(window);
            SDL_Quit();
        };

        void InitTexture(int width, int height) {
            if (cached_texture) {
                SDL_DestroyTexture(cached_texture);
                cached_texture = nullptr;
            }
            cached_width = width;
            cached_height = height;
            cached_texture = SDL_CreateTexture(renderer,
                                         SDL_PIXELFORMAT_RGBA8888,
                                         SDL_TEXTUREACCESS_STREAMING,
                                         cached_width,
                                         cached_height);
            if (!cached_texture) {
                SDL_Log("Failed to create texture: %s", SDL_GetError());
            }
            pixelBuffer.resize(cached_width * cached_height);
        }

        // Default Gmeng ImGui Child definition
        template<typename ImGuiHandler> struct DefaultImGuiChild_t {
            ImGuiHandler handler;
            bool enabled = true;

            void disable() { this->enabled = false; };
            void enable()  { this->enabled = true;  };
        };
        /// Map of all Default Gmeng ImGui windows.
        /// can be disabled by child.disable();
        std::vector<GmengImGuiLog> default_windows;

        using sdlevent_handler_type = std::function<void(SDL_Event*)>;
        /// user defined SDL Event handler
        /// optional, not required
        sdlevent_handler_type external_sdl_event_handler = NULL;

        /// Defines an external SDL Handler that the user
        /// can specify, allowing a game to handle SDL events
        /// seperately from the engine.
        void sdl_handler(sdlevent_handler_type callback) {
            this->external_sdl_event_handler = callback;
        };

        /// draws a gmeng source image file to the SDL Renderer.
    void draw(const Gmeng::sImage& img, SDL_Point position, int pixelSize = 15) {
        // Calculate hash of the color indices
        size_t new_hash = hash_pixels(img.content);

        // Check if texture needs recreating
        bool need_recreate = false;
        if (!cached_texture) need_recreate = true;
        else if (img.width != cached_width || img.height != cached_height) need_recreate = true;
        else if (new_hash != cached_hash) need_recreate = true;

        if (need_recreate) {
            // Convert color indices to packed pixels
            pixelBuffer.resize(img.width * img.height);
            for (size_t i = 0; i < img.content.size(); ++i) {
                auto v = color32_t( img.content.at(i) );
                SDL_Color color = { v.r, v.g, v.b, 255 };
                pixelBuffer[i] = color_to_uint32(color);
            }

            if (cached_texture) {
                SDL_DestroyTexture(cached_texture);
                cached_texture = nullptr;
            }

            // Create SDL_Texture from pixelBuffer
            cached_texture = SDL_CreateTexture(renderer,
                                              SDL_PIXELFORMAT_RGBA8888,
                                              SDL_TEXTUREACCESS_STATIC,
                                              img.width,
                                              img.height);
            if (!cached_texture) {
                SDL_Log("Failed to create texture: %s", SDL_GetError());
                return;
            }

            // Upload pixel data to texture
            SDL_UpdateTexture(cached_texture, nullptr, pixelBuffer.data(), img.width * sizeof(uint32_t));

            cached_width = img.width;
            cached_height = img.height;
            cached_hash = new_hash;
        }

        SDL_Rect src_rect = {0, 0, img.width, img.height};
        SDL_Rect dst_rect = {position.x, position.y, img.width * pixelSize, img.height * pixelSize};
        SDL_RenderCopy(renderer, cached_texture, &src_rect, &dst_rect);
    }
        /// Draws the main camera viewpoint of any Gmeng Camera
        /// that has been converted to a source image with auto
        /// pixel scaling based on the window size.
        void main_camera(const Gmeng::sImage& img) {
            this->draw( img, {0, 0}, 1);
        };

        /// draws text to the SDL Renderer.
        /// INEFFICIENT!
        void text(std::string message, int size, SDL_Point pos, SDL_Color color, SDL_Color bgcolor = {0,0,0,0}) {
            // Get best font and scale factor
            ScaledFontResult res = this->fonts.getBestFont(size);
            if (!res.font) {
                gm_log("failed to load font: " + std::string(TTF_GetError()));
                return;
            };

            DEBUGGER gm_log("Rendering '" + message + "' at size " + std::to_string(size) +
               " (chosen font size " + std::to_string(int(size / res.scale)) +
               ") with scale " + std::to_string(res.scale));

            // 2. Render text surface (unscaled)
            SDL_Surface* textSurface = TTF_RenderUTF8_Blended(res.font, message.c_str(), color);
            if (!textSurface) {
                gm_log("TTF_RenderUTF8_Blended failed: " + std::string(TTF_GetError()));
                return;
            }
            DEBUGGER gm_log("Text surface size: " + std::to_string(textSurface->w) + " x " + std::to_string(textSurface->h));

            // Optionally render background rect if alpha > 0
            if (bgcolor.a > 0) {
                SDL_Rect bgRect = { pos.x, pos.y,
                    std::max(1, static_cast<int>(textSurface->w * res.scale)),
                    std::max(1, static_cast<int>(textSurface->h * res.scale)) };
                SDL_SetRenderDrawColor(this->renderer, bgcolor.r, bgcolor.g, bgcolor.b, bgcolor.a);
                SDL_RenderFillRect(this->renderer, &bgRect);
            }

            // Create texture from surface
            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(this->renderer, textSurface);
            if (!textTexture) {
                gm_log("SDL_CreateTextureFromSurface failed: " + std::string(SDL_GetError()));
                return;
            }

            // Set destination rect, clamping to minimum size 1
            SDL_Rect dstRect = {
                pos.x,
                pos.y,
                std::max(1, static_cast<int>(textSurface->w * res.scale)),
                std::max(1, static_cast<int>(textSurface->h * res.scale))
            };

            DEBUGGER gm_log("Rendering texture at x=" + std::to_string(dstRect.x) +
                   ", y=" + std::to_string(dstRect.y) +
                   ", w=" + std::to_string(dstRect.w) +
                   ", h=" + std::to_string(dstRect.h));
            // Copy texture to renderer
            SDL_RenderCopy(this->renderer, textTexture, nullptr, &dstRect);

            // Cleanup
            SDL_FreeSurface(textSurface);
            SDL_DestroyTexture(textTexture);
        }

        /// Presents current SDL Renderer to the Window.
        void refresh() {
            SDL_RenderPresent(renderer);
        };

        /// Clears the SDL Renderer.
        void clear() {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
        };

        G_Window_FontSelector fonts;
        SDL_Window* window;
        SDL_Renderer* renderer;
        int width, height;
    };

    inline GameWindow create_window(const char* title, int width, int height) {
        __functree_call__(gmeng_external::__optional_utils__::libsdl2::GameWindow::create_window);
        gm_log("[SDL_FRAME] window creation");
        if (IS_SET PREF("pref.max_screens_reached")) {
            gm_log("[SDL_FRAME] max screens reached assertion is jWRAP::ON, set to OFF or NOT_SET to create this screen. Cancelling GameWindow[constructor]->e");
            return GameWindow("UNUSABLE_WINDOW",0,0);
        };
        if (IS_SET PREF("pref.screens")) gm_log("[SDL_FRAME] a previous window was already created, possible loop\n assert `pref.max_screens_reached` to ON to disable creating more windows");
        ASSERT("pref.screens", Assertions::vd_assert::ON);
        GameWindow w(title, width, height);
        return w;
    };
};


void ImGui_VerticalSeparator(float thickness = 1.0f, float spacing = 8.0f, ImU32 color = IM_COL32(150, 150, 150, 255))
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    float height = ImGui::GetContentRegionAvail().y;

    ImGui::GetWindowDrawList()->AddLine(
        ImVec2(p.x + spacing * 0.5f - thickness * 0.5f, p.y),
        ImVec2(p.x + spacing * 0.5f - thickness * 0.5f, p.y + height),
        color,
        thickness
    );

    ImGui::Dummy(ImVec2(spacing, height));
    ImGui::SameLine();
}

bool ImGui_SelectableText(const char* label, bool& selected, ImU32 clicked_color = IM_COL32(100, 200, 255, 255))
{
    bool activated = false;

    ImVec2 text_size = ImGui::CalcTextSize(label);
    ImVec2 padding = ImGui::GetStyle().FramePadding;
    ImVec2 item_size = ImVec2(text_size.x + padding.x * 2, text_size.y + padding.y * 2);

    ImGui::PushID(label);
    ImGui::InvisibleButton("##clickable", item_size);
    ImGui::PopID();

    ImVec2 p = ImGui::GetItemRectMin();
    ImVec2 q = ImGui::GetItemRectMax();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    if (ImGui::IsItemHovered())
    {
        draw->AddRectFilled(p, q, IM_COL32(200, 200, 200, 100)); // Hover highlight
    }

    if (ImGui::IsItemClicked())
    {
        selected = !selected;
        activated = true;
    }

    if (selected)
    {
        draw->AddRectFilled(p, q, clicked_color); // Selected background
    }

    // Compute text position manually (top-left + padding)
    ImVec2 text_pos = ImVec2(p.x + padding.x, p.y + padding.y);
    draw->AddText(text_pos, ImGui::GetColorU32(ImGuiCol_Text), label);

    return activated;
}


struct GmengImGuiLevel {
    SDL_Renderer* renderer;
    Gmeng::EventLoop* ev;

    bool redo_cache = false;

    GmengImGuiLevel( SDL_Renderer* _renderer, Gmeng::EventLoop* _ev ) :
        renderer( _renderer ), ev( _ev ) {
            __functree_call__(GmengImGuiLevel::create);
        };

    int selected_chunk = 0;
    bool enable_popup = false;

    std::vector< GmengImGuiVGM::VGMItem > items;
    GmengImGuiVGM::VGMItem* selected_item = nullptr;

    char objSearchBuffer[256] = "";

    void Draw() {
        if (selected_chunk >= ev->level->chunks.size()) selected_chunk = 0;
        if (selected_chunk < 0) selected_chunk = ev->level->chunks.size()-1;

        auto& cur_chunk = ev->level->chunks.at( selected_chunk );

        if ( redo_cache || ( items.size() != cur_chunk.models.size() + ev->level->entities.size() ) ) {
            redo_cache = false;

            for ( auto& item : items ) {
                SDL_DestroyTexture( item.txtr );
            };
            GmengImGuiVGM::VGMItem empty_item = {};

            auto prev_id = selected_item != nullptr ? (
                selected_item->is_entity ? selected_item->entity->entity_id : selected_item->model.id)
                : -1;

            items.clear();

            for ( auto& entity : ev->level->entities ) {
                bool _selected = false;
                /// previously selected item was this item, set it again
                if (prev_id == entity->entity_id) _selected = true;

                items.push_back(GmengImGuiVGM::VGMItem {
                    nullptr, entity->sprite, Gmeng::nomdl, entity, _selected, true
                });

            };

            for ( auto& model : cur_chunk.models ) {
                auto txtr = model.texture;
                Gmeng::sImage source = { (int)txtr.width, (int)txtr.height };
                for ( auto un : txtr.units ) source.content.push_back( (Gmeng::color_t)un.color );

                auto stretched = stretchImage( source, 100, 100 );
                bool _selected = false;

                /// previously selected item was this item, set it again
                if (prev_id == model.id) _selected = true;
                items.push_back( GmengImGuiVGM::VGMItem {
                    make_texture( renderer, stretched ), txtr, model, nullptr, _selected, false
                } );

            };
        };

        ImGui::SetNextWindowPos({ 1220, 0 });
        ImGui::SetNextWindowSize( { 700, 540 } );
        ImGui::Begin("level inspector", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
        ImGui_CenteredText( ( "level '" + (
                                ev->level->name.empty() ? Gmeng::global.executable : ev->level->name) + "' (chunk " + v_str(selected_chunk) + ")" ).c_str() );
        ImGui::Separator();

        ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x-87 );
        ImGui::InputInt("chunk select", &selected_chunk);

        ImGui::Separator();

        ImGui::BeginChild("sidebar", ImVec2(275, 0), true);
        ImGui_CenteredText(("chunk " + v_str(selected_chunk) + " | models: "$(cur_chunk.models.size())"").c_str());
        ImGui::Separator();

        ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x-45 );
        ImGui::InputText("search", objSearchBuffer, IM_ARRAYSIZE(objSearchBuffer));

        ImGui::Separator();

        ImGui::Dummy({ -2.5, 0 });
        ImGui::SameLine();
        ImGui::SetNextItemWidth( ImGui::CalcTextSize("[M]").x+3 );
        if (ImGui::Button("+")) {
            *ev->states.vgm_open = true;
        };
        ImGui::SameLine();
        ImGui::Text("add object");

        std::string searchstr_ = objSearchBuffer;

        for ( auto& item : items ) {
            /// item label
            std::string text =
                        (item.is_entity ?
                        ( ICON_FA_BOOK + item.entity->sprite.name + " (" + v_str(item.entity->entity_id) + ")" + " (" + _uconv_1ihx( item.entity->entity_id ) + ")" )
                        : ICON_FA_CUBE + std::string( &item.model.name[0] ) + std::string(":") + item.g_txtr.name + " (" + v_str( item.model.id ) +
                        ") (" + (item.model.texture.collidable ? "C" : "A") +
                        ") (" + _uconv_1ihx( item.model.id ) + ")" ).c_str();
            /// filter item if it doesn't include the search string
            if ( !searchstr_.empty() && text.find( searchstr_ ) == text.npos ) continue;

            if ( ImGui_SelectableText( text.c_str(), item.selected ) ) {
                for ( auto& _item : items )
                    _item.selected = false;

                item.selected = true; /// only set this item to 'selected'
                selected_item = &item; /// set the selected item
            };
        };


        ImGui::EndChild();

        ImGui::SameLine();


        bool model_deleted = false;

        if (selected_item == nullptr) {
            ImGui::BeginChild("view", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y/2 });
            ImGui_CenteredText("no item selected");
            ImGui::EndChild();
        } else {
            ImGui::BeginChild("view", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y/2 });

            if ( !selected_item->is_entity ) {

                ImGui_CenteredText( (
                    &selected_item->model.name[0] + std::string("::") + selected_item->g_txtr.name + " (" + v_str(selected_item->model.id) + ") (" + _uconv_1ihx( selected_item->model.id ) + ")"
                ).c_str() );
                ImGui::Separator();

                selected_item->model.name.resize(128);
                ImGui::SetNextItemWidth( ImGui::GetContentRegionAvail().x-50 );
                if (ImGui::InputText("rename", &selected_item->model.name[0], (int)selected_item->model.name.size())) {
                    selected_item->model.name.resize(strlen(&selected_item->model.name[0]));
                };

                ImGui::Separator();

                ImGui::SetNextItemWidth( 150 );
                ImGui::InputInt( "pos.x", &selected_item->model.position.x );
                ImGui::SameLine();
                ImGui::SetNextItemWidth( 150 );
                ImGui::InputInt( "pos.y", &selected_item->model.position.y );

                ImGui::Checkbox("selected", &selected_item->selected);
                ImGui::SameLine();
                ImGui::Checkbox("active", &selected_item->model.active);
                ImGui::SameLine();
                ImGui::Checkbox("highlight", &selected_item->model.highlighted);
                ImGui::SameLine();
                ImGui::Checkbox("collidable", &selected_item->model.texture.collidable);

                ImGui::Separator();

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 204, 36, 29, 255 ));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 244, 73, 52, 255 ));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 197, 79, 69, 255 ));

                if (ImGui::Button("reset texture")) {
                    selected_item->model.reset_texture();
                    redo_cache = true;
                };

                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 151, 150, 26, 255 ));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 184, 187, 38, 255 ));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 159, 192, 124, 255 ));

                if (ImGui::Button("attach texture")) {
                    ImGui::OpenPopup(("attach texture: " + selected_item->g_txtr.name).c_str());
                    enable_popup = true;
                };

                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                if (ImGui::Button("reverse texture")) {
                    selected_item->model.texture.units = Gmeng::mirror( selected_item->model.texture.units, true );
                    redo_cache = true;
                };

                ImGui::SameLine();

                if (ImGui::Button("mirror")) {
                    flip_vector( selected_item->model.texture.units, selected_item->model.texture.width, selected_item->model.texture.height );
                };

                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 204, 36, 29, 255 ));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 244, 73, 52, 255 ));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 197, 79, 69, 255 ));
                if (ImGui::Button("remove object")) {
                    selected_item->selected = false;
                    redo_cache = true;
                    model_deleted = true;
                };

                ImGui::PopStyleColor(3);

                ImGui::SameLine();

                if (ImGui::Button("duplicate")) {
                    auto nmdl = selected_item->model;
                    nmdl.id = g_mkid();
                    nmdl.position = { selected_item->model.position.x+1,
                                      selected_item->model.position.y+1 };
                    cur_chunk.models.push_back( nmdl );
                    selected_item->selected = false;
                };

                auto selections = ImGui_TextureSelectionMenu(renderer, "attach texture: " + selected_item->g_txtr.name, 1);

                if (selections.cancelled) enable_popup = false;
                if (selections.finished && !selections.cancelled) {
                    selected_item->model.attach_texture( selections.textures.at(0) );
                    redo_cache = true;
                };
            } else {
                auto entity_name = Gmeng::get_entity_name(selected_item->entity->get_serialization_id());

                ImGui_CenteredText(
                        ( "Entity<" + entity_name + ">(" + v_str( selected_item->entity->get_serialization_id() ) + ")::" + v_str(selected_item->entity->entity_id) ).c_str()
                );
                ImGui::Separator();

                if ( selected_item->entity->get_serialization_id() == Gmeng::SERIALIZED_ENGINE_INFO::id ) {
                    // engine info
                    std::shared_ptr<Gmeng::SERIALIZED_ENGINE_INFO> derived_entity = std::dynamic_pointer_cast<Gmeng::SERIALIZED_ENGINE_INFO>( selected_item->entity );                    // 

                    if (!derived_entity)
                        ImGui::TextColored(ImVec4(0.7, 0, 0, 255 ), "entity_error: dynamic_pointer_cast to SERIALIZED_ENGINE_INFO from EntityBase failed");
                    else {
                        ImGui_CenteredText("Serialized Engine Information of This Level File");
                        ImGui_CenteredText("  Gmeng engine information is only updated/embedded\nwhen the command 'embed_engine' is ran in the console.");
                        ImGui::Separator();
                        ImGui_CenteredText(("Build No: " + derived_entity->build + " | Engine Version: " + derived_entity->version).c_str());

                        if ( ImGui::Button("update data") ) {
                            gmeng_run_dev_command(ev, "embed_engine");
                        };

                        ImGui::Separator();
                    };
                };

                if ( selected_item->entity->get_serialization_id() == Gmeng::LightSource::id ) {
                    // lightsource
                    std::shared_ptr<Gmeng::LightSource> derived_entity = std::dynamic_pointer_cast<Gmeng::LightSource>( selected_item->entity );

                    if (!derived_entity)
                        ImGui::TextColored(ImVec4(0.7, 0, 0, 255 ), "entity_error: dynamic_pointer_cast to LightSource from EntityBase failed");
                    else {
                        /// lightsource-only properties here
                        ImGui_CenteredText(
                        ("lightsource cached pos: " + Gmeng::Renderer::conv_dp( derived_entity->cached_position )).c_str());

                        ImGui::SetNextItemWidth(150);
                        ImGui::InputInt("pos.x", &derived_entity->position.x);
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(150);
                        ImGui::InputInt("pos.y", &derived_entity->position.y);

                        ImGui::SetNextItemWidth(150);
                        ImGui::InputInt("intensity", &derived_entity->intensity);
                        ImGui::SameLine();
                        ImGui::SetNextItemWidth(150);
                        ImGui::InputInt("max brightness", &derived_entity->max_brightness);
                        float color_values[3] = { derived_entity->color.r / 255.0f, derived_entity->color.g / 255.0f, derived_entity->color.b / 255.0f, };
                        if (ImGui::ColorEdit3("modulation color", color_values)) {
                            Gmeng::color32_t new_col( color_values[0]*255, color_values[1]*255, color_values[2]*255 );
                            derived_entity->color = new_col;
                        };

                        if (ImGui::Button("invalidate cache")) {
                            derived_entity->cached = false;
                        };

                        ImGui::SameLine();
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 204, 36, 29, 255 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 244, 73, 52, 255 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 197, 79, 69, 255 ));
                        if (ImGui::Button("remove entity")) {
                            selected_item->selected = false;
                            redo_cache = true;

                            vector<std::shared_ptr<Gmeng::EntityBase>> new_entity_vector;
                            for (auto entity : ev->level->entities) {
                                if (entity->entity_id != selected_item->entity->entity_id)
                                    new_entity_vector.push_back(entity);
                            };

                            ev->level->entities.swap( new_entity_vector );
                            gmeng_run_dev_command(ev, "lighting_redoall", true);
                        };
                        ImGui::PopStyleColor(3);
                    };
                } else {
                    ImGui::SetNextItemWidth(150);
                    ImGui::InputInt("pos.x", &selected_item->entity->position.x);
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(150);
                    ImGui::InputInt("pos.y", &selected_item->entity->position.y);

                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 204, 36, 29, 255 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 244, 73, 52, 255 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 197, 79, 69, 255 ));
                        if (ImGui::Button("remove entity")) {
                            selected_item->selected = false;
                            redo_cache = true;

                            vector<std::shared_ptr<Gmeng::EntityBase>> new_entity_vector;
                            for (auto entity : ev->level->entities) {
                                if (entity->entity_id != selected_item->entity->entity_id)
                                    new_entity_vector.push_back(entity);
                            };

                            ev->level->entities.swap( new_entity_vector );
                            gmeng_run_dev_command(ev, "lighting_redoall", true);
                        };
                        ImGui::PopStyleColor(3);
                };
            };

            ImGui::EndChild();

            for ( auto& mdl : cur_chunk.models ) {
                if (mdl.id == selected_item->model.id) mdl = selected_item->model;
            };

            if ( model_deleted ) {
                gm_log("model deletion: " + selected_item->model.name + ":" + v_str(selected_item->model.id));
                std::vector<Gmeng::Renderer::Model> mdl_list;
                for ( auto mdl : cur_chunk.models )
                    if (mdl.id != selected_item->model.id) mdl_list.push_back( mdl );
                cur_chunk.models.swap( mdl_list );
            };

            if (!selected_item->selected) selected_item = nullptr;
        };

        ImGui::End();
    };
};


    void GmengImGuiTextureEditor::ResizeTexture(std::size_t new_w, std::size_t new_h) {
        if (new_w < 1) new_w = 1;
        if (new_h < 1) new_h = 1;

        std::vector<Gmeng::Unit> new_units(new_w * new_h, Gmeng::Unit { .transparent = true });

        // Safely copy old data into the new buffer where overlapping
        for (std::size_t y = 0; y < std::min(tex.height, new_h); ++y) {
            for (std::size_t x = 0; x < std::min(tex.width, new_w); ++x) {
                new_units[y * new_w + x] = tex.units[y * tex.width + x];
            }
        }

        tex.width = new_w;
        tex.height = new_h;
        tex.units = new_units;

        zoom = (ImGui::GetContentRegionAvail().x + 10) / tex.width;
    }

    void GmengImGuiTextureEditor::Save(const std::string& fname) {
        std::ofstream file(fname);
        if (!file.is_open()) return;

        // Write Header matching parser's expected layout
        file << "name=" << tex.name
             << ",width=" << tex.width
             << ",height=" << tex.height
             << ",collision=" << (tex.collidable ? "true" : "false") << "\n";

        // Write Units line-by-line
        for (const auto& unit : tex.units) {
            if (unit.transparent) {
                file << "__gtransparent_unit__\n";
            } else {
                // Ensure the special char isn't empty, otherwise g_splitStr might misalignment it upon reloading
                std::string s_char = unit.special_c_unit.empty() ? "X" : unit.special_c_unit;
                file << unit.color << " "
                     << (unit.special ? "true" : "false") << " "
                     << unit.special_clr << " "
                     << s_char << "\n";
            }
        }

        // Add the closing builder signature comment
        file << ";%tsgmeng::builder.texture4_0() -> auto_generated\n";
        file.close();
        gm_log("wrote " + fname + " texturedata (" $(tex.units.size()) " units)");
    }

    void GmengImGuiTextureEditor::Load(const std::string& fname) {
        // Delegate reading entirely to engine's parsing logic
        if (filesystem::exists( fname ) && !filesystem::is_directory(fname))
            tex = Gmeng::LoadTexture(fname), loaded = true;

        /// palette finder
        texture_palette.clear();
        for ( auto un : tex.units ) {
            if ( !texture_palette.contains( un.color ) ) texture_palette.insert( un.color );
        };
    }

void GmengImGuiTextureEditor::Draw() {
        if (!is_open) return;

        ImGui::SetNextWindowSize({ 860, 550 });
        ImGui::Begin("Gmeng Texture Editor", &is_open, ImGuiWindowFlags_NoResize);

        ImGui_CenteredText(("texture editor for Gmeng Game Development Kit " + Gmeng::version + " build " + GMENG_BUILD_NO).c_str());
        // ==========================================
        // --- LEFT PANEL: Modifiers ---
        // ==========================================
        // Create a bordered child panel with a fixed width of 350. '0' height means it fills the window vertically.
        ImGui::BeginChild("LeftSettingsPanel", ImVec2(350, 0), true);

        // --- File I/O ---
        char fileBuf[256];
        strncpy(fileBuf, current_filename.c_str(), sizeof(fileBuf));
        if (ImGui::InputText("Target File", fileBuf, sizeof(fileBuf))) {
            current_filename = fileBuf;
        }

        if (ImGui::Button("Save")) Save(current_filename);
        ImGui::SameLine();
        if (ImGui::Button("Load")) Load(current_filename);
        ImGui::Separator();

        // --- Texture Properties ---
        char nameBuf[256];
        strncpy(nameBuf, tex.name.c_str(), sizeof(nameBuf));
        if (ImGui::InputText("Texture Name", nameBuf, sizeof(nameBuf))) {
            tex.name = nameBuf;
        }

        // Convert size_t to int for ImGui manipulators
        int size[2] = { static_cast<int>(tex.width), static_cast<int>(tex.height) };
        if (ImGui::InputInt2("Size (W/H)", size)) {
            ResizeTexture(static_cast<std::size_t>(size[0]), static_cast<std::size_t>(size[1]));
        }

        ImGui::Checkbox("collision", &tex.collidable);
        ImGui::Separator();

        // --- Brush Settings ---
        ImGui_CenteredText("Brush Settings");
        ImGui::Checkbox("transparent", &current_brush.transparent);

        if (!current_brush.transparent) {
            // Edit the brush color using ImGui's floating color picker
            if (ImGui::ColorEdit3("color", brush_color_float)) {
                // Instantly convert ImGui floats back into your engine's color32_t / uint32_t
                Gmeng::color32_t new_color(
                    static_cast<uint8_t>(brush_color_float[0] * 255.0f),
                    static_cast<uint8_t>(brush_color_float[1] * 255.0f),
                    static_cast<uint8_t>(brush_color_float[2] * 255.0f)
                );
                current_brush.color = Gmeng::uint32_from_color32(new_color);
            }

            ImGui::Separator();
            ImGui_CenteredText("Special Settings");
            ImGui::Checkbox("enabled", &current_brush.special);
            ImGui::InputInt("color", &current_brush.special_clr);

            char charBuf[16];
            strncpy(charBuf, current_brush.special_c_unit.c_str(), sizeof(charBuf));
            if (ImGui::InputText("character", charBuf, sizeof(charBuf))) {
                current_brush.special_c_unit = charBuf;
            }
            ImGui::Separator();

            int brush_size = 52;
            float b_zoom_item_height = ImGui::GetFrameHeight();
            float offset = (texture_palette.size()/6.0f) * (brush_size + 2);

            ImGui::BeginChild("palette", {0, ImGui::GetContentRegionAvail().y + ( offset )}, false, ImGuiWindowFlags_NoScrollbar);


            ImGui_CenteredText(("texture palette (" + v_str(texture_palette.size()) + ")").c_str());
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            int i = 0;

            ImVec2 c_pos = ImGui::GetCursorScreenPos();

            for ( auto brush : texture_palette ) {
                Gmeng::color32_t rgb( brush );

                ImVec2 p_min( ( (i % 6)*2 ) + c_pos.x + (i % 6) * brush_size    , ( (int)(i / 6)*2 ) + ((c_pos.y) + ((i / 6.0f))*brush_size) - (i % 6)*8.6658 );
                ImVec2 p_max( ( (i % 6)*2 ) + c_pos.x + ((i % 6)+1) * brush_size, ( (int)(i / 6)*2 ) + c_pos.y + ((i / 6.0f)+1) * brush_size - (i % 6)*8.6658 );

                if (ImGui_ClickableRect((v_str(i) + "_colord").c_str(), p_min, p_max, IM_COL32( rgb.r, rgb.g, rgb.b, 255 ),
                                                                                 IM_COL32( 255-rgb.r, 255-rgb.g, 255-rgb.b, 255 ),
                                                                                 IM_COL32( rgb.r-20, rgb.g-20, rgb.b-20, 255 ) )) {
                    current_brush.color = uint32_from_color32(rgb);
                    brush_color_float[0] = rgb.r / 255.0f;
                    brush_color_float[1] = rgb.g / 255.0f;
                    brush_color_float[2] = rgb.b / 255.0f;
                };

                // outline the item if its selected
                if ( current_brush.color == uint32_from_color32(rgb) ) ImGui::SetNextItemAllowOverlap(),
                    draw_list->AddRect(p_min, p_max, IM_COL32( 255-rgb.r, 255-rgb.g, 255-rgb.b, 155 ), 0, 0, 3);

                i++;
            };
            ImGui::EndChild();

        }
        ImGui::EndChild(); // End Left Panel

        ImGui::SameLine(); // Move to the right for the next panel

        // ==========================================
        // --- RIGHT PANEL: Workspace (Canvas & Zoom) ---
        // ==========================================
        // This child takes the remaining width and full height
        ImGui::BeginChild("RightWorkspacePanel", ImVec2(0, 0), false);

        // Pre-calculate the height of the zoom slider so we know how much space to leave at the bottom
        float zoom_item_height = ImGui::GetFrameHeightWithSpacing();

        // --- Interactive Canvas Area ---
        // We put the canvas in its own scrollable child window. It takes all vertical space MINUS the zoom slider.
        ImGui::BeginChild("CanvasRegion", ImVec2(0, ImGui::GetContentRegionAvail().y - zoom_item_height), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
        ImVec2 canvas_size(tex.width * zoom, tex.height * zoom);
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Draw the background border for the canvas
        draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(30, 30, 30, 255));

        // Invisible button acts as a hit-box mapping mouse interactions
        ImGui::InvisibleButton("canvas_interact", canvas_size);

        // Painting Logic
        if (ImGui::IsItemActive() && (ImGui::IsMouseDown(ImGuiMouseButton_Left) || ImGui::IsMouseDragging(ImGuiMouseButton_Left))) {
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;
            int tx = static_cast<int>((mouse_pos.x - canvas_pos.x) / zoom);
            int ty = static_cast<int>((mouse_pos.y - canvas_pos.y) / zoom);
            if (tx >= 0 && tx < static_cast<int>(tex.width) && ty >= 0 && ty < static_cast<int>(tex.height)) {
                tex.units[ty * tex.width + tx] = current_brush;
                if ( !texture_palette.contains( current_brush.color ) ) texture_palette.insert( current_brush.color );
            }
        }

        // Render Canvas
        for (std::size_t y = 0; y < tex.height; ++y) {
            for (std::size_t x = 0; x < tex.width; ++x) {
                ImVec2 p_min(canvas_pos.x + x * zoom, canvas_pos.y + y * zoom);
                ImVec2 p_max(canvas_pos.x + (x + 1) * zoom, canvas_pos.y + (y + 1) * zoom);

                const Gmeng::Unit& unit = tex.units[y * tex.width + x];
                std::string coordinate_str( v_str(x) + "," + v_str(y));

                if (unit.transparent) {
                    // Render checkerboard pattern for transparent cells
                    ImU32 checker_col = ((x + y) % 2 == 0) ? IM_COL32(50, 50, 50, 255) : IM_COL32(40, 40, 40, 255);
                    draw_list->AddRectFilled(p_min, p_max, checker_col);
                    draw_list->AddLine(p_min, p_max, IM_COL32(200, 50, 50, 150), 2.0f);

                    if ( zoom >= 30.0f ) {
                        ImVec2 text_size = ImGui::CalcTextSize(coordinate_str.c_str() );
                        ImVec2 text_pos(p_min.x + (zoom - text_size.x) * 0.5f, p_min.y + (zoom - text_size.y) * 0.5f);
                        draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), coordinate_str.c_str());
                    };
                } else {
                    // Convert your uint32_t color back to color32_t for drawing
                    Gmeng::color32_t rgb(unit.color);
                    ImU32 cell_color = IM_COL32(rgb.r, rgb.g, rgb.b, 255);
                    draw_list->AddRectFilled(p_min, p_max, cell_color);

                    if ( zoom >= 30.0f ) {
                        ImVec2 text_size = ImGui::CalcTextSize(coordinate_str.c_str() );
                        ImVec2 text_pos(p_min.x + (zoom - text_size.x) * 0.5f, p_min.y + (zoom - text_size.y) * 0.5f);
                        draw_list->AddText(text_pos, IM_COL32(255 - rgb.r, 255 - rgb.g, 255 - rgb.b, 255), coordinate_str.c_str());
                    };

                    // If special flags are raised and zoom allows, show the special char
                    if (unit.special && zoom >= 15.0f && !unit.special_c_unit.empty()) {
                        ImVec2 text_size_sp = ImGui::CalcTextSize(unit.special_c_unit.c_str());
                        ImVec2 text_pos_sp(p_min.x + (zoom - text_size_sp.x) * 0.5f, p_min.y + (zoom - text_size_sp.y) * 0.5f);
                        draw_list->AddText(text_pos_sp, IM_COL32(255, 255, 255, 255), unit.special_c_unit.c_str());
                    }
                }

                // Draw a faint outline for the grid
                draw_list->AddRect(p_min, p_max, IM_COL32(100, 100, 100, 50));
            }
        }

        ImGui::EndChild(); // End CanvasRegion (Scrollable area)

        // --- Zoom Control (Anchored to bottom of RightWorkspacePanel) ---
        // Set the width to fill the available space dynamically
        ImGui::SetNextItemWidth( (int)ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("Zoom").x - ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::SliderFloat("Zoom", &zoom, 5.0f, 100.0f, "%.1fx");

        ImGui::EndChild(); // End Right Panel

        ImGui::End(); // End Main Window
    };

#define GMENG_SDL_INIT true
