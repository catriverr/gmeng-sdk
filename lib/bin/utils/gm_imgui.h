#pragma once
#include <vector>

#include "cursor.h"
#include "global.h"
#include "imgui.h"
#include "keyboard.h"
#include "../types/event.h"

/// Define the Implementation if its not set to default (true)
#ifndef GMENG_IMGUI_IMPLEMENTATION
    #define GMENG_IMGUI_IMPLEMENTATION 1
#endif


/// Dear ImGui, Implementation for Gmeng
/// (Supported only for applications using the Kitty Terminal Graphics Protocol)

#if GMENG_IMGUI_IMPLEMENTATION == 1

struct rgb_texture {
    uint8_t* pixels;
    int width;
    int height;
};

#include "../../../helpers/imgui_impl_kitty_render.h"

// Replace the old raw pointer with our new struct for the font atlas
static rgb_texture* g_FontAtlasTexture = nullptr;

#include "../types/IconsFontAwesome7.h"

inline bool ImGui_ImplKitty_Init() {
    ImGuiIO& io = ImGui::GetIO();
    io.BackendRendererName = "imgui_impl_kitty_sw";
    io.Fonts->ClearFonts();

    ImFont* font = io.Fonts->AddFontFromFileTTF((get_executable_directory() + "/assets/ProggyClean.ttf").c_str(), 22.0f);
    IM_ASSERT( font != nullptr && "Could not load font file. Check if Gmeng's assets folder is in the working directory." );

    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = 4;
    icons_config.OversampleV = 4;

    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    float icon_font_size = 23.2f;
    io.Fonts->AddFontFromFileTTF((get_executable_directory() + "/assets/fa7.ttf").c_str(), icon_font_size, &icons_config, icons_ranges);

    // Extract the ImGui font atlas as 32-bit RGBA
    unsigned char* pixels;
    int width, height;

    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    g_FontAtlasTexture = new rgb_texture();
    g_FontAtlasTexture->width = width;
    g_FontAtlasTexture->height = height;

    // Store texture in memory for the rasterizer to sample
    size_t tex_size = width * height * 4;
    g_FontAtlasTexture->pixels = new uint8_t[tex_size];
    std::memcpy(g_FontAtlasTexture->pixels, pixels, tex_size);
    
    // Set a dummy texture ID so ImGui knows the atlas is built
    io.Fonts->SetTexID((ImTextureID)(intptr_t)g_FontAtlasTexture);


    return true;
}

inline void ImGui_ImplKitty_Shutdown() {
    if ( g_FontAtlasTexture ) {
        delete[] g_FontAtlasTexture;
        g_FontAtlasTexture = nullptr;
    }
}

#include <algorithm>
#include <cmath>

inline float Edge(const ImVec2& a, const ImVec2& b, const ImVec2& c) {
    return (c.x - a.x) * (b.y - a.y) - (c.y - a.y) * (b.x - a.x);
}


inline void ImGui_ImplGmeng_UpdateMouseCursor() {
    ImGuiIO& io = ImGui::GetIO();
    
    // Allow the user to disable cursor changes via ImGui flags
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    
    // If ImGui is drawing a software cursor, we treat it as "None" for the OS/Terminal
    if (io.MouseDrawCursor) 
        imgui_cursor = ImGuiMouseCursor_None;

    // Prevent spamming the terminal with escape sequences every frame
    static ImGuiMouseCursor last_cursor = ImGuiMouseCursor_COUNT;
    if (last_cursor == imgui_cursor)
        return;

    last_cursor = imgui_cursor;

    // Map ImGui cursors to your Gmeng cursor_type enum
    switch (imgui_cursor) {
        case ImGuiMouseCursor_TextInput:
            gmeng_set_cursor(cursor_type::TEXT);
            break;
        case ImGuiMouseCursor_ResizeNS:
            gmeng_set_cursor(cursor_type::NSRESIZE);
            break;
        case ImGuiMouseCursor_ResizeEW:
            gmeng_set_cursor(cursor_type::EWRESIZE);
            break;
        case ImGuiMouseCursor_Hand:
            gmeng_set_cursor(cursor_type::POINTER);
            break;
        case ImGuiMouseCursor_NotAllowed:
            gmeng_set_cursor(cursor_type::NOTALLOWED);
            break;
        case ImGuiMouseCursor_None:
            break;
        case ImGuiMouseCursor_Arrow:
            gmeng_set_cursor(cursor_type::CELL);
            break;
        case ImGuiMouseCursor_ResizeAll:  // No exact match in your enum
            gmeng_set_cursor(cursor_type::NESWRESIZE);
            break;
        case ImGuiMouseCursor_ResizeNESW: // No exact match in your enum
            gmeng_set_cursor(cursor_type::NESWRESIZE);
            break;
        case ImGuiMouseCursor_ResizeNWSE: // No exact match in your enum
            gmeng_set_cursor(cursor_type::NWSERESIZE);
            break;
        default:
            gmeng_set_cursor(cursor_type::DEFAULT);
            break;
    }
};

// Translates raw char inputs from the terminal into ImGui keys
static ImGuiKey gmeng_get_imgui_key(char c) {
    if (c >= 'a' && c <= 'z') return (ImGuiKey)(ImGuiKey_A + (c - 'a'));
    if (c >= 'A' && c <= 'Z') return (ImGuiKey)(ImGuiKey_A + (c - 'A'));
    if (c >= '0' && c <= '9') return (ImGuiKey)(ImGuiKey_0 + (c - '0'));
    
    switch (c) {
        case '\n': case '\r': return ImGuiKey_Enter;
        case '\t': return ImGuiKey_Tab;
        case '\x1b': return ImGuiKey_Escape; // ESC
        case ' ': return ImGuiKey_Space;
        case '\x7f': case '\b': return ImGuiKey_Backspace;
        default: return ImGuiKey_None;
    }
}

// Forward declaration of the handler
void ImGui_ImplGmeng_EventHandler(Gmeng::Level* lvl, Gmeng::EventInfo* info);

inline bool ImGui_ImplGmeng_Init(Gmeng::EventLoop* loop) {
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformName = "imgui_impl_gmeng";

    std::vector<Gmeng::Event> imgui_events = {
        Gmeng::Event::UPDATE,
        Gmeng::Event::MOUSE_MOVE,
        Gmeng::Event::MOUSE_CLICK_LEFT_START, Gmeng::Event::MOUSE_CLICK_LEFT_END,
        Gmeng::Event::MOUSE_CLICK_RIGHT_START, Gmeng::Event::MOUSE_CLICK_RIGHT_END,
        Gmeng::Event::MOUSE_CLICK_MIDDLE_START, Gmeng::Event::MOUSE_CLICK_MIDDLE_END,
        Gmeng::Event::MOUSE_SCROLL_UP, Gmeng::Event::MOUSE_SCROLL_DOWN,
        Gmeng::Event::KEYPRESS, Gmeng::Event::KEYRELEASE, Gmeng::Event::KEYREPEAT
    };

    loop->add_hook(imgui_events, ImGui_ImplGmeng_EventHandler);
    return true;
}



inline void ImGui_ImplGmeng_NewFrame(int width, int height, float delta_time) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2((float)width, (float)height);
    io.DeltaTime = delta_time > 0.0f ? delta_time : 1.0f / 60.0f;
    ImGui_ImplGmeng_UpdateMouseCursor();
}

inline void ImGui_ImplGmeng_EventHandler(Gmeng::Level* lvl, Gmeng::EventInfo* info) {
    ImGuiIO& io = ImGui::GetIO();
    
    using namespace Gmeng;
#ifdef __APPLE__
    io.AddKeyEvent(ImGuiMod_Ctrl, getkey(gk_LMETA) || getkey(gk_RMETA));   // Cmd triggers ImGui's Ctrl shortcuts
    io.AddKeyEvent(ImGuiMod_Super, getkey(gk_LCTRL) || getkey(gk_RCTRL)); // Ctrl triggers Super
#else
    io.AddKeyEvent(ImGuiMod_Ctrl, getkey(gk_LCTRL) || getkey(gk_RCTRL));   // Windows/Linux Ctrl
    io.AddKeyEvent(ImGuiMod_Super, getkey(gk_LMETA) || getkey(gk_RMETA)); // Windows/Linux Win/Meta
#endif
    io.AddKeyEvent(ImGuiMod_Alt, getkey(gk_LALT));
    io.AddKeyEvent(ImGuiMod_Shift, getkey(gk_LSHIFT) || getkey(gk_RSHIFT));

    io.AddKeyEvent(ImGuiKey_Backspace, getkey(gk_BACKSPACE));

    io.AddKeyEvent(ImGuiKey_UpArrow, getkey(gk_UP));
    io.AddKeyEvent(ImGuiKey_DownArrow, getkey(gk_DOWN));
    io.AddKeyEvent(ImGuiKey_LeftArrow, getkey(gk_LEFT));
    io.AddKeyEvent(ImGuiKey_RightArrow, getkey(gk_RIGHT));

    switch (info->EVENT) {
        // MOUSE EVENTS
        case Gmeng::Event::MOUSE_MOVE:
            io.AddMousePosEvent((float)info->RAW_MOUSE_POS.x, (float)info->RAW_MOUSE_POS.y);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
            
        case Gmeng::Event::MOUSE_CLICK_LEFT_START:
            io.AddMouseButtonEvent(0, true);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
        case Gmeng::Event::MOUSE_CLICK_LEFT_END:
            io.AddMouseButtonEvent(0, false);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
            
        case Gmeng::Event::MOUSE_CLICK_RIGHT_START:
            io.AddMouseButtonEvent(1, true);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
        case Gmeng::Event::MOUSE_CLICK_RIGHT_END:
            io.AddMouseButtonEvent(1, false);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
            
        case Gmeng::Event::MOUSE_CLICK_MIDDLE_START:
            io.AddMouseButtonEvent(2, true);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
        case Gmeng::Event::MOUSE_CLICK_MIDDLE_END:
            io.AddMouseButtonEvent(2, false);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
            
        case Gmeng::Event::MOUSE_SCROLL_UP:
            io.AddMouseWheelEvent(0.0f, 1.0f);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
        case Gmeng::Event::MOUSE_SCROLL_DOWN:
            io.AddMouseWheelEvent(0.0f, -1.0f);
            if (io.WantCaptureMouse) info->prevent_further = true;
            break;
            
        // KEYBOARD EVENTS
        case Gmeng::Event::KEYPRESS:
        case Gmeng::Event::KEYREPEAT: {
            ImGuiKey key = gmeng_get_imgui_key(info->KEYPRESS_CODE);
            if (key != ImGuiKey_None) io.AddKeyEvent(key, info->EVENT == Gmeng::Event::KEYPRESS);
            if (info->KEYPRESS_CODE >= 32 && info->KEYPRESS_CODE <= 126) {
                io.AddInputCharacter(info->KEYPRESS_CODE);
            }
            if (io.WantCaptureKeyboard) info->prevent_further = true;
            break;
        };
        case Gmeng::Event::KEYRELEASE: {
            ImGuiKey key = gmeng_get_imgui_key(info->KEYPRESS_CODE);
            if (key != ImGuiKey_None) io.AddKeyEvent(key, false);
            if (io.WantCaptureKeyboard) info->prevent_further = true;
            break;
        }
        default: break;
    }
}
#endif // GMENG_IMGUI_IMPLEMENTATION == 1

#define FILE_DIRECTORY(file) \
    (std::string(STR(file)).substr(0, std::string(STR(file)).find_last_of("/\\")))
// Returns true if selection changed, current item index updated by reference
inline bool ImGui_CenteredSelection(
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

inline bool ImGui_ClickableRect(const char* str_id, ImVec2 p_min, ImVec2 p_max, ImU32 color, ImU32 hover_color, ImU32 click_color)
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

inline bool ImGui_SelectableImage(ImTextureID texture, ImVec2 size, bool selected = false, ImU32 highlightColor = IM_COL32(255, 255, 0, 255), float thickness = 2.0f) {
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
inline void ImGui_CenteredText(const char* text, ImFont* font = nullptr) {
    ImVec2 text_size = ImGui::CalcTextSize(text);
    float window_width = ImGui::GetContentRegionAvail().x;
    float text_x = (window_width - text_size.x) * 0.5f;

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + text_x);
    if (font) ImGui::PushFont(font);
    ImGui::TextUnformatted(text);
    if (font) ImGui::PopFont();
};

inline void ImGui_2d_position(int* x, int* y, int screenspace_width, int screenspace_height) {
    // 1. Force the child panel to a consistent 200x200px box size for the plane
    auto plane_size = ImGui::GetContentRegionAvail();
    
    // We add some extra padding for the bottom X slider and the right Y slider layouts
    ImGui::BeginChild("##2DSelectorPanel",
                      plane_size,
                      ImGuiChildFlags_None);

    // Capture the starting draw cursor position
    ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // 2. Draw the 200x200 Plane Background
    ImVec2 plane_end = ImVec2(canvas_pos.x + plane_size.x, canvas_pos.y + plane_size.y);
    draw_list->AddRectFilled(canvas_pos, plane_end, IM_COL32(40, 40, 40, 255)); // Dark gray background
    draw_list->AddRect(canvas_pos, plane_end, IM_COL32(100, 100, 100, 255));   // Grid border

    // 3. Handle Invisible Button over the plane for Drag-to-Position interactions
    ImGui::InvisibleButton("##PlaneDropZone", plane_size);
    if (ImGui::IsItemActive() && (screenspace_width > 0 && screenspace_height > 0)) {
        ImVec2 mouse_pos = ImGui::GetIO().MousePos;
        
        // Map pixel coordinates relative to the plane back into screenspace coordinates
        float pct_x = (mouse_pos.x - canvas_pos.x) / plane_size.x;
        float pct_y = (mouse_pos.y - canvas_pos.y) / plane_size.y;
        
        // Clamp percentages between 0.0 and 1.0
        pct_x = pct_x < 0.0f ? 0.0f : (pct_x > 1.0f ? 1.0f : pct_x);
        pct_y = pct_y < 0.0f ? 0.0f : (pct_y > 1.0f ? 1.0f : pct_y);

        *x = (int)(pct_x * screenspace_width);
        *y = (int)(pct_y * screenspace_height);
    }

    // 4. Draw the actual crosshair indicator point
    if (screenspace_width > 0 && screenspace_height > 0) {
        float dot_x = canvas_pos.x + (((float)*x / screenspace_width) * plane_size.x);
        float dot_y = canvas_pos.y + (((float)*y / screenspace_height) * plane_size.y);
        
        // Draw crosshair lines
        draw_list->AddLine(ImVec2(canvas_pos.x, dot_y), ImVec2(plane_end.x, dot_y), IM_COL32(255, 255, 255, 100));
        draw_list->AddLine(ImVec2(dot_x, canvas_pos.y), ImVec2(dot_x, plane_end.y), IM_COL32(255, 255, 255, 100));
        // Draw the point core
        draw_list->AddCircleFilled(ImVec2(dot_x, dot_y), 4.0f, IM_COL32(255, 100, 100, 255));
    }

    ImGui::EndChild();
}

inline void ImGui_ModifierList(Gmeng::ModifierList& modlist) {
    for ( auto& mod : modlist.values ) {
        ImGui::InputInt(mod.name.c_str(), &mod.value);
    };
};
