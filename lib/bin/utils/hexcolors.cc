#pragma once
#include <string>
#include <sstream>

constexpr uint32_t gmeng_hex_color( std::string hex_str ) {
    if (hex_str.at(0) == '#') hex_str = hex_str.substr(1);

    std::istringstream converter(hex_str);
    unsigned int value;
    converter >> std::hex >> value;

    return value;
};

/// Converts a CSS hex color string into an unsigned int (U32).
/// Example: HxCLR(#ffffff) -> 0xFFFFFF
#define HxCLR(hexcolor) gmeng_hex_color(#hexcolor)

#if GMENG_SDL == true
#include "../../../include/imgui/imgui.h"

constexpr ImU32 HexToImU32(std::string hexStr) {
    if (!hexStr.empty() && hexStr[0] == '#') {
        hexStr = hexStr.substr(1);
    }

    unsigned int value = 0;
    try {
        value = std::stoul(hexStr, nullptr, 16);
    } catch (...) {
        return IM_COL32(255, 255, 255, 255); // Fallback color on error
    }

    unsigned char r = 255, g = 255, b = 255, a = 255;

    if (hexStr.length() == 8) { // #RRGGBBAA
        r = (value >> 24) & 0xFF;
        g = (value >> 16) & 0xFF;
        b = (value >> 8) & 0xFF;
        a = value & 0xFF;
    } else if (hexStr.length() == 6) { // #RRGGBB
        r = (value >> 16) & 0xFF;
        g = (value >> 8) & 0xFF;
        b = value & 0xFF;
    } else if (hexStr.length() == 3) { // #RGB (shorthand)
        r = ((value >> 8) & 0xF) * 17;
        g = ((value >> 4) & 0xF) * 17;
        b = (value & 0xF) * 17;
    }

    return IM_COL32(r, g, b, a);
}
#define hex_imu32(hexcolor) HexToImU32(#hexcolor)



#define hex_vec4(hexcolor) ImGui::ColorConvertU32ToFloat4( hex_imu32(hexcolor) )

void imgui_style_global() {
    // Get the global style object
    ImGuiStyle& style = ImGui::GetStyle();

    // Modify specific colors (RGBA values between 0.0f and 1.0f)
    style.Colors[ImGuiCol_WindowBg]      = hex_vec4(#2e222f);
    style.Colors[ImGuiCol_Button]        = hex_vec4(#484a77);
    style.Colors[ImGuiCol_ButtonHovered] = hex_vec4(#4d65b4);
    style.Colors[ImGuiCol_ButtonActive]  = hex_vec4(#4d9be6);
    style.Colors[ImGuiCol_Text]          = hex_vec4(#ffffff);
    style.Colors[ImGuiCol_FrameBg]       = hex_vec4(#525150aa);
    style.Colors[ImGuiCol_FrameBgActive] = hex_vec4(#7f708a);

    style.Colors[ImGuiCol_TitleBg]          = hex_vec4(#3e3546);
    style.Colors[ImGuiCol_TitleBgActive]    = hex_vec4(#905ea9);
    style.Colors[ImGuiCol_TitleBgCollapsed] = hex_vec4(#3e3546aa);

    style.Colors[ImGuiCol_ScrollbarBg]            = hex_vec4(#2e222f);
    style.Colors[ImGuiCol_ScrollbarGrab]          = hex_vec4(#625565);
    style.Colors[ImGuiCol_ScrollbarGrabHovered]   = hex_vec4(#7f708a);
    style.Colors[ImGuiCol_ScrollbarGrabActive]    = hex_vec4(#9babb2);

    style.Colors[ImGuiCol_FrameBg]        = hex_vec4(#3e3546);
    style.Colors[ImGuiCol_FrameBgHovered] = hex_vec4(#625565);
    style.Colors[ImGuiCol_FrameBgActive]  = hex_vec4(#7f708a);
};

#endif
