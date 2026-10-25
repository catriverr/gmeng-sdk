#pragma once
#include "hexcolors.cc"
#include <imgui.h>
#include <string_view>
#include <vector>
#include <charconv>
#include <algorithm>

inline void ImGui_TextAnsi(std::string_view text) {
    ImVec4 defaultColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    ImVec4 currentColor = defaultColor;
    ImVec4 currentBgColor = ImVec4(0, 0, 0, 0);
    bool hasBg = false;

    bool isUnderline = false;
    bool isStrikethrough = false;

    struct Chunk {
        std::string_view data;
        ImVec4 color;
        ImVec4 bgColor;
        bool hasBg;
        bool underline;
        bool strikethrough;
    };

    std::vector<Chunk> chunks;
    
    auto addChunk = [&](std::string_view chunkData) {
        if (chunkData.empty()) return;
        chunks.push_back({chunkData, currentColor, currentBgColor, hasBg, isUnderline, isStrikethrough});
    };

    size_t lastPos = 0;
    size_t pos = text.find("\033[");

    while (pos != std::string_view::npos) {
        if (pos > lastPos) {
            addChunk(text.substr(lastPos, pos - lastPos));
        }

        size_t endPos = text.find('m', pos);
        if (endPos == std::string_view::npos) break;

        std::string_view ansiParams = text.substr(pos + 2, endPos - (pos + 2));
        
        std::vector<int> codes;
        size_t pStart = 0;
        while (pStart < ansiParams.length()) {
            size_t pEnd = ansiParams.find(';', pStart);
            if (pEnd == std::string_view::npos) pEnd = ansiParams.length();
            
            int val = 0;
            std::from_chars(ansiParams.data() + pStart, ansiParams.data() + pEnd, val);
            codes.push_back(val);
            
            pStart = pEnd + 1;
        }

        for (size_t i = 0; i < codes.size(); ++i) {
            int code = codes[i];
            if (code == 0) {
                currentColor = defaultColor;
                currentBgColor = ImVec4(0, 0, 0, 0);
                hasBg = false;
                isUnderline = false;
                isStrikethrough = false;
            } 
            else if (code == 4)  { isUnderline = true; }
            else if (code == 9)  { isStrikethrough = true; }
            else if (code == 24) { isUnderline = false; }
            else if (code == 29) { isStrikethrough = false; }
            // Foreground Standard (30-37)
            else if (code >= 30 && code <= 37) {
                switch (code) {
                    case 30:
                        currentColor = hex_vec4(#2e222f);
                        break;
                    case 31:
                        currentColor = hex_vec4(#e83b3b);
                        break;
                    case 32:
                        currentColor = hex_vec4(#91db69);
                        break;
                    case 33:
                        currentColor = hex_vec4(#fbb954);
                        break;
                    case 34:
                        currentColor = hex_vec4(#4d9be6);
                        break;
                    case 35:
                        currentColor = hex_vec4(#a884f3);
                        break;
                    case 36:
                        currentColor = hex_vec4(#8fd3ff);
                        break;
                    case 37:
                    default:
                        currentColor = hex_vec4(#ffffff);
                        break;
                };
            }
            // Foreground Bright (90-97)
            else if (code >= 90 && code <= 97) {
                int baseCode = code - 60;
                float r = (baseCode & 1) ? 1.0f : 0.3f;
                float g = (baseCode & 2) ? 1.0f : 0.3f;
                float b = (baseCode & 4) ? 1.0f : 0.3f;
                if (code == 97) { r = 1.0f; g = 1.0f; b = 1.0f; }
                if (code == 90) { r = 0.5f; g = 0.5f; b = 0.5f; }
                currentColor = ImVec4(r, g, b, 1.0f);
            }
            else if (code == 39) {
                currentColor = defaultColor;
            }
            // Foreground Truecolor (38;2;R;G;B)
            else if (code == 38 && i + 4 < codes.size() && codes[i + 1] == 2) {
                currentColor = ImVec4(
                    (float)codes[i + 2] / 255.0f,
                    (float)codes[i + 3] / 255.0f,
                    (float)codes[i + 4] / 255.0f,
                    1.0f
                );
                i += 4;
            }
            // Background Standard (40-47)
            else if (code >= 40 && code <= 47) {
                float r = (code & 1) ? 0.8f : 0.1f;
                float g = (code & 2) ? 0.8f : 0.1f;
                float b = (code & 4) ? 0.8f : 0.1f;
                if (code == 47) { r = 0.8f; g = 0.8f; b = 0.8f; }
                currentBgColor = ImVec4(r, g, b, 1.0f);
                hasBg = true;
            }
            // Background Bright (100-107)
            else if (code >= 100 && code <= 107) {
                int baseCode = code - 60;
                float r = (baseCode & 1) ? 1.0f : 0.3f;
                float g = (baseCode & 2) ? 1.0f : 0.3f;
                float b = (baseCode & 4) ? 1.0f : 0.3f;
                if (code == 107) { r = 1.0f; g = 1.0f; b = 1.0f; }
                if (code == 100) { r = 0.4f; g = 0.4f; b = 0.4f; }
                currentBgColor = ImVec4(r, g, b, 1.0f);
                hasBg = true;
            }
            else if (code == 49) {
                hasBg = false;
            }
            // Background Truecolor (48;2;R;G;B)
            else if (code == 48 && i + 4 < codes.size() && codes[i + 1] == 2) {
                currentBgColor = ImVec4(
                    (float)codes[i + 2] / 255.0f,
                    (float)codes[i + 3] / 255.0f,
                    (float)codes[i + 4] / 255.0f,
                    1.0f
                );
                hasBg = true;
                i += 4;
            }
        }

        lastPos = endPos + 1;
        pos = text.find("\033[", lastPos);
    }

    if (lastPos < text.length()) {
        addChunk(text.substr(lastPos));
    }

    // Render chunks
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (size_t i = 0; i < chunks.size(); ++i) {
        const auto& chunk = chunks[i];

        // Draw background highlight rectangle behind the text chunk if active
        if (chunk.hasBg) {
            ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
            ImVec2 textSize = ImGui::CalcTextSize(chunk.data.data(), chunk.data.data() + chunk.data.length());
            drawList->AddRectFilled(
                cursorScreenPos, 
                ImVec2(cursorScreenPos.x + textSize.x, cursorScreenPos.y + textSize.y), 
                ImGui::ColorConvertFloat4ToU32(chunk.bgColor)
            );
        }

        ImGui::TextColored(chunk.color, "%.*s", (int)chunk.data.length(), chunk.data.data());

        if (chunk.underline || chunk.strikethrough) {
            ImVec2 min = ImGui::GetItemRectMin();
            ImVec2 max = ImGui::GetItemRectMax();
            ImU32 col = ImGui::ColorConvertFloat4ToU32(chunk.color);

            if (chunk.underline) {
                drawList->AddLine(ImVec2(min.x, max.y - 1.0f), ImVec2(max.x, max.y - 1.0f), col, 1.0f);
            }
            if (chunk.strikethrough) {
                float midY = min.y + (max.y - min.y) * 0.5f;
                drawList->AddLine(ImVec2(min.x, midY), ImVec2(max.x, midY), col, 1.0f);
            }
        }

        if (i + 1 < chunks.size()) {
            ImGui::SameLine(0.0f, 0.0f);
        }
    }

    if (chunks.empty()) {
        ImGui::NewLine();
    }
}
