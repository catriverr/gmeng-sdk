<p align="center">
    <img src ="https://allahcorp.com/static/assets/gmeng-more-hd.png##" style="width: 100%;"><br/>
    <p3>A Powerful, Terminal-Based 2D Game Engine written in C++.</p3>
</p>
<hr/>
<p align="center">
<img src="https://github.com/catriverr/gmeng-sdk/actions/workflows/c-cpp.yml/badge.svg">
<img src="https://github.com/catriverr/liblmf/actions/workflows/c-cpp.yml/badge.svg">
<img src="https://img.shields.io/github/v/release/catriverr/gmeng-sdk?label=version&color=F68019">
</p>

## Changelog
[`See what's new`](CHANGELOG.md) **(31-Dec-2024) info**: 10.3.0: Audio Engine Implementation | [`gmeng.org/changelog`](https://gmeng.org/changelog)

## Documentation
Gmeng's documentation can be found in [`gmeng.org`](https://gmeng.org). Please refer to the website for enquiries about functionality and usage.

## Building

This project uses the [**GNU Make**](https://www.gnu.org/software/make/) build system for an easy building process. Currently, Gmeng only runs on `Darwin (MacOS)` systems.

### Requirements
```cmake
Systems:
- gnu-make
- g++ with stdc++20 or gnu++20

Frameworks:
- noble-lang (included)
- liblmf (included)
- pkg-config 0.29.2 or later

```
### Optional Utilities
```cmake
Systems:
- Node.JS 19.8.1 or later
- npm 9.2.0 or later
- homebrew

Frameworks:
- ncurses/ncursesw 6.5 or later
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
> - [`catriverr/liblmf`](https://github.com/catriverr/liblmf) for interleafing and configuration file parsing
> - [`catriverr/noble`](https://github.com/catriverr/noble) for scripting, script parsing and plugin support
### Build Options
```make
builds:
    make all (builds engine and cli)
    make test (builds interface tests / test.cpp)
    make test2 (builds unit tests / tests/test.cpp)
    make compile (builds your target file / specified in buildoptions.mk or `make configure`)
    make compile-windows (builds your target file / cross compiled to windows)
    make compile-file filename=<file.cpp> (builds the file defined in filename)
    make compile-file-windows filename=<file.cpp> (builds the file defined in filename, for windows target)
options:
    make [debug] [no-ncurses] [use-external] [warnings] [all/test/test2/compile/compile-windows]
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

## Contributing
Please refer to [`gmeng-sdk/CONTRIBUTING`](.github/CONTRIBUTING.md) for guidelines on how to contribute to gmeng.

## Contact
you may contact me on  discord via `@catriverr`, or e-mail me at `nehir@mybutton.org`.
