# Gmeng Engine Walkthrough
This header is a basic walkthrough of getting started with Gmeng assuming that the reader is familiar with the `README.md`.

The Gmeng game engine works as a namespace and static utility commands and compiled into a shared object `gmeng_engine.o`.

The user of the engine does not import the source code of the engine. Instead, Gmeng has a separate header that declares the definitions for classes, functions and namespaces.

While this does mean that we have to declare functions, structs and objects twice, it's not a problem: the source containing the implementations of any classes, functions, global variables and vice versa do not require user-exposed comments. Only for source code, so it's not that big of a hassle.

Gmeng maintains only the parts of the engine that a user of the engine will require. For example, a crucial function like  `gmeng_check_keyboard()` will not be exposed through the header, as it is only used internally and there isn't a probable case of the user requiring this function.

# How to Use the Engine Header
This walkthrough assumes that you have read the [`README.md`](README.md), which contains the steps for setting up a development environment with cloning the engine with git and running `make setup & make configure`.


## 1. Main source code for a project

The main source file, similar to `main.cpp / index.cpp` should import `lib/gmeng.h` to access the engine's exported symbols.

>[!WARNING]
> Make sure to import `lib/gmeng.h` and not `lib/bin/gmeng.h`: **they are seperate files!**


## 2. Compiling

There are multiple different ways to compile a gmeng-based project. However, non-power users and even most power users will be using only two compilation commands: `make compile-engine` and `make with-engine filename=<...>`.

1. Compile the engine source: `make compile-engine`, _this is required only once_. It will compile the version of the Gmeng game engine once and create a binary named `gmeng_engine.o`.

2. Compile your project: `make with-engine filename=<your_code.cpp>`, this is the make command to recompile your game every time you make changes in your source code. This allows you to compile only the parts of the final executable that changes. The engine source never changes, no need to compile it repeatedly at each compilation.


## 3. When to Recompile

This goes for all C++ code written in general, but the less times you need to recompile your program for configuration changes and other such minor adjustments, the better.

This is why Gmeng supports a fully-implemented version of the engine source via `sol2`. The Lua library is able to execute functions directly from the C++ Gmeng namespace as well as some static commands _that in fact may not be accessible from the header-only declaration of Gmeng!_ In some ways, the C++ to Lua binding is more powerful than the pure C++ code that you will be writing.

You don't need to define Entity movement routines in pure C++. Use Lua instead. See the `GMENG ENTITY MANAGER` in the `gmeng development console` option available for SDL2 and kitty graphics protocol based games (recommended) to apply subroutines or link files containing code for entity and eventloop routines.


# Example Program using the latest Standard version of Gmeng

```cpp
#include <gmeng/lib/bin/easy.h>

int my_game_loop();

int main(int argc, char** argv) {
    patch_argv_global(argc, argv);
    vgm_load("envs/models");

    return my_game_loop();
};

int my_game_loop() {
    using namespace Gmeng;

    static Gmeng::Level level;
    static Gmeng::EventLoop ev;

    /// read a level from a file. see the level
    /// editor in `make editor` to create and edit
    /// level files.
    read_level_data("some_level.glvl", level);

    ev.level = &level;

    ev.add_hook({
        UPDATE
    }, [](Level* lvl, EventInfo* info) {
        lvl->display.refresh(lvl);
    });

    ev.add_hook({
        FIXED_UPDATE
    }, [&ev](Level* lvl, EventInfo* info) {
        /// if 'q' is pressed, quit the game.
        if ( getkey(gk_Q) ) ev->cancel();
    });

    do_event_loop(&ev);

    return 0;
};
```
