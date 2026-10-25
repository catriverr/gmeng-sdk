#pragma once

#include "global.h"
#include "gm_imgui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "kitty.h"
#include "level_manager.h"
#include "script_manager.h"

#include "../src/gmeng.cpp"


#include <string>


/// commands for the in-game developer console/
/// the commands defined below (the default implementations)
/// are part of the gmeng internal toolkit.
///
/// more commands can be added later on with Noble dylib scripts.
/// see 'github.com/catriverr/noble' and 'include/noble' on how they work.
///
/// for a list of commands, run the 'help' command in a game instance.
/// the developer console is handled internally, so no set-up functions need to be called.
///
/// a do_event_loop() instance handles the internal workings of the engine automatically.
static vector< std::tuple<string, std::function<int(vector<string>, Gmeng::EventLoop*)>> > gmeng_console_commands = {
        { "resize", [](vector<string> params, Gmeng::EventLoop* ev) -> int{
                params.erase(params.begin());

                if ( params.size() < 2 ) {
                    resize_usage:
                    GAME_LOG("resize: resizes the terminal (in pixels)\nusage: resize <width> <height>");
                    return 1;
                };

                    int start = std::stoi( params.at(0) );
                    int end = std::stoi( params.at(1) );

                    kitty_resize_window_pixels( start, end );
                return 0;
            } },
        { "setvp", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
                params.erase( params.begin() );

                if ( params.size() < 2 ) {
                    usage:
                    GAME_LOG("setvp: sets the display viewpoint.\nusage: setvp 0,0 10,10\nsetvp <start_xy> <end_xy>");
                    return 1;
                };

                auto start = g_splitStr( params.at(0), "," );
                auto end   = g_splitStr( params.at(1), "," );
                if ( start.size() != end.size() || start.size() != 2 || end.size() != 2 ) {
                    goto usage;
                    return 1;
                };

                std::vector<int> nums;
                try {
                    for ( auto s : start ) {
                        nums.push_back( std::stoi( s ) );
                    };
                    for ( auto e : end ) {
                        nums.push_back( std::stoi(e) );
                    };
                } catch (std::exception& e) { goto usage; return 1; };

                for ( auto n : nums ) GAME_LOG("val: "$(n)".");

                ev->level->display.viewpoint = {
                    { nums.at(0), nums.at(1) },
                    { nums.at(2), nums.at(3) }
                };
                GAME_LOG("Set viewpoint to " + g_joinStr( params, " -> " ));

                return 0;
            } },
        { "lua", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            #ifdef GMENG_NO_SOL
                GAME_LOG("gmeng:lua support disabled by the developer");
                return 1;
            #else

            params.erase(params.begin());

            if ( params.size() < 1 ) {
                GAME_LOG("lua: evaluates a lua expression.\nusage: lua <expression>");
                return 1;
            };

            try {
                Gmeng::Scripts::lua.script( g_joinStr(params, " ") );
            } catch(std::exception& e) {
                GAME_LOG((std::string)e.what());
            };

            return 0;

            #endif
        } },
        { "echo", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            string joined = g_joinStr( params, " " );

            replace_all(joined, "$!__E", "\033");

            GAME_LOG(colorformat(joined, false));
            return 0;
        } },
        { "force_update", [](vector<string>, Gmeng::EventLoop* ev) -> int {
            ev->call_event( Gmeng::UPDATE, Gmeng::NO_EVENT_INFO );
            ev->call_event(Gmeng::FIXED_UPDATE, Gmeng::NO_EVENT_INFO);
            return 0;
        } },
        { "restart", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            GAME_LOG("[gmeng_internal] restarting...");
            GAME_LOG("replacing current executable image...\nexecvp()");
            int state = restart_program();
            if (state != 0) {
                GAME_LOG("an error occurred while restarting\nthe current program.\n");
                GAME_LOG("gmeng state: "$(state)" - errno: "$(errno)"");
            };

            return 0;
        } },
        { "setwidth", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            if (params.size() < 1) {
                GAME_LOG("usage: setwidth <console_width(int)>");
                GAME_LOG("this command will crash the game\nif you use a\nnon-integer-conversible parameter");
                return 1;
            } else GMENG_DEV_CONSOLE_WIDTH = std::stoi(params.at(0));
            return 0;
        } },
        { "setheight", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            if (params.size() < 1) {
                GAME_LOG("usage: setheight <console_height(int)>");
                GAME_LOG("this command will crash the game\nif you use a\nnon-integer-conversible parameter");
                return 1;
            } else GMENG_DEV_CONSOLE_HEIGHT = std::stoi(params.at(0));
            return 0;
        } },
        { "crash", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
                /// deliberately cause a crash with a segmentation fault.
                Gmeng::TerminalUtil::disable_mouse_tracking();
                gmeng_keyboard_deinit();
                int* ptr = nullptr;
                *ptr = 42;
            return 1;
        } },
        { "exit", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            GAME_LOG("quiting...");
            ev->cancel();
            return 0;
        } },
        { "showtype", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            /// TODO: multiple types
            GAME_LOG("Gmeng::EventLoop {");
            for ( auto v : ev->gmeng_variables ) {
                auto type_name = Gmeng::EventLoop::get_static_type_map().at( v.first );
                GAME_LOG( "    " + v.first + " -> " + type_name );
            };
            GAME_LOG("}");
            return 0;
        } },
        { "erasemod", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            if ( params.size() < 1 ) {
                GAME_LOG("usage: erasemod <name>\n" + (std::string)
                         "deletes the first instance of a modifier");
                return 1;
            };
            ev->level->display.camera.modifiers.erase(params.at(0));
            GAME_LOG("try_erased modifier " + params.at(0) + '.');
            return 0;
        } },
        { "mod", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
            params.erase(params.begin());
            if (params.size() < 1) { GAME_LOG("usage: mod modifier_name val<int>"); return 1; };
            if (params.size() < 2) {
                GAME_LOG("modifier " + std::string(params.at(0)) + std::string(" = ") + v_str(ev->level->display.camera.modifiers.get_value(params.at(0)))); return 0; };
            try {
                int val = std::stoi(params.at(1));
                if ( ev->level->display.camera.modifiers.get_value(params.at(0)) != -1 )
                     ev->level->display.camera.modifiers.set_value(params.at(0), val);
                else ev->level->display.camera.modifiers.add_value(params.at(0), val);
            } catch(std::invalid_argument& e) {
                GAME_LOG("invalid modifier value. must be int");
                return 1;
            };
            return 0;
        } },
        { "clear", [](vector<string>, Gmeng::EventLoop* ev) -> int { GAME_LOGSTREAM->clear(); return 0; } },
        { "refresh", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                ev->level->display.camera.clear_screen();
                return -13; // -13 means no vertical line to divide the console after the command is ran
            } },
        { "help", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                for (auto& cmd : gmeng_console_commands) {
                    string name; std::function<int(vector<string>, Gmeng::EventLoop*)> handler;
                    std::tie(name, handler) = cmd;
                    GAME_LOG("cmd: `" + name + "`");
                };
                GAME_LOG("\nSDL keybinds:");
                GAME_LOG("  |shift + TAB| : toggle developer console");
                GAME_LOG("  | alt + TAB | : toggle vgm model editor");
                GAME_LOG("  | alt +  L  | : toggle level inspector");
                return 0;
            } },
        { "info", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                GAME_LOG("level " + ev->level->name + "");
                GAME_LOG("desc: " + ev->level->desc);
                GAME_LOG("chunks: " + v_str(ev->level->chunks.size()));
                int model_count = 0;
                for (const auto ch : ev->level->chunks) {
                    model_count += ch.models.size();
                };
                GAME_LOG("models: "$(model_count)"F");
                GAME_LOG("frame time: "$(ev->level->display.camera.frame_time)"ms");
                GAME_LOG("draw time: "$(ev->level->display.camera.draw_time)"ms");
                return 0;
            } },

        { "runscript", [](vector<string> params, Gmeng::EventLoop* ev) -> int {
                if (params.size() < 2) { GAME_LOG("usage: runscript <script name>"); GAME_LOG("runs a NOBLE prebuilt script."); return 1; };
                std::string filename = params.at(1);
                if (!filesystem::exists(filename)) {
                    GAME_LOG("ERR: script \"" + filename + "\" doesn't exist, filesystem::exists() nonzero");
                    return 1;
                };
                #ifndef GMENG_COMPILING_SCRIPT
                try {
                    GAME_LOG("file exists, pass");
                    GAME_LOG("initializing script...");
                    GAME_LOG("importing noble dylib handle");
                    // actual import
                    // handle of the dylib
                    auto scr_handle = noble_file_open(filename);
                    GAME_LOG("script import OK");

                    GAME_LOG("attempting to execute <script_obj_dir>::gmeng_script_command()");
                    int comm_execution_result = noble_function(scr_handle, gmeng_script_command, ev);
                    GAME_LOG("execution complete, command result: "$(comm_execution_result)".");

                    dlclose(scr_handle);
                    return 0;
                } catch (const std::exception& e) {
                    GAME_LOG("error at some point: " + std::string(e.what()) + " - can not execute script");
                    return 1;
                };

                #else
                GAME_LOG("how did you run this method from a string anyways");
                return -1;
                #endif
            } },
        { "savelvl", [](vector<string>, Gmeng::EventLoop* ev) -> int {
                GAME_LOG("saving current gamestate & level data.");
                GAME_LOG("FOR DEVELOPERS: add a save function in an event hook for EXIT to make this work");
                // Exit should always save. It'd be the developer's fault if this didn't work, not ours.
                // Even though it's extremely annoying to the user for the engine to do this.
                ev->call_event(Gmeng::Event::EXIT, Gmeng::NO_EVENT_INFO);
                return 0;
            } },
        { "newton", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                Gmeng::LightSource new_light_source;
                int val = 15;
                if (args.size() == 1) {
                        GAME_LOG("\tusage: newton [brightness]");
                        return 1;
                };
                if (args.size() >= 2) {
                    try {
                        val = std::stoi( args.at(1) );
                    } catch (std::exception& ex) {
                        GAME_LOG("invalid argument, using default brightness of 15.");
                        GAME_LOG("anton chigurh:");
                        GAME_LOG("\tusage: newton [brightness]");
                    };
                }
                new_light_source.position = { val, val };
                new_light_source.intensity = val;
                GAME_LOG("God Said 'Let Newton be!' and there was light. (" + args.at(1) + ")");
                ev->level->entities.push_back(std::make_shared<Gmeng::LightSource>(new_light_source));
                return 0;
            } },
        { "embed_engine", [](auto, Gmeng::EventLoop* ev) -> int {
                Gmeng::SERIALIZED_ENGINE_INFO engine_info;

                engine_info.sprite.name = "ENGINE_INFO";

                bool done = false;
                for ( auto& entity : ev->level->entities ) {
                        if ( entity->get_serialization_id() == Gmeng::SERIALIZED_ENGINE_INFO::id ) {
                        GAME_LOG("found previous data, updating.");
                        auto av = std::make_shared<Gmeng::SERIALIZED_ENGINE_INFO>( engine_info );
                        entity = av;
                        done = true;
                    };
                };

                if (!done) {
                    GAME_LOG("no previous data, applying new.");
                    ev->level->entities.push_back(std::make_shared<Gmeng::SERIALIZED_ENGINE_INFO>( engine_info ));
                };

                GAME_LOG("done");
                return 0;
            } },
        { "lighting_redoall", [](auto, Gmeng::EventLoop* ev) -> int {
                GAME_LOG("deleting & invalidating all lighting cache");
                ev->level->lighting_cache.clear();
                for (auto lightsource : ev->level->light_sources) {
                    lightsource.second->cached = false;
                }
                return 0;
            } },
        { "tutorial", [](auto args, Gmeng::EventLoop* ev) -> int {

            GAME_LOG("\n>>> CONSOLE(0): USER MANUAL");

            GAME_LOG("\nwelcome to the gmeng debug & development console.");
            GAME_LOG("this tutorial will help you get started with using it.");
            GAME_LOG(colorformat("visit ~u~~b~https://gmeng.org/console~n~ for a more in-depth guide."));

            GAME_LOG("\n>>> CONSOLE(1): SYPNOSIS");

            GAME_LOG("\nthe console works like a unix shell. you feed commands into it,");
            GAME_LOG("and it will execute them and return an errorlevel, where 0 = success.");
            GAME_LOG(colorformat("commands can have arguments, which are passed as a ~b~vector<string>~n~."));
            GAME_LOG(colorformat("\nfor example, the command ~b~'echo'~n~ compiles all of its arguments"));
            GAME_LOG("into a single string and prints it to the console output.");
            GAME_LOG("\ncommand arguments will include the called command too. this means");
            GAME_LOG("that args[0] will always be the command being called. So, if we run");
            GAME_LOG(colorformat("~b~echo hello, world!~n~ then args[0] will be 'echo'. commands that depend"));
            GAME_LOG("on its arguments need to check them starting from args[1]. Just like C's");
            GAME_LOG("entry-point function `int main(int argc, char** argv`, where the program");
            GAME_LOG("will receive argv[0] as the call to the program path by which it is called.");

            GAME_LOG("\n>>> CONSOLE(2): EXECUTING COMMANDS");

            GAME_LOG("\nthe console can execute commands either with echo-on or echo-off mode.");
            GAME_LOG("the difference is that in echo-off mode, no artifacts from the command");
            GAME_LOG("execution process will be displayed. no 'success/fail' outputs,");
            GAME_LOG("no command splitters, etc. this is particularly useful in scripts.");

            GAME_LOG(colorformat("\nTo run a command in echo-off mode, start the command with: ~u~@~n~"));
            GAME_LOG("in the input prompt. So, '@echo hello, world!', '@clear', '@runscript'...");

            GAME_LOG("\n>>> CONSOLE(3): SCRIPTING");

            GAME_LOG("\nscripts can be called with the built-in command: 'call'.");
            GAME_LOG("when launching, gmeng will check the program's directory for files");
            GAME_LOG("with the extension: '.gmeng'. These files are gmeng script files which");
            GAME_LOG("contain lines of commands that are executed line-by-line. It isn't a language.");
            GAME_LOG("It's not turing complete. It doesn't have logic. However, the extension from");
            GAME_LOG("`scripts/console_logic.dylib` will give some support for logic operations.");

            GAME_LOG("\nAlternative to the 'call' command, you may also start your prompt with the");
            GAME_LOG("special character '!' to assert that you are executing a script. This will");
            GAME_LOG("provide the same functionality as running the 'call' command, but cleaner.");

            GAME_LOG("\n>>> CONSOLE(4): FORMATTED OUTPUTS");

            GAME_LOG("\nthe console supports different text formatting options.\n");

            GAME_LOG(colorformat("~u~ANSI ESCAPE SEQUENCES~n~"));
            GAME_LOG("  the console fully supports ANSI escape sequences, including truecolor.");
            GAME_LOG("  'echo' recognizes the inline text '$!__E' as the escape key (\\033).");

            GAME_LOG(colorformat("\n~u~SEPARATORS~n~"));
            GAME_LOG("$!__NF  you can output title separators by writing: '>>> Heading Name'.");
            GAME_LOG("$!__NF  it is also possible to write out a line divider by writing: '$!__SP'.");
            GAME_LOG("  note that both have to be at the beginning of the log line in the console.");

            GAME_LOG(colorformat("\n~u~DISABLE FORMATTING~n~"));
            GAME_LOG("  to disable formatting (excluding colorcodes), begin the log with '$!__NF'");
            GAME_LOG("  this way, even if the text that follows this code is a formatter,");
            GAME_LOG("  its style will be ignored and displayed as raw text only.");


            return 0;
        } },
        { "ent_list", [](auto args, Gmeng::EventLoop* ev) -> int {


                if ( args.size() > 1 && args.at(1) == "types" ) {
                    GAME_LOG("entity types:");
                    for ( auto ent_type_listing : Gmeng::entity_dictionary ) {
                        GAME_LOG("ENTITY("$( ent_type_listing.first )") = " + ent_type_listing.second);
                    };
                    return 0;
                };

                for ( auto entity : ev->level->entities ) {
                    int ent_sID = entity->get_serialization_id();
                    std::string ent_type = ent_sID == Gmeng::Player::id ? "P" : (
                                ent_sID == Gmeng::LightSource::id ? "L" : (
                                    ent_sID == Gmeng::GMENG_ENTITYTYPES_END::id ? "!" : (
                                            ent_sID == Gmeng::SERIALIZED_ENGINE_INFO::id ? "G" : "E"
                                        )
                                )
                            );
                    GAME_LOG(v_str(ent_sID) + " " + ent_type + " " +
                            _uconv_1ihx(entity->entity_id) +
                            " "$(entity->entity_id)" (" + Gmeng::entity_dictionary.at(ent_sID) + ")");
                    GAME_LOG("  position: " + Gmeng::Renderer::conv_dp( entity->position ));
                    GAME_LOG("  active: "$(entity->active)"");
                    GAME_LOG("  interaction proximity: "$(entity->interaction_proximity)"");
                };
                GAME_LOG("\n"$(ev->level->entities.size()) + " entities total");
                return 0;
            } },
        { "ent", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                if (args.size() < 4) {
                    GAME_LOG("universal entity parameter modifier");
                    GAME_LOG("usage: ent <entity id> <modifier name> <value>");
                    GAME_LOG("this command is only for the base properties of the Entity class.");
                    GAME_LOG("you can not modify derived entity parameters (by default).");
                    return 1;
                };
                auto arg_ent_id = args.at(1);
                auto modifier_name = args.at(2);
                auto new_value = args.at(3);
                int ent_id = 0;
                try { ent_id = std::stoi(arg_ent_id); }
                catch (std::exception& e) { GAME_LOG("invalid entity id entered"); return 1; };

                bool found = false;
                for ( auto ent : ev->level->entities ) {
                    if ( ent->entity_id  == ent_id ) {
                        found = true;
                        // x position modifier
                        if ( modifier_name == "posx" ) {
                            try {
                                int new_pos = std::stoi(new_value);
                                ent->position.x = new_pos;
                            } catch (std::exception& e) { GAME_LOG(e.what()); };
                        }
                        // y position modifier
                        else if ( modifier_name == "posy" ) {
                            try {
                                int new_pos = std::stoi(new_value);
                                ent->position.y = new_pos;
                            } catch (std::exception& e) { GAME_LOG(e.what()); };
                        }
                        else if ( modifier_name == "active" ) {
                            bool should_it_be = stob(new_value);
                            ent->active = should_it_be;
                        }
                        else if ( modifier_name == "prox" ) {
                            try {
                                int proximity = std::stoi(new_value);
                                ent->interaction_proximity = proximity;
                            } catch (std::exception& e) { GAME_LOG(e.what()); };
                        } else {
                            GAME_LOG("unknown modifier '"+ modifier_name + "'.\nthis command only modifies parameters from the base Entity class.\nderived parameters can not be modified (by default).");
                            return 1;
                        };
                        // if the entity has been found, don't search the next entities.
                        break;
                    };
                };
                if (!found) { GAME_LOG("could not find entity with the specified id"); return 1; }
                else {
                    GAME_LOG("modifier " + modifier_name + " of entity "$(ent_id)" set to " + new_value + ".");
                    return 0;
                };
            } },
        { "ent_create", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                int ent_type;
                if (args.size() < 2) {
                    GAME_LOG("usage: ent_create <entity derived class id>");
                    return 1;
                };
                try { ent_type = std::stoi( args.at(1) ); } catch(std::exception) {
                    GAME_LOG("invalid parameter. pass a number value.");
                    return 1;
                };

                auto entity_factory = Gmeng::EntityBase::get_derived_factory();
                auto derived_type = entity_factory.find( ent_type );

                if ( derived_type == entity_factory.end() ) {
                    GAME_LOG("invalid derived class id. run 'ent_list types' to see all class ids.");
                    return 1;
                };

                auto entity_ = derived_type->second();
                std::shared_ptr<Gmeng::EntityBase> entity = std::move(entity_);
                ev->level->entities.push_back(entity);
                return 0;
            } },
        { "edit_texture", [](vector<string> args, Gmeng::EventLoop* ev) -> int {
                std::string fname = "texture_"$(g_mkid())".gt";
                if (args.size() > 1) fname = args.at(1);

#if GMENG_SDL != true
                if ( !Gmeng::global.terminal_has_kgp_pixel_support ) {
                    GAME_LOG("your terminal does not support the Kitty Graphics protocol.");
                    return 0;
                };
#endif

                GAME_LOG("creating or summoning instance GmengImGuiTextureEditor(" + fname + ")...");

                if ( ev->editors.contains(fname) ) ev->editors.find( fname )->second->is_open = true;
                else ev->editors[fname] = std::make_unique<GmengImGuiTextureEditor>(GmengImGuiTextureEditor( fname ));

                GAME_LOG("[edit_texture/" + fname + "] editor.create complete");
                return 0;
        } },
};

/// executes a developer command to the target of an EventLoop pointer '* ev'
/// 'noecho' means no INTERNAL output will be generated by the command to the console.
/// the command can still echo text. It just will not generate logs like 'unknown command', or splitter lines
inline int gmeng_run_dev_command(Gmeng::EventLoop* ev, const std::string& _command, bool noecho = false, TRACEFUNC) {
    __functree_call__(gmeng_run_dev_command);
#define _GAME_LOG(x) do { if (!noecho) GAME_LOG(x); } while(0)
    using namespace Gmeng;
    using namespace Gmeng::Renderer;
    using std::string, std::vector, std::deque, std::cout;


    std::string command = trim(_command, false);
    if ( command.starts_with("@") )
        noecho = true, command = command.substr(1);

    if (!noecho) gm_log("command '" + command + "' called from origin " + TRACEFUNC_STR);

    vector<string> params = g_splitStr(command, " ");

    if (params.size() < 1) return -1; // no empty commands like ' ' or ''

    _GAME_LOG(">>> " + command);

    Level* level = ev->level;
    Display* display = &ev->level->display;
    Camera<0, 0>* camera = &ev->level->display.camera;

    std::string cmdname = params[0];
    bool no_line;

    int state = -11151; /// -11151 is no command found by default

    for (auto &cmd : gmeng_console_commands) {
        string name; std::function<int(vector<string>, EventLoop*)> handler;
        std::tie(name, handler) = cmd;
        if (name == cmdname) state = handler(params, ev);
    };

    if (state == -11151) { GAME_LOG("unknown command: " + params.at(0)); }
    else if (state == -13) no_line = true; // don't draw vertical line seperator

    variables.at(0) = { "errorlevel", state }; // set the errorlevel value
    ev->call_event(UPDATE, Gmeng::NO_EVENT_INFO); // fixed update can be called afterwards

    if ( command != "refresh" ) ev->call_event(UPDATE, Gmeng::NO_EVENT_INFO);


    _GAME_LOG("> " + v_str(state) + ";" + command); // command name

    return state == -11151 ? 1 : (state == -13 ? 0 : state);
};
static bool console_draw_lock = false;

#include "imgui_ansitext.h"

inline bool block_cursor_input(const char* label, const char* hint, char* buf, size_t buf_size)
{
    ImGuiContext& g = *GImGui;
    ImGuiID id = ImGui::GetID(label);
    
    bool is_active = (g.ActiveId == id && g.InputTextState.ID == id);
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

    bool enter_pressed = ImGui::InputTextWithHint(label, hint, buf, buf_size, flags);

    if (is_active)
    {
        float anim_timer = g.InputTextState.CursorAnim;
        
        bool is_cursor_visible_this_frame = (!g.IO.ConfigInputTextCursorBlink) 
                                         || (anim_timer <= 0.0f) 
                                         || (std::fmodf(anim_timer, 1.20f) <= 0.80f);

        if (is_cursor_visible_this_frame)
        {
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            ImVec2 frame_padding = g.Style.FramePadding;
            ImRect input_rect = g.LastItemData.Rect;

            int cursor_char_idx = g.InputTextState.GetCursorPos();
            float cursor_offset_x = 0.0f;
            if (cursor_char_idx > 0 && (size_t)cursor_char_idx <= strlen(buf))
            {
                cursor_offset_x = ImGui::CalcTextSize(buf, buf + cursor_char_idx).x;
            }

            float cursor_x = input_rect.Min.x + frame_padding.x + cursor_offset_x - g.InputTextState.Scroll.x;
            float cursor_y = input_rect.Min.y + frame_padding.y;

            char current_char = '\0';
            if ((size_t)cursor_char_idx < strlen(buf))
            {
                current_char = buf[cursor_char_idx];
            }

            char char_str[2] = { current_char, '\0' };
            float block_width = (current_char != '\0') ? ImGui::CalcTextSize(char_str).x : ImGui::CalcTextSize("W").x; 
            float block_height = ImGui::GetTextLineHeight();

            ImVec2 p_min = ImVec2(cursor_x, cursor_y);
            ImVec2 p_max = ImVec2(cursor_x + block_width, cursor_y + block_height);

            ImVec2 clip_min = ImVec2(input_rect.Min.x + frame_padding.x, input_rect.Min.y);
            ImVec2 clip_max = ImVec2(input_rect.Max.x - frame_padding.x, input_rect.Max.y);
            
            draw_list->PushClipRect(clip_min, clip_max, true);
            
            ImU32 block_color = IM_COL32(255, 255, 255, 255); 
            draw_list->AddRectFilled(p_min, p_max, block_color);
            
            if (current_char != '\0')
            {
                ImU32 inverted_text_color = IM_COL32(0, 0, 0, 255);
                draw_list->AddText(p_min, inverted_text_color, char_str);
            }
            
            draw_list->PopClipRect();
        }
    }

    return enter_pressed;
}

static bool GMENG_CONSOLE_FOCUSED = false;

static void gmeng_kgp_console( Gmeng::EventLoop* ev, Gmeng::EventInfo* info ) {
    static bool level_manager_open = false;

    if ( console_draw_lock ) return;
    if ( !dev_console_open ) return;

    console_draw_lock = true;

    using namespace Gmeng;
    using namespace Gmeng::Renderer;
    using std::string, std::vector, std::deque, std::cout;

    static bool script_manager = false;
    static GmengImGuiScripts script_window;

    static int last_result = -1;
    static char commandline[512];
    static bool run = true;


    if ( level_manager_open ) gmeng_level_manager(ev);

    ImGui::SetNextWindowSize(ImVec2(1500, 1000));
    ImGui::Begin(("gmeng " + Gmeng::version + " | " + GMENG_BUILD_NO + " development console").c_str(), NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

    auto s = ImGui::GetWindowContentRegionMax();

    ImGui::BeginChild("log", ImVec2( s.x - 562.5, s.y - 36 ), ImGuiChildFlags_Borders, ImGuiWindowFlags_NoScrollbar);

    auto cs = ImGui::GetWindowContentRegionMax();

    for ( auto& s : *GAME_LOGSTREAM ) {
        int stylec = 0;
        int stylev = 0;

        short cut_str = 0;

        if ( s.starts_with("$!__NF") ) {
            cut_str += 6;
        };

        if ( s.starts_with("$!__SP") ) {
            ImGui::Separator(); continue;
        };

        if ( s.starts_with(">>> ") ) {
            ImGui::SeparatorText(s.substr(4).c_str());
            continue;
        };

        if ( s.starts_with("> ") ) {
            int pos = s.find_first_of(';');
            cut_str += pos;
            int v = std::stoi(s.substr(2, pos-1));

            stylec++;
            if ( v == 0 ) ImGui::PushStyleColor( ImGuiCol_Text, hex_imu32(#91db69) );
            else ImGui::PushStyleColor( ImGuiCol_Text, hex_imu32(#e83b3b) );
        };

        ImGui_TextAnsi(s.substr(cut_str).c_str());
        ImGui::PopStyleColor(stylec);
        ImGui::PopStyleVar(stylev);
    };

    if (run) ImGui::SetScrollHereY(1.0f);


    ImGui::SetNextItemWidth( cs.x - 10 );
    if (run) run = false, ImGui::SetKeyboardFocusHere();
    if (block_cursor_input("##gmeng_console_input", "enter command to execute...",
                     commandline,
                     IM_ARRAYSIZE(commandline)
                                    ) ) {
        last_result = gmeng_run_dev_command(ev, std::string(commandline));
        run = true;
        commandline[0] = '\0';
    };

    GMENG_CONSOLE_FOCUSED = ImGui::GetID("##gmeng_console_input") == ImGui::GetActiveID();

    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("right-side info", ImVec2(0, 0), ImGuiChildFlags_Borders);

    ImGui_CenteredText(( "GMENG " + Gmeng::version + ", BUILD " + GMENG_BUILD_NO + ". " + ("FPS: " + v_str((int)( 1 / ev->level->display.profiler.duration.count() ))).c_str()).c_str());

    ImGui::Separator();

    static auto typemap = Gmeng::global.get_static_type_map();

    ImGui::PushStyleColor(ImGuiCol_Header, hex_imu32(#00000000));

    if (ImGui::CollapsingHeader(("FLAGS (" + v_str(typemap.size()) +")").c_str())) {

        for ( const auto& flag : Gmeng::global.gmeng_variables ) {
            if (typemap.at(flag.first) == "bool") {
                ImGui::Checkbox( flag.first.c_str(), (bool*)flag.second );
            };
        };

    };

    auto c2s = ImGui::GetContentRegionAvail();

    if (ImGui::CollapsingHeader(("MODIFIERS (" + v_str(ev->level->display.camera.modifiers.values.size()) + ")").c_str())) {
        int i = 0;
        for ( auto& mod : ev->level->display.camera.modifiers.values ) {
            ImGui::SetNextItemWidth( 300 );
            if (i == 0 || mod.name != "allow_plugins")
                ImGui::InputInt((mod.name + ":" + v_str(i++)).c_str(), &mod.value);
        };
    };

    ImGui::PopStyleColor();

    ImGui::Separator();

    if (ImGui::Button("GMENG DEBUG & DEVELOPER CONSOLE: USER MANUAL")) {
        gmeng_run_dev_command(ev, "@tutorial");
    };

    if ( ImGui::Button("GMENG TEXTURE EDITOR") ) {
        gmeng_run_dev_command(ev, "@edit_texture");
    };

    ImGui::SameLine();

    static bool script_manager_opened_on_this_frame = false;

    if ( script_manager ) ImGui::PushStyleColor(ImGuiCol_Button, hex_imu32(#91db69));

    if ( ImGui::Button("GMENG SCRIPT MANAGER") ) {
        script_manager = !script_manager;
        script_manager_opened_on_this_frame = script_manager;
        if ( !script_manager ) ImGui::PopStyleColor();
    };

    if ( script_manager && !script_manager_opened_on_this_frame )
        ImGui::PopStyleColor(), script_window.Draw();

    if ( script_manager_opened_on_this_frame )
        script_manager_opened_on_this_frame = false;

    if (ImGui::Button("GMENG LEVEL MANAGER")) {
        level_manager_open = !level_manager_open;
    };

    ImGui::EndChild();

    ImGui::End();
    console_draw_lock = false;
};
