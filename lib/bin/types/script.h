#pragma once
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <string>
#include <exception>

#include "../gmeng.h"

using std::map, std::vector, std::string;

namespace Gmeng {
    namespace Scripts {

        struct return_type {
            map<string, string> values;
        };

        struct base_class {
            string name;
            string hashed_id;
        };

        struct object_data {
            string raw;
            base_class type;
        };

        struct function_handle {
            bool success;
            return_type rvalue;
            function_handle* parent;
        };

        struct function_parameter {
            string name;
            object_data value;
        };

        using function_return = Scripts::function_handle(function_parameter ...);

        struct hashed_function {
            vector<function_parameter> parameters;
            string hashed_id = v_str(g_mkid());
            hashed_function* parent = nullptr;
            string name;

            static string get_full_name(hashed_function& va) {
                string final = "";
                if (va.parent != nullptr) final += hashed_function::get_full_name(*va.parent) + "::";
                final += va.name; return final;
            };
        };

        struct function {
            vector<function_parameter> parameters;
            function* parent = nullptr;
            string name;

            static hashed_function hash(function& a) {
                hashed_function final;
                function* parent = a.parent;
                if (parent != nullptr) {
                    hashed_function parent_d = function::hash(*parent);
                    final.parent = &parent_d;
                };
                final.name = a.name;
                final.parameters = a.parameters;
                return final;
            };
        };


        class object_class {
          public:
            string name; string hash = v_str(g_mkid());
            map<string, hashed_function> methods;
            map<string, base_class> variables;

            void add_or_assign_function(function func) {
                hashed_function ifunc = function::hash(func);
                this->methods.insert_or_assign(func.name, ifunc);
            };

        };

        struct object {
            base_class type;
            object_class* content;
        };


    };
}


#include "../utils/script.cpp"

#define GMENG_SCRIPT_INIT
