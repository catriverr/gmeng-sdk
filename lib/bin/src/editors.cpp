#pragma once

#include "../gmeng.h"
#include "../cli/index.h"
#include <map>
#include <memory>
#include <string>

namespace Gmeng::Editors {
    using string = std::string;
    class Editor {
      protected:
        struct editor_info_t {
            string name; string description; bool uses_sdl = false;
            Gmeng_Commandline::execution_scope scope = Gmeng_Commandline::gmng_user;
        };
      public:
        editor_info_t info;

        Editor(string name, string description) {
            this->info = { name, description };
            if (Gmeng::global.dev_mode) this->info.scope = Gmeng_Commandline::gmng_internal;
        };

        virtual ~Editor() = default;

        /// save to file
        virtual int save(string) = 0;
        /// load from file
        virtual int load(string) = 0;
        /// menu for the editor, file path etc
        virtual int menu() = 0;
    };

    std::map<string, std::unique_ptr<Editor>> map;

    template<typename T>
    static int init_editor(string name, std::unique_ptr<T> ptr) {
        map.insert_or_assign(name, std::make_unique( ptr ));
        return 0;
    };

    static int load_editors() {
        return 0;
    };

    static int __load_editors_trigger = load_editors();
};
