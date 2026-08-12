#pragma once

#include "log.h"
#include "global.h"
#include "tracefunc.h"
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename T>
struct function_return_type;

template <typename R, typename... Args>
struct function_return_type<R(Args...)> {
    using type = R;
};

template <typename T>
using function_return_type_t = typename function_return_type<T>::type;


namespace Gmeng {
    template<typename T>
    class CommandSet {
      public:
        struct Command {
            std::string name;
            std::function<T> executor;
        };

        std::map<std::string, Command> commands;

        template<typename... Args>
        function_return_type_t<T> run( std::string command, Args&&... args) {
            return this->commands.at( command ).executor( std::forward<Args>( args )... );
        };

        CommandSet() = default;
        CommandSet( std::vector<std::tuple<std::string, std::function<T>>> _commands ) {
            for ( auto command : _commands ) {
                Command new_cmd{ std::get<0>(command), std::get<1>(command) };
                this->commands[ new_cmd.name ] = new_cmd;
            };
        };
    };
};
