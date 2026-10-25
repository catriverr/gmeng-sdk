#pragma once


#include "ImGuiColorTextEdit/TextEditor.h"
#include "ImGuiColorTextEdit/TextEditor.cpp"
#include "gm_imgui.h"
#include "hexcolors.cc"
#include <fstream>
#include <sstream>
#include <string>

#include "../types/IconsFontAwesome7.h"

/// From TextEditor.h:GetRetroBluePalette() method
/// const TextEditor::Palette & TextEditor::GetRetroBluePalette()
/// {
/// 	const static Palette p = { {
/// 			#ff00ffff,	// None
/// 			#ffffff00,	// Keyword
/// 			#ff00ff00,	// Number
/// 			#ff808000,	// String
/// 			#ff808000,  // Char literal
/// 			#ffffffff,  // Punctuation
/// 			#ff008000,  // Preprocessor
/// 			#ff00ffff,  // Identifier
/// 			#ffffffff,  // Known identifier
/// 			#ffff00ff,  // Preproc identifier
/// 			#ff808080,  // Comment (single line)
/// 			#ff404040,  // Comment (multi line)
/// 			#ff800000,  // Background
/// 			#ff0080ff,  // Cursor
/// 			#80ffff00,  // Selection
/// 			#a00000ff,  // ErrorMarker
/// 			#80ff8000,  // Breakpoint
/// 			#ff808000,  // Line number
/// 			#40000000,  // Current line fill
/// 			#40808080,  // Current line fill (inactive)
/// 			#40000000,  // Current line edge
/// 		} };
/// 	return p;
/// }

static TextEditor::Palette imguicolortextedit_ressurect64_palette = {
    hex_imu32(#ffffff), // Default (Text)
    hex_imu32(#e83b3b), // Keyword
    hex_imu32(#fbb954), // Number
    hex_imu32(#91db69), // String
    hex_imu32(#eaaded), // Char literal
    hex_imu32(#9babb2), // Punctuation
    hex_imu32(#a884f3), // Preprocessor
    hex_imu32(#8fd3ff), // Identifier
    hex_imu32(#4d65b4), // Known identifier
    hex_imu32(#4d9be6), // Predefined identifier
    hex_imu32(#ab947a), // Comment (single line)
    hex_imu32(#ab947a), // Comment (multi line)
    hex_imu32(#2e222f), // Background
    hex_imu32(#ffe0e0), // Cursor
    hex_imu32(#525150), // Selection
    hex_imu32(#e83b3b), // Error marker
    hex_imu32(#e83b3b), // Breakpoint
    hex_imu32(#fdcbb0), // Line number
    hex_imu32(#3e3546), // Current line fill
    hex_imu32(#525150), // Current line fill (inactive)
    hex_imu32(#3e3546)  // Current line edge
};

struct GmengImGuiScripts {
  public:
    bool open = 0;
    std::string selected_file = "";
    std::fstream file;
    bool file_open = false;
    std::atomic<bool> compilation_finished{ false };
    std::atomic<bool> is_compiling{ false };
    TextEditor editor;
    std::stringstream data;
    std::stringstream ab;

    GmengImGuiScripts(std::string_view filetype = "cpp") {
        if ( filetype == "cpp" )
            this->editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());
        if ( filetype == "lua" )
            this->editor.SetLanguageDefinition(TextEditor::LanguageDefinition::CPlusPlus());

        //this->editor.SetPalette(imguicolortextedit_ressurect64_palette);
    };

    inline void Draw() {
        ImGui::SetNextItemOpen(false);
        ImGui::SetNextWindowSize({ 1280, 720 });
        ImGui::Begin("gmeng script manager", &open, ImGuiWindowFlags_NoResize);
        ImGui_CenteredText(selected_file.length() > 0 ? selected_file.c_str() : "select a script file to view its contents");

            ImGui::BeginChild("script_list", { 340,0 }, true);
                ImGui::SeparatorText("scripts/src/**");
                for (auto f : std::filesystem::directory_iterator("scripts/src/")) {
                    bool disabled = f.is_directory() || contains( f.path().generic_string(), "dylib" );
                    auto file_icon = f.is_directory() ? ICON_FA_FOLDER : ICON_FA_FILE_CODE;
                    auto label = (file_icon + f.path().generic_string());

                    if (f.path().generic_string() != selected_file) {
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32( 0, 0, 0, 0 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32( 40, 40, 170, 200 ));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32( 40, 40, 170, 255 ));
                    };

                    if ( disabled ) ImGui::BeginDisabled();
                    if (ImGui::Button(label.c_str()) && selected_file != f.path().generic_string()) {
                        selected_file = f.path().generic_string();
                        this->file.close();
                        this->data.str("");
                        this->data.clear();
                        this->file = std::fstream(selected_file);
                        this->file_open = true;
                        this->data << this->file.rdbuf();
                        editor.SetText( data.str() );
                        ImGui::PopStyleColor(3);
                    };
                    if (disabled) ImGui::EndDisabled();
                    if ( selected_file != f.path().generic_string() ) ImGui::PopStyleColor(3);
                    ImGui::Separator();
                }

            ImGui::EndChild();
            ImGui::SameLine();
            ImGui::Separator();
            ImGui::SameLine();
            ImGui::Dummy({ 0, 20 });
            ImGui::SameLine();

            if ( !selected_file.empty() && this->file_open ) {
                if (!this->file.is_open()) {
                    ImGui_CenteredText("Select a script file to get started");
                };
                ImGui::BeginChild("editor_colored");

                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("Save")) {
                        std::string text = editor.GetText();
                        this->file.close();
                        std::ofstream writeF( selected_file );
                        for ( const auto& line : g_splitStr( text, "\n" ) )
                            writeF << line << "\n";

                        writeF.close();
                        this->file.open( selected_file );
                    };
                    ImGui::EndMenu();
                };

                std::string temp_filename = "";
if (ImGui::BeginMenu("Script")) {
    if (ImGui::MenuItem("Compile", NULL, false, !is_compiling)) {
        temp_filename = v_str(g_mkid()) + "_gmeng_script_compilation";

        is_compiling = true;
        std::string string_path = get_executable_directory();
        std::string full_file_path = string_path + "/" + selected_file;

        std::thread([string_path, full_file_path, temp_filename, this]() {
            std::string command = "make -C \"" + string_path +
                                  "\" compile-script filename=\"" + full_file_path +
                                  "\" > \"" + temp_filename + "\"";
            std::system(command.c_str());

            // Read the output file into a local string first to keep disk I/O out of the shared stream
            std::ifstream ifs(temp_filename);
            std::string output_contents;
            if (ifs.is_open()) {
                std::stringstream ss;
                ss << ifs.rdbuf();
                output_contents = ss.str();
                ifs.close();
            }
            std::filesystem::remove(temp_filename);

            this->ab.str(output_contents);
            this->ab.clear();
            this->compilation_finished = true;
            this->is_compiling = false;
        }).detach();
    }
    ImGui::EndMenu();
}

                editor.Render("TextEditorComponent");

                ImGui::EndChild();

                if ( compilation_finished ) {
                    compilation_finished = false;
                    ImGui::OpenPopup("compilation_results");
                };

                if (ImGui::BeginPopupModal("compilation_results", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    ImGui_CenteredText("Compilation finished | makecmd output");
                    ImGui::Separator();
                    ImGui::Text( ab.str().c_str() );

                    if (ImGui::Button("Close")) {
                        ab.str("");
                        ab.clear();
                        ImGui::CloseCurrentPopup();
                    }

                    // Always call EndPopup() if BeginPopup() returned true
                    ImGui::EndPopup();
                }
            };

        ImGui::End();
    };
};
