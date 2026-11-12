#pragma once

#include <algorithm>
#include <imgui.h>
#include <imgui_internal.h>

#include "functree.h"
#include "hexcolors.cc"
#include "log.h"
#include "kitty.h"
#include "global.h"
#include "gm_imgui.h"
#include "layer.h"
#include "imgui_ansitext.h"

#include "../types/eventloop.h"
#include "../types/entity.h"
#include "../types/level.h"

#include <string>

#include "ImGuiColorTextEdit/TextEditor.h"
#include "routine.h"

struct Gmeng_CustomEntityEditorCommand {
    int entity_serialization_id;

    std::function<void(Gmeng::EntityBase*)> short_options;

    std::map<std::string, std::function<void(Gmeng::EntityBase*)>> properties;
};

/// ImGui-based Entity-type specific editor commands. You can register an entity to have
/// custom editor options by adding to this map with the type's serialization ID and
/// a function that takes an EntityBase* pointer, cast the object to your derived
/// type and add ImGui editor commands here. An example can be seen at
/// `GMENG_ENTITY_TYPE_SPECIFIC_EDITOR_COMMANDS[ -4 ]` which is for `Animated_Entity`.
///
/// Example on how to add a custom entity editor:
/// ```
/// class SomeDerivedEntity : public Gmeng::Entity<SomeDerivedEntity> {
///     bool some_option = false;
/// }; GMENG_ENTITY_SET_ID( SomeDerivedEntity, 10 ); REGISTER_ENTITY_TYPE( SomeDerivedEntity );
///
/// GMENG_ENTITY_TYPE_SPECIFIC_EDITOR_COMMANDS[ GMENG_ENTITYID( SomeDerivedEntity ) ] = [](
///     Gmeng::EntityBase* base_entity_type
/// ) {
///     GMENG_ENTITY( derived_entity_type, SomeDerivedEntity, base_entity_type );
///     ImGui::Checkbox("SOME OPTION", &derived_entity_type->some_option);
/// };
/// ```
static std::map<int, Gmeng_CustomEntityEditorCommand> GMENG_ENTITY_TYPE_SPECIFIC_EDITOR_COMMANDS = {
    {
        GMENG_ENTITYID( Animated_Entity ),
        {
          GMENG_ENTITYID( Animated_Entity ),
          [](Gmeng::EntityBase* ent) {
              GMENG_ENTITY( ent_cast, Gmeng::Animated_Entity, ent );

              static float scale = 1.0f;

              ImGui::SetNextItemWidth(105);
              ImGui::SliderFloat("SCALE", &scale, 0.05f, 20.0f);
              ImGui::SameLine();
              ImGui::PushID( ent_cast->entity_id + 2 );

              if ( ImGui::Button("RESIZE") )
                  for ( auto& tx : ent_cast->anim_textures )
                      tx.resize( scale );

              ImGui::PopID();
          },
          {
            {
                  "tabitems",
                  [](Gmeng::EntityBase* ent) {
                    GMENG_ENTITY( ent_cast, Gmeng::Animated_Entity, ent );

                    if (ImGui::BeginTabItem("ANIMATION")) {
                        auto sz = ImGui::GetContentRegionAvail();

                        unsigned int stride_used = 0;
                        for ( auto& tx : ent_cast->anim_textures ) {
                            rgb_texture* rtx = new rgb_texture();
                              rtx->height = tx.height;
                              rtx->width  = tx.width;
                              rtx->pixels = tx.rgb_array();

                            ImTextureID id = (ImTextureID)(intptr_t)rtx;
                            ImVec2 size    = ImVec2( 
                                    std::min((float)rtx->width*7,  sz.y),
                                    std::min((float)rtx->height*7, sz.y)
                            );

                            if ( stride_used != 0 && stride_used + size.x + 10 < sz.x )
                                stride_used += size.x + 10, ImGui::SameLine();
                            else stride_used = size.x + 10, ImGui::Dummy({
                                    (sz.x - size.x*(sz.x / (size.x + 10)))/2, size.y
                                }), ImGui::SameLine();

                            ImVec2 pos = ImGui::GetCursorScreenPos();
                            ImGui::Image(id, size);
                            if ( ImGui::IsItemHovered() ) {
                                ImVec2 min_pos = pos;
                                ImVec2 max_pos = ImVec2(pos.x + size.x, pos.y + size.y);
                                ImU32 border_color = hex_imu32(#ffaa00);
                                ImGui::GetWindowDrawList()->AddRect(min_pos, max_pos, border_color, 0.0f, 0, 2.0f); 
                                ImGui::BeginTooltip();
                                ImGui_TextAnsi(colorformat(

                                            "~u~TEXTURE INFORMATION~n~"
                                ).c_str());
                                ImGui_TextAnsi(colorformat("\t  NAME:\t" + tx.name));
                                ImGui_TextAnsi(colorformat("\t WIDTH:\t"+ v_str(tx.width)));
                                ImGui_TextAnsi(colorformat("\tHEIGHT:\t"+v_str(tx.height)));
                                ImGui::EndTooltip();
                            };
                            if ( ImGui::IsItemClicked() ) {
                                if ( Gmeng::main_event_loop->editors.contains(
                                    tx.name
                                ) ) Gmeng::main_event_loop->editors.find( tx.name )
                                    ->second->is_open = true;
                                else Gmeng::main_event_loop->editors[tx.name] =
                                    std::make_unique<GmengImGuiTextureEditor>
                                        (GmengImGuiTextureEditor(tx, &tx));
                            };
                        };
                        ImGui::EndTabItem();
                    }
                }
            }
        }
    }},
};

inline void ImGui_GmengEntity(Gmeng::EntityBase* selected_entity, Gmeng::EventLoop* ev) {
            if ( selected_entity == nullptr ) return;

            ImGui_CenteredText(("ENTITY(" + std::to_string( selected_entity->entity_id ) + ") = " + entity_dictionary[ selected_entity->get_serialization_id() ] + "(" + std::to_string(selected_entity->get_serialization_id()) + ")").c_str());
            ImGui::Separator();

            auto sz = ImGui::GetContentRegionAvail();

            ImGui::BeginChild("flags", ImVec2( 280, sz.y / 3 ), ImGuiChildFlags_Borders);
                ImGui_CenteredText("FLAGS");
                int i = 0;

                for ( auto& var : selected_entity->gmeng_variables ) {
                    auto typemap = selected_entity->get_static_type_map();
                    if ( typemap.at(var.first) == "bool" ) {
                        ImGui::Checkbox(var.first.c_str(), (bool*)var.second);
                        if (i++ % 2 == 0) ImGui::SameLine();
                    };
                };
                ImGui_ModifierList(selected_entity->modifiers);
            ImGui::EndChild();

            ImGui::SameLine();

            auto sz2 = ImGui::GetContentRegionAvail();

            ImGui::BeginChild("position", ImVec2(sz2.x, sz.y/3), ImGuiChildFlags_Borders);
            
                ImGui::Text("POSITION(");
                ImGui::SameLine();
                ImGui::SetNextItemWidth(55.0f);
                ImGui::InputInt("##XVALUE", &selected_entity->position.x, 0, 0);

                ImGui::SameLine(); ImGui::Text(",");

                ImGui::SameLine();
                ImGui::SetNextItemWidth(55.0f);
                ImGui::InputInt("##YVALUE", &selected_entity->position.y, 0, 0);

                ImGui::SameLine(); ImGui::Text(")");
                ImGui::SameLine();
                ImGui::Text((" : DSPLCMNT(" + std::to_string(selected_entity->sprite.width) + "," + std::to_string(selected_entity->sprite.height) + ")").c_str());

                    ImGui_2d_position(
                        &selected_entity->position.x,
                        &selected_entity->position.y,
                        vp_width(ev->level->display.viewpoint),
                        vp_height(ev->level->display.viewpoint)
                    );

            ImGui::EndChild();

            ImGui::BeginChild("OPTIONS", ImVec2( 280, sz.y * 0.66 ), ImGuiChildFlags_Borders);

            ImGui_CenteredText("DEFAULT OPTIONS");

                if ( ImGui::Button("ATTACH TEXTURE") ) {};

                ImGui::SameLine();

                if ( ImGui::Button("RESIZE") ) {};

                if ( ImGui::Button("MIRROR TEXTURE") ) {
                    std::reverse( selected_entity->sprite.units.begin(),
                                  selected_entity->sprite.units.end());
                };

                ImGui::SameLine();
                if ( ImGui::Button(" SORT ") ) {
                    gmeng_sort_entities_by_z_index(ev->level->entities);
                };

                ImGui::Separator();

            ImGui_CenteredText("TYPE-SPECIFIC OPTIONS");

            if ( GMENG_ENTITY_TYPE_SPECIFIC_EDITOR_COMMANDS.contains( selected_entity->get_serialization_id() ) ) {
                GMENG_ENTITY_TYPE_SPECIFIC_EDITOR_COMMANDS[
                    selected_entity->get_serialization_id()
                ].short_options( selected_entity );
            } else {
                ImGui_TextAnsi(colorformat(
                    "~h~~r~none available~n~"
                ));
            };

            ImGui::EndChild();

            static std::unordered_map<int, TextEditor> editors;

            ImGui::SameLine();
            
            ImGui::BeginChild("properties", ImVec2(sz2.x, sz.y * 0.66), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

            if ( ImGui::BeginTabBar("ENTITY_TABBAR") ) {

                if (ImGui::BeginTabItem("PREVIEW")) {
                    auto szav = ImGui::GetContentRegionAvail();
                    ImGui_CenteredText("TEXTURE");

                    rgb_texture* rtx = new rgb_texture();
                      rtx->height = selected_entity->sprite.height;
                      rtx->width  = selected_entity->sprite.width;
                      rtx->pixels = selected_entity->sprite.rgb_array();

                    ImTextureID id = (ImTextureID)(intptr_t)rtx;
                    ImVec2 size = {
                        std::min((float)rtx->width*10,  szav.y),
                        std::min((float)rtx->height*10, szav.y)
                    };
                    ImGui::Image(id, size);



                    ImGui::EndTabItem();
                };

                if (ImGui::BeginTabItem("ROUTINES")) {
                    ImGui::BeginChild("routines__1", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_NoScrollbar);
                    ImGui::PushStyleColor(ImGuiCol_Header, hex_imu32(#00000000));

                    for ( auto& routine : selected_entity->routines) {
                        if ( routine.entry_point.size() != 64 ) routine.entry_point.resize(64);
                        if (ImGui::CollapsingHeader(routine.title.c_str())) {

                            if (!editors.contains(routine.id)) {
                                editors[routine.id] = TextEditor();
                                editors[routine.id].SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
                                editors[routine.id].SetText( routine.lua_source );
                            };

                            TextEditor& editor = editors[routine.id];

                            if ( !routine.executable ) ImGui::BeginDisabled();


                            if (ImGui::InputText(ICON_FA_CODE " ENTRY POINT", &routine.entry_point[0], (int)routine.entry_point.size()+1)) {
                                routine.entry_point.resize( strlen( &routine.entry_point[0] ) );
                            };

                            if (ImGui::IsItemHovered()) {
                                ImGui::BeginTooltip();
                                    if ( routine.executable )
                                        ImGui_TextAnsi(colorformat(
                                              "~h~~b~ENTRY POINT~n~ is the name of the "
                                                    "function that is executed on every iteration of the subroutine."
                                        ));
                                    else
                                        ImGui_TextAnsi(colorformat(
                                              "~h~~b~ENTRY POINT~n~ is the name of the "
                                                    "function that is executed on every iteration of the subroutine."
                                                    "\n~u~item disabled because the subroutine is not executable."
                                        ));
                                ImGui::EndTooltip();
                            };

                            ImGui::SliderFloat(ICON_FA_CLOCK " PERIOD LENGTH", &routine.period, 1.0f / ev->level->display.fixed_update_fps, 60.0f);

                            if ( ImGui::IsItemHovered() ) {
                                ImGui::BeginTooltip();
                                if (routine.executable) ImGui_TextAnsi(colorformat(
                                      "~h~~b~PERIOD LENGTH~n~ is the amount of time "
                                            "in seconds between each execution of the subroutine."
                                ));
                                else ImGui_TextAnsi(colorformat(
                                      "~h~~b~PERIOD LENGTH~n~ is the amount of time "
                                            "in seconds between each execution of the subroutine."
                                            "\n~u~item disabled because the subroutine is not executable."
                                ));
 
                                ImGui::EndTooltip();
                            }

                            if ( !routine.executable ) ImGui::EndDisabled();

                            ImGui::Separator();

                                ImGui::PushID( routine.id - 1 );
                                ImGui::Checkbox("EXECUTABLE", &routine.executable);
                                ImGui::PopID();

                                if (ImGui::IsItemHovered()) {
                                    ImGui::BeginTooltip();
                                    ImGui_TextAnsi(colorformat(
                                        "~h~~b~EXECUTABLE~n~ means whether the subroutine will be executed,"
                                        " if disabled, it will be executable from other routines."
                                    ));
                                    ImGui::EndTooltip();
                                };

                                ImGui::SameLine();

                                ImGui::PushID( routine.id );
                                if ( ImGui::Button("REAPPLY SUBROUTINE") ) {
                                    routine.lua_source = editor.GetText();
                                    routine.init = false;
                                }
                                ImGui::PopID();

                                if ( ImGui::IsItemHovered() ) {
                                    ImGui::BeginTooltip();
                                    ImGui_TextAnsi(colorformat(
                                        "~h~~b~REAPPLY SUBROUTINE~n~ when the code for the subroutine is changed."
                                        " otherwise the old function body will be executed."
                                    ));
                                    ImGui::EndTooltip();
                                };

                                ImGui::SameLine();

                                ImGui::PushID(routine.id + 1);
                                ImGui::PushStyleColor(ImGuiCol_Button, hex_imu32(#ff0000));
                                if (ImGui::Button("DELETE SUBROUTINE")) {
                                    ImGui::OpenPopup("Confirm Delete?");
                                };
                                ImGui::PopID();
                                ImGui::PopStyleColor();

                                ImGui::SetNextWindowPos( ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f) );
                                if ( ImGui::BeginPopupModal("Confirm Delete?", NULL, ImGuiWindowFlags_AlwaysAutoResize) ) {
                                    ImGui_TextAnsi(colorformat(
                                        "Are you sure you would like to ~h~~r~delete subroutine"
                                        "(" + v_str( routine.id ) + "): " + routine.entry_point + "~n~?"
                                    ));
                                    if ( ImGui::Button("NO, NEVERMIND") ) ImGui::CloseCurrentPopup();
                                    else if ( ImGui::Button("YES, DELETE") ) {
                                        auto it = 
                                            std::find_if(selected_entity->routines.begin(),
                                                      selected_entity->routines.end(),
                                                      [routine](const Routine& r) {
                                                            return r.id == routine.id;
                                                      });
                                        if (it != selected_entity->routines.end())
                                            selected_entity->routines.erase(
                                                it
                                            );
                                        ImGui::CloseCurrentPopup();
                                    };

                                    ImGui::EndPopup();
                                };

                            ImGui::ProgressBar(routine.executable ? routine.progress / routine.period : 0.0f  );
                            ImGui::Separator();
                            editor.Render(v_str(routine.id).c_str());
                        };
                    };
                    ImGui::PopStyleColor();

                    if ( ImGui::Button(ICON_FA_SQUARE_PLUS " CREATE NEW SUBROUTINE...") ) {
                        Gmeng::Routine new_routine;
                        
                        std::sort( selected_entity->routines.begin(), selected_entity->routines.end(),
                                [](const Routine& first, const Routine& other) {
                            return first.priority < other.priority;
                        });


                        std::string DEFAULT_SCRIPT_TEXT =
                            "-- entity handle for subroutine " + v_str(new_routine.id) + ".\n"
                            "function entity_handle_" + v_str(new_routine.id) + "(entity)\n"
                            "\t-- example subroutine code\n"
                            "\tif gmeng.getkey(gmeng.KEY.gk_A) then\n"
                            "\t\tentity.position.x = entity.position.x - 1\n"
                            "\tend\n"
                            "end\n"
                            "-- do not add any other code beyond this point";

                        new_routine.entry_point = "entity_handle_" + v_str(new_routine.id);
                        new_routine.lua_source = DEFAULT_SCRIPT_TEXT;
                        new_routine.title = v_str(new_routine.id) + ".routine";
                        selected_entity->routines.push_back(new_routine);
                    };

                    ImGui::EndChild();
                    ImGui::EndTabItem();
                };


                if (GMENG_ENTITY_TYPE_SPECIFIC_EDITOR_COMMANDS.contains(
                        selected_entity->get_serialization_id()
                   )) GMENG_ENTITY_TYPE_SPECIFIC_EDITOR_COMMANDS[
                    selected_entity->get_serialization_id()
                ].properties["tabitems"]( selected_entity );

                ImGui::EndTabBar();
            };

            ImGui::EndChild();
};

#include <cstdint>
#if UINTPTR_MAX == UINT64_MAX
    #define imgui_sizetype ImGuiDataType_U64
#else
    #define imgui_sizetype ImGuiDataType_U32
#endif

inline void ImGui_GmengDisplay(Gmeng::Renderer::Display display, Gmeng::EventLoop* ev) {
    auto& camera = display.camera;

    ImGui::BeginChild("modifiers", ImVec2(375, 0), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui_CenteredText(("FLAGS (" + std::to_string(camera.modifiers.values.size()) + ")").c_str());
        ImGui_ModifierList(camera.modifiers);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("camera__", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Borders);

    ImGui_CenteredText(("frame: " + std::to_string(camera.frame_time) + ", draw: " + std::to_string(camera.draw_time)).c_str());
    ImGui::Separator();


    ImGui::PushStyleColor(ImGuiCol_ChildBg, hex_imu32(#00000000));
    ImGui::BeginChild("logs", ImGui::GetContentRegionAvail(), ImGuiChildFlags_Borders, ImGuiWindowFlags_HorizontalScrollbar);
        ImGui::Text(Gmeng::functree_calls.str().c_str());
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::EndChild();
};

#include "../types/IconsFontAwesome7.h"

static void gmeng_level_manager(Gmeng::EventLoop* ev) {
    using namespace Gmeng;

    enum {
        DISPLAY,
        ENTITY,
        MODEL,
        UNSET
    };
    static auto selection_type = UNSET;
    static Renderer::Model* selected_model = nullptr;
    static EntityBase*      selected_entity = nullptr;
    static int selected_id = -1;

    ImGui::SetNextWindowSize({ 1560, 720 });
    ImGui::Begin("gmeng level manager", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

    ImGui::BeginChild("object_selector", ImVec2(400, 0), ImGuiChildFlags_Borders);

    ImGui::PushStyleColor(ImGuiCol_Header, hex_imu32(#00000000));

    ImGui::PushStyleColor(ImGuiCol_Button, hex_imu32(#00000000));
    if ( ImGui::Button("DISPLAY") ) {
        selection_type = DISPLAY;
    }
    ImGui::PopStyleColor();

    if (ImGui::CollapsingHeader(("MODELS (" + std::to_string( ev->level->modelmap.size() ) + ")").c_str())) {
        for ( auto& model : ev->level->modelmap ) {
            if ( selected_id == model.second->id ) ImGui::PushStyleColor(ImGuiCol_Button, hex_imu32(#2665B4));
            else ImGui::PushStyleColor(ImGuiCol_Button, hex_imu32(#00000000));

            if (ImGui::Button( (std::to_string(model.second->id) + ' ' + model.second->name).c_str() )) {
                selection_type = MODEL;
                selected_model = model.second;
                selected_id = model.second->id;
            };

            ImGui::PopStyleColor();
        };
    };

    if (ImGui::CollapsingHeader(("ENTITIES (" + std::to_string( ev->level->entities.size() ) + ")").c_str())) {
        for (int eid = (int)ev->level->entities.size() - 1; eid >= 0; --eid) {
            auto& entity_ = ev->level->entities[eid];

            if ( selected_id == entity_->entity_id ) ImGui::PushStyleColor(ImGuiCol_Button, hex_imu32(#2665B4));
            else ImGui::PushStyleColor(ImGuiCol_Button, hex_imu32(#00000000));

            if (ImGui::Button( (std::to_string(entity_->entity_id) + ' ' + entity_dictionary[entity_->get_serialization_id()]).c_str() )) {
                selection_type  = ENTITY;
                selected_entity = entity_.get();
                selected_id = entity_->entity_id;
            };
            ImGui::SameLine();

            ImGui::PushID(entity_->entity_id + 1);
            if (ImGui::Button(ICON_FA_CARET_UP)) {
                if ( eid >= (int)ev->level->entities.size()-1 )
                    entity_->modifiers["z_index"].value++;
                else
                    entity_->modifiers["z_index"].value =
                        ev->level->entities[eid + 1]->modifiers["z_index"].value + 1;

                gmeng_sort_entities_by_z_index(ev->level->entities);
            }
            ImGui::PopID();

            ImGui::SameLine();

            ImGui::PushID(entity_->entity_id - 1);
            if (ImGui::Button(ICON_FA_CARET_DOWN)) {
                if ( eid == 0 )
                    entity_->modifiers["z_index"].value--;

                else
                    entity_->modifiers["z_index"].value =
                        ev->level->entities[eid - 1]->modifiers["z_index"].value - 1;

                gmeng_sort_entities_by_z_index(ev->level->entities);
            }
            ImGui::PopID();

            ImGui::PopStyleColor();
        };
    };

    ImGui::PopStyleColor();

    ImGui::EndChild();

    ImGui::SameLine();

    static std::map<int, float> texture_zoom_values;

    ImGui::BeginChild("entity_editor", ImVec2(0, 0), ImGuiChildFlags_Borders);

    switch ( selection_type ) {
        case DISPLAY: ImGui_GmengDisplay(ev->level->display, ev); break;
        case ENTITY: ImGui_GmengEntity(selected_entity, ev); break;
        case MODEL:  {

                     }; break;
        case UNSET:  {

                     }; break;
    };

    ImGui::EndChild();
    ImGui::End();
};
