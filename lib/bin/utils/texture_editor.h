#include <cstddef>
#include <set>
#include <string>

#include <imgui.h>

#include "../types/unit.h"
#include "../types/texture.h"
#include "../src/textures.cpp"

#include "global.h"

#ifndef GMENG_TEXTURE_EDITOR_DECLARATION
class GmengImGuiTextureEditor {
  public:
    std::string current_filename;
    bool loaded = false;
    Gmeng::texture tex;
    Gmeng::Unit current_brush;
    float brush_color_float[3] = { 1.0f, 1.0f, 1.0f }; // For ImGui ColorEdit3
    std::set<uint32_t> texture_palette;

    float zoom = 57.2f;
    bool is_open = true;

    GmengImGuiTextureEditor(const std::string& default_filename = "custom.gt")
        : current_filename(default_filename)
    {
        // Initialize an empty texture to prevent rendering a 0x0 grid
        tex.name = v_str(g_mkid());
        ResizeTexture(8, 8);
    }
    void ResizeTexture(std::size_t, std::size_t);
    void Save(const std::string& fname);
    void Load(const std::string& fname);
    void Draw();
};

#define GMENG_TEXTURE_EDITOR_DECLARATION 1

#endif

#ifdef GMENG_TEXTURE_EDITOR_IMPLEMENTATION

#include "gm_imgui.h"
    void GmengImGuiTextureEditor::ResizeTexture(std::size_t new_w, std::size_t new_h) {
        if (new_w < 1) new_w = 1;
        if (new_h < 1) new_h = 1;

        std::vector<Gmeng::Unit> new_units(new_w * new_h, Gmeng::Unit { .transparent = true });

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
        Gmeng::save_texture( fname, tex );
    }

    void GmengImGuiTextureEditor::Load(const std::string& fname) {
        if (std::filesystem::exists( fname ) && !std::filesystem::is_directory(fname))
            tex = Gmeng::LoadTexture(fname), loaded = true;

        /// palette finder
        texture_palette.clear();
        for ( auto un : tex.units ) {
            if ( !texture_palette.contains( un.color ) ) texture_palette.insert( un.color );
        };
    }


void GmengImGuiTextureEditor::Draw() {
        if (!is_open) return;

        ImGui::SetNextWindowSize({ 1280, 720 });
        ImGui::Begin("Gmeng Texture Editor", &is_open, ImGuiWindowFlags_NoResize);

        ImGui_CenteredText(("texture editor for Gmeng Game Development Kit " + Gmeng::version + " build " + GMENG_BUILD_NO).c_str());

        // Child Panel
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

        // Texture Properties
        char nameBuf[256];
        strncpy(nameBuf, tex.name.c_str(), sizeof(nameBuf));
        if (ImGui::InputText("Texture Name", nameBuf, sizeof(nameBuf))) {
            tex.name = nameBuf;
        }

        int size[2] = { static_cast<int>(tex.width), static_cast<int>(tex.height) };
        if (ImGui::InputInt2("Size (W/H)", size)) {
            ResizeTexture(static_cast<std::size_t>(size[0]), static_cast<std::size_t>(size[1]));
        }

        ImGui::Checkbox("collision", &tex.collidable);
        ImGui::Separator();

        // Brush Settings
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
            ImGui::InputInt("color", &(current_brush.special_clr));

            char charBuf[16] = { current_brush.special_c_unit, '\0' };
            if (ImGui::InputText("character", charBuf, sizeof(charBuf))) {
                current_brush.special_c_unit = charBuf[0];
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

                if ( current_brush.color == uint32_from_color32(rgb) ) ImGui::SetNextItemAllowOverlap(),
                    draw_list->AddRect(p_min, p_max, IM_COL32( 255-rgb.r, 255-rgb.g, 255-rgb.b, 155 ), 0, 0, 3);

                i++;
            };


            ImGui::Dummy( ImVec2 { 0, (float)((int)(i-1)/6) * brush_size + 54 } );

            ImGui::Separator();

            ImGui_CenteredText("engine palette (gmeng:default)");







            int j = 0;
            ImVec2 c_pos2 = ImGui::GetCursorScreenPos();

            for ( auto brush : Gmeng::rgb_colors ) {
                Gmeng::color32_t rgb( brush[0], brush[1], brush[2] );

                ImVec2 p_min( ( (j % 6)*2 ) + c_pos.x + (j % 6) * brush_size    , ( ((int)((i-1) / 6) + 1) *brush_size + 30) + ( (int)(j / 6)*2 ) + ((c_pos.y) + ((j / 6.0f))*brush_size) - (j % 6)*8.6658 );
                ImVec2 p_max( ( (j % 6)*2 ) + c_pos.x + ((j % 6)+1) * brush_size, ( ((int)((i-1) / 6) + 1) *brush_size + 30) + ( (int)(j / 6)*2 ) + c_pos.y + ((j / 6.0f)+1) * brush_size - (j % 6)*8.6658 );

                if (ImGui_ClickableRect((v_str(j) + "_default_palette_colord").c_str(), p_min, p_max, IM_COL32( rgb.r, rgb.g, rgb.b, 255 ),
                                                                                 IM_COL32( 255-rgb.r, 255-rgb.g, 255-rgb.b, 255 ),
                                                                                 IM_COL32( rgb.r-20, rgb.g-20, rgb.b-20, 255 ) )) {
                    current_brush.color = uint32_from_color32(rgb);
                    brush_color_float[0] = rgb.r / 255.0f;
                    brush_color_float[1] = rgb.g / 255.0f;
                    brush_color_float[2] = rgb.b / 255.0f;
                };

                if ( current_brush.color == uint32_from_color32(rgb) ) ImGui::SetNextItemAllowOverlap(),
                    draw_list->AddRect(p_min, p_max, IM_COL32( 255-rgb.r, 255-rgb.g, 255-rgb.b, 155 ), 0, 0, 3);

                j++;
            }

            ImGui::EndChild();

        }
        ImGui::EndChild(); // End Left Panel

        ImGui::SameLine();

        // Canvas
        // This child takes the remaining width and full height
        ImGui::BeginChild("RightWorkspacePanel", ImVec2(0, 0), false);

        float zoom_item_height = ImGui::GetFrameHeightWithSpacing();

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

        // Right-Click erases to transparent unit
        if (ImGui::IsItemActive() && (ImGui::IsMouseDown(ImGuiMouseButton_Right) || ImGui::IsMouseDragging(ImGuiMouseButton_Right))) {
            ImVec2 mouse_pos = ImGui::GetIO().MousePos;
            int tx = static_cast<int>((mouse_pos.x - canvas_pos.x) / zoom);
            int ty = static_cast<int>((mouse_pos.y - canvas_pos.y) / zoom);
            if (tx >= 0 && tx < static_cast<int>(tex.width) && ty >= 0 && ty < static_cast<int>(tex.height)) {
                tex.units[ty * tex.width + tx] = Gmeng::Unit { .transparent=true };
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
                    if (unit.special && zoom >= 15.0f && unit.special_c_unit != ' ') {
                        ImVec2 text_size_sp = ImGui::CalcTextSize((unit.special_c_unit + (std::string)"").c_str());
                        ImVec2 text_pos_sp(p_min.x + (zoom - text_size_sp.x) * 0.5f, p_min.y + (zoom - text_size_sp.y) * 0.5f);
                        draw_list->AddText(text_pos_sp, IM_COL32(255, 255, 255, 255), ((std::string)"" + unit.special_c_unit).c_str());
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
#endif
