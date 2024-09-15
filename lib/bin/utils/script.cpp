#pragma once

#include "../gmeng.h"
#include "../strops.h"
#include "../types/script.h"

#include "./error.h"

#include <vector>
#include <string>
#include <map>

namespace Gmeng::script_parser_util {
    using namespace Gmeng::Scripts; using std::map, std::string, std::vector;
    struct script_file {
        string filename;
        int current_line;
    };
    script_file gmng_scrinternal = { .filename = "gmeng:internals", .current_line=0 };
    script_file gmng_scrrepl = { .filename = "script:REPL", .current_line=0 };


    namespace builtin {
        const object_class number {
            .name = "number",
            .hash = v_str(g_mkid()),
            .methods = map<std::string, hashed_function> {
                { "to_string", hashed_function {
                    .parameters = {  },
                    .hashed_id = "to_string",
                    .parent = nullptr,
                    .name = "to_string"
                } },
            }
        };
        const object_class string = {
            .name = "string",
            .hash = v_str(g_mkid()),
            .methods = map<std::string, hashed_function> {
                { "empty", hashed_function {
                    .parameters = { function_parameter { .name="slice_until" } },
                    .hashed_id = v_str(g_mkid()) + "_empty",
                    .parent = nullptr,
                    .name = "empty",
                } },
            },
            .variables = map<std::string, base_class> {
                { "content", base_class {
                              .name = "$!_BASE_STRING_DONT_USE",
                              .hashed_id = builtin::string.hash + "_content",
                } }
            }
        };

        void make_string_type(object_class* obj) {
            obj->name = string.name;
            
        };
    };

    class Scope {
      public:
        map<string, object_class> classes;
        map<string, hashed_function> functions;
        map<string, object> variables;
        script_file* file = &gmng_scrrepl; /// unless a file that contains this scope is set, consider it a repl
      private:
        inline object* parse_object(string texts) {
            object* dat = new object();
            if (!texts.starts_with("!!")) return nullptr;

            string text = trim(texts);
            string typename_ = text.substr(2); // remove !!

            if (!this->classes.contains(typename_)) throw script_type_exception(file->filename + ":" + v_str(file->current_line) + " :: reference to undefined type " + typename_ + "");
            vector<string> params = split_string(trim(text.substr(typename_.length())), '=');
            if (params.size() <= 2) throw script_syntax_exception(file->filename + ":" + v_str(file->current_line) + " :: variable declaration requires arguments");

            string varname_  = trim(params[0]), value_ = trim(params[1]);
            decltype(this->classes.find(typename_)->second) t_obj;
            t_obj = *this->parse_value(value_);

            return dat;
        };

        inline object_class* parse_value(string data) {
            object_class* obj = new object_class();
            string v_data = trim(data);
            if (v_data.starts_with("\"")) {
                if (!v_data.ends_with("\"")) throw script_syntax_exception("string not closed, missing '\"' character at the end of string");
                obj = builtin::string;
            } else if (v_data.starts_with("{")) {
                if (!v_data.ends_with("}")) throw script_syntax_exception("variable call must follow {var_name} syntax");
                
            };
            return obj;
        };
    };
};

#define GMENG_SCRIPT_INTERPRETER_INIT
