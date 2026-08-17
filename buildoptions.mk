# whether gmeng will use ncurses in the final build. when disabled,
# `make` (so `make all`) may not compile. ncurses is not used by the
# engine implementation of gmeng (i.e. stuff rendered to the screen
# is done natively, not through ncurses). however, it's used for basi
# cli-based menus like the GLVL level editor and more.
USE_NCURSES := true
# whether gmeng will compile the program for SDL instead of the terminal.
# changing this option will add the `-DGMENG_SDL` option to the compiler,
# so the renderer will use the SDL-based renderer and the utilities for
# the terminal-based renderer will not be compiled at all. This will change
# behaviour AND remove functions since it's changing the default renderer.
#
# Also visit `.vim/coc-settings.json` for the clangd flags and add the flag
# `-DGMENG_SDL=true` as mentioned above. this will correctly set the clangd
# flags for coc-nvim. other text editors/IDEs will also have similar lsp
# settings, you can add this flag there as well if you do not use vim.
USE_EXTERNAL := false
# the path to your program. this is the file that will be compiled when the
# `make compile` command is ran. absolute paths are not required but may be
# better as g++ (at least on macos) embeds some compile-time file path information
# that may distort `gmeng.log` filepath strings.
TARGET_NAME := tests/event_loop.cpp
# debug mode. setting this option will link AddressSanitizer as well as the
# `-g` debug parameter to the compiler flags. when this is set, using a
# debugger like lldb or gdb you can debug your game code.
#
# Note that this will __DISABLE OPTIMIZATION COMPLETELY__. and on top of that,
# since ASAN is linked, there will be extra lag and slowness in your program.
# only set this option if you're actively debugging a problem in your code.
DEBUG_MODE := false

# vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
EXTRA_CXX_FLAGS := -Ilib/bin -Llib/bin
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# extra compiler flags to your program. useful for
# importing libraries. any arguments added to this
# variable will be added as a compiler flag.
#
# you may not need to add much here if you're not linking
# libraries or suppressing specific warnings. gmeng already
# adds a lot of common compiler flags by default. see the
# `makefile` file to check if they are required.
