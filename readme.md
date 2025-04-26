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
[`See what's new`](CHANGELOG.md) **(28-Apr-2025) info**: Linux compilation, `make setup` command | [`gmeng.org/changelog`](https://gmeng.org/changelog)

## Documentation
Gmeng's documentation can be found in [`gmeng.org`](https://gmeng.org). Please refer to the website for enquiries about functionality and usage.

## Building

This project uses the [**GNU Make**](https://www.gnu.org/software/make/) build system for an easy building process. Gmeng currently runs on all `UNIX` (macOS & linux) systems.
While a Windows port for Gmeng is being worked on it is not yet complete. If you're on a Windows machine,
install [`WSL (Windows Subsystem for Linux)`](https://learn.microsoft.com/en-us/windows/wsl/install) to run Gmeng without having to change computers or boot into a different Operating System.

### Getting Started

To get started, clone the repository and set it up with
```sh
git clone https://github.com/catriverr/gmeng-sdk
cd gmeng-sdk
sudo make setup
```
> [!IMPORTANT]
> You will need to run `sudo make setup`
> to include the sources for libraries & dependencies which gmeng uses.<br>
>
> **submodules**<br>
> Currently, gmeng uses the following submodules:
> - [`catriverr/liblmf`](https://github.com/catriverr/liblmf) for interleafing and configuration file parsing,
> - [`catriverr/noble`](https://github.com/catriverr/noble) for scripting, script parsing and plugin support
>
> **libraries & utilities**<br>
> - `homebrew` for installing & implementing libraries required by gmeng,
> - `pkg-config & pkgconf` for implementing cflags for C++ compiler libraries,
> - `libcurl4-openssl-dev` for easy telecommunications & networking functions,
> - `libncurses-dev` for easy terminal-based UI creation on CLI commands,
> - `libasound2-dev` **(linux only)** for the ALSA (Advanced Linux Sound Architecture) audio engine,
> - `ApplicationServices` **(macOS only)** for the ApplicationServices general Mac Service utilities

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
    make compile-script filename=<file.cc> (builds the file defined in filename, as a NOBLE shared library script)

    make setup (sets up the environment and installs the libraries required for gmeng to function)
    make build (builds the target in Production mode, all scripts in the scripts/src directory and your target will be compiled)
options:
    make [debug] [no-ncurses] [use-external] [warnings] [all/test/test2/compile/compile-windows/compile-file/compile-file-windows/compile-script/build]
    make configure
```
- The `debug` option adds the `-g -O0 -fsanitize=address` flags to the compiler.
- The `no-ncurses` option disables the auto-imports to `utils/interface.cpp` and `types/interface.h` from the `gmeng.h` header.
- The `use-external` option enables the auto-imports to `SDL2/SDL.h` headers for SDL-based windows.
- The `warnings` option enables `-Wall` so all warnings are displayed by the compiler.
- The `configure` option runs the configuration utility to set up the buildconfig for a program.

## Debugging
Gmeng is designed and configured to be debugged with the `llvm/lldb` debugger. To debug different functionalities of the library, run llvm with the unit tests binary.

### Internal Engine Debugging
Gmeng as its own testing utility for internal tests. Compile with:
```sh
make test2; lldb ./tests/out/test.o
```
to run different tests, add the `-l=0,1,2,3..8` flag to the `run` command in lldb.
refer to the [Unit Tests](tests/test.cpp) file to see different tests and what they do.

### External Application Debugging
You may also compile with debugging parameters in `clang` too with the `make debug (compile,build,etc..make parameters)` parameter.
```sh
make debug build; lldb ./game.out
```
This will allow you to compile with debugger parameters.

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
