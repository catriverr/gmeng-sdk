<p align="center">
    <img src ="https://allahcorp.com/static/assets/gmeng-more-hd.png" style="width: 100%;"><br/>
    <p3>A Powerful, Terminal-Based 2D Game Engine written in C++.</p3>
</p>
<hr/>
<p align="center">
<img src="https://github.com/catriverr/gmeng-sdk/actions/workflows/c-cpp.yml/badge.svg">
</p>

## Building

This project uses the [**GNU Make**](https://www.gnu.org/software/make/) build system for an easy building process. Currently, Gmeng only runs on `Darwin (MacOS)` systems.

### Requirements
```cmake
Systems:
- gnu-make
- Node.JS 19.8.1 or later
- npm 9.2.0 or later
- g++ with stdc++20 or gnu++20
- homebrew

Frameworks:
- ncurses/ncursesw 6.5 or later
- pkg-config 0.29.2 or later
- libcurl 8.4.0 or later
- ApplicationServices
```

To get started, clone the repository with
```sh
git clone https://github.com/catriverr/gmeng-sdk
git submodule update --init --recursive
```
> [!WARNING]
> You will need to run `git submodule update --init --recursive`<br>
> to include the sources for libraries & dependencies which gmeng uses.<br>
> Currently, gmeng uses the following libraries:
> - [`catriverr/liblmf`](https://github.com/catriverr/liblmf) for script, interleaf and configuration parsing

you may use the following build options:
```make
builds:
    make all (builds engine and cli)
    make test (builds interface tests / test.cpp)
    make test2 (builds unit tests / tests/test.cpp)
options:
    make [debug] [no-ncurses] [use-external] [warnings] [all/test/test2]
    make configure
```
- The `debug` option adds the `-g -O0 -fsanitize=address` flags to the compiler.
- The `no-ncurses` option disables the auto-imports to `utils/interface.cpp` and `types/interface.h` from the `gmeng.h` header.
- The `use-external` option enables the auto-imports to `SDL2/SDL.h` headers for SDL-based windows.
- The `warnings` option enables `-Wall` so all warnings are displayed by the compiler.

- The `configure` option runs the configuration utility to set up the buildconfig for a program.

## Debugging
Gmeng is designed and configured to be debugged with the `llvm/lldb` debugger. To debug different functionalities of the library, run llvm with the unit tests binary.
```sh
make test2; lldb ./tests/out/test.o
```
to run different tests, add the `-l=0,1,2,3..8` flag to the `run` command in lldb.
refer to the [Unit Tests](tests/test.cpp) file to see different tests and what they do.

## Flags & Command-line Arguments
For **Gmeng**'s command-line arguments, refer to the `help` page in the command-line interface via:
```sh
make; ./gmeng -help
```

Gmeng takes the following preprocessor definitions:
```cpp
#define GMENG_NO_CURSES // false by default, for no ncurses imports, same as the make no-ncurses parameter
#define __GMENG_LOG_TO_COUT__ // false by default, streams gm_log() calls to the std::cout stream.
#define __GMENG_ALLOW_LOG__ // true by default, allows logging
#define __GMENG_DRAW_AFTER_LOG__ // false, by default draws the dev-c console
```

## Contact
you may contact me on  discord via `@catriverr`, or e-mail me at `nehir@mybutton.org`.
