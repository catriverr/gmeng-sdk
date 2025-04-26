# Compiler and flags
CXX := g++
CXXWARNINGS := -Wno-deprecated-declarations -Wno-writable-strings -Wno-switch-bool -Wno-format-security
CXXFLAGS := -Linclude -Iinclude --std=c++2a -pthread `pkg-config --libs --cflags libcurl` -fpermissive
VERSIONFLAGS := -DGMENG_BUILD_NO="UNKNOWN"
OUTFILE := -o gmeng

# Compiler to Windows
WINDOWS_CXX := i686-w64-mingw32-g++ # MinGW compiler for unix-to-windows cross compile.
WINDOWS_CXXFLAGS := -std=c++20 -Wno-write-strings -Wno-return-type -static -static-libgcc -static-libstdc++ -lpthread -pthread -lwinmm

USE_NCURSES := true
USE_EXTERNAL := false
TARGET_NAME := all
DEBUG_MODE := false

UNAME_S := $(shell uname -s)

BUILD_NUMBER := $(shell printf G$$RANDOM-$$RANDOM)

ifeq ($(OS), Windows_NT)
	BUILD_NUMBER := $(shell echo | set /p version="G%random%-%random%")
	WINDOWS_CXX := g++
endif
ifeq ($(UNAME_S), Windows_NT)
	BUILD_NUMBER := $(shell echo | set /p version="G%random%-%random%")
	WINDOWS_CXX := g++
endif

$(info selected build number: $(BUILD_NUMBER))
VERSIONFLAGS := -DGMENG_BUILD_NO=\"$(BUILD_NUMBER)\"

# Check if any arguments were passed to make
ifneq ($(filter-out $@, $(MAKECMDGOALS)),)
    # If arguments were passed, skip the warning and inclusion
    skip_warning := true
endif

ifeq ($(filter compile, $(MAKECMDGOALS)),compile)
ifeq ($(wildcard buildoptions.mk),)
$(error run `make configure` to use `make compile`.)
else
include buildoptions.mk
$(info buildoptions selected.)
endif
endif

ifeq ($(filter compile-windows, $(MAKECMDGOALS)),compile-windows)
ifeq ($(wildcard buildoptions.mk),)
$(error create a file named `buildoptions.mk` and add the line `TARGET_NAME := your_game_code.cpp`.)
else
include buildoptions.mk
$(info buildoptions selected. (WINDOWS))
endif
endif

ifeq ($(filter build, $(MAKECMDGOALS)),build)
ifeq ($(wildcard buildoptions.mk),)
$(error run `make configure` to use `make build`.)
else
include buildoptions.mk
$(info buildoptions selected.)
endif
endif



# If no arguments were passed, check for buildoptions.mk
ifndef skip_warning
ifeq ($(wildcard buildoptions.mk),)
$(error run `make configure` first)
else
include buildoptions.mk
$(info buildoptions.mk file found, configuration:)
$(info ncurses: $(USE_NCURSES))
$(info use_sdl: $(USE_EXTERNAL))
$(info target_name: $(TARGET_NAME))
$(info debug_mode: $(DEBUG_MODE))
endif
endif

ifeq ($(filter test,$(MAKECMDGOALS)),test)
  ifneq ($(UNAME_S), Darwin)
    $(error sorry, you may not build the editor right now. switch to a Darwin/MacOS environment.)
  endif
endif


ifeq ($(UNAME_S), Darwin)
	CXXFLAGS += -framework ApplicationServices -framework AudioUnit -framework CoreAudio -framework AudioToolbox
endif

ifeq ($(UNAME_S), Linux)
	CXXFLAGS += -Wno-write-strings -lasound
	USE_NCURSES := false
endif

ifeq ($(filter debug,$(MAKECMDGOALS)), debug)
    CXXFLAGS += -fsanitize=address
	CXXFLAGS += -g -pg
endif


ifeq ($(filter warnings,$(MAKECMDGOALS)), warnings)
	CXXFLAGS += -Wall
else
	CXXFLAGS += $(CXXWARNINGS)
endif

ifeq ($(filter no-ncurses,$(MAKECMDGOALS)), no-ncurses)
    CXXFLAGS += -DGMENG_NO_CURSES
endif

ifeq ($(filter use-external,$(MAKECMDGOALS)), use-external)
	CXXFLAGS += `pkg-config --libs --cflags sdl2 sdl2_ttf`
    CXXFLAGS += -DGMENG_SDL
endif


# Default target builds lib/bin/src/index.cpp
all: lib/bin/out/gmeng

# Rule to build lib/bin/src/index.cpp
lib/bin/out/gmeng: lib/bin/src/index.cpp
	$(CXX) $(VERSIONFLAGS) $(OUTFILE) lib/bin/src/index.cpp $(CXXFLAGS)

# Target for test, builds test.cpp
test: tests/editor_test.cpp
	$(CXX) $(VERSIONFLAGS) -o test tests/editor_test.cpp $(CXXFLAGS)

# Target for test2, builds tests/test.cpp
test2: tests/test.cpp
	$(CXX) $(VERSIONFLAGS) -o tests/out/test.o tests/test.cpp $(CXXFLAGS)

# Target for building with the debug flag
debug:
	@$(MAKE) CXXFLAGS="$(CXXFLAGS)" $(filter-out debug,$(MAKECMDGOALS))

no-ncurses:
	@$(MAKE) CXXFLAGS="$(CXXFLAGS)" $(filter-out no-ncurses,$(MAKECMDGOALS))

use-external:
	@$(MAKE) CXXFLAGS="$(CXXFLAGS)" $(filter-out use-external,$(MAKECMDGOALS))

a = $(shell echo ALL WARNINGS ARE ENABLED, YOUR SHELL WILL BE FILLED WITH WARNINGS.)
warnings:
	$(info $(a))
	@$(MAKE) CXXFLAGS="$(CXXFLAGS)" $(filter-out warnings,$(MAKECMDGOALS))

# build requirement checkers

current_dir = $(shell pwd)
$(info compiling in $(current_dir))
include $(current_dir)/make/buildcheck.mk

ifeq ($(DEBUG_MODE),true)
	CXXFLAGS += -fsanitize=address
	CXXFLAGS += -g -pg
endif

ifeq ($(USE_EXTERNAL),true)
	CXXFLAGS += -DGMENG_SDL
	CXXFLAGS += `pkg-config --cflags --libs sdl2 sdl2_ttf`
endif

ifeq ($(USE_NCURSES),true)
	CXXFLAGS += `pkg-config --cflags --libs ncursesw`
endif

ifeq ($(filter configure,$(MAKECMDGOALS)),configure)
  ifeq ($(HAS_PKG_CONFIG),no)
  	$(warning Error: pkg-config is not installed. Exiting.)
	exit 1
  endif
endif

ifeq ($(filter configure,$(MAKECMDGOALS)),configure)
  ifeq ($(HAS_NCURSES),no)
    $(warning Error: ncurses is not installed. Exiting.)
    exit 1
  endif
endif

CXXFLAGS_MINIMAL := -Wno-all -std=c++2a -o config lib/bin/utils/configure.cc `pkg-config --cflags --libs ncursesw`

configure:
	@echo GMENG BUILD DEPENDENCIES:
	@echo 	- pkg-config\: $(HAS_PKG_CONFIG)
	@echo   - ncurses/ncursesw\: $(HAS_NCURSES)
	@echo build requirement check succeeded
	@echo all dependencies are installed
	@echo building configuration program...
	$(CXX) $(CXXFLAGS_MINIMAL)
	@./config

compile:
	@echo GMENG-ACCEPTED COMPILING FLAGS
	@echo COMPILING YOUR BUILDOPTIONS.MK
	@echo TO CONFIGURE, USE make configure
	@echo PROGRAM\: $(TARGET_NAME)
	@echo TARGET WILL BE NAMED\: ./game.out
	$(CXX) $(TARGET_NAME) $(VERSIONFLAGS) $(CXXFLAGS) -o game.out

compile-windows:
	@echo CROSS COMPILING TO WINDOWS
	@echo COMPILER\: $(WINDOWS_CXX)
	@echo FLAGS\: $(WINDOWS_CXXFLAGS)
	@echo
	@echo GMENG-ACCEPTED COMPILING FLAGS
	@echo COMPILING YOUR BUILDOPTIONS.MK
	@echo TO CONFIGURE, EDIT THE FILE buildoptions.mk TO CHANGE YOUR TARGET
	@echo EXECUTABLE WILL BE NAMED\: ./game.exe
	$(WINDOWS_CXX) $(TARGET_NAME) $(VERSIONFLAGS) $(WINDOWS_CXXFLAGS) -o game.exe


compile-file:
	@echo "Compiling file: $(filename)"
	$(CXX) $(filename) $(VERSIONFLAGS) $(CXXFLAGS) -o $(basename $(filename))

compile-file-windows:
	@echo "Cross compiling file: $(filename)"
	$(WINDOWS_CXX) $(filename) $(VERSIONFLAGS) $(WINDOWS_CXXFLAGS) -o $(basename $(filename))$(suffix .exe)

compile-script:
	@echo "Compiling NOBLE prebuilt shared library: $(filename)"
	$(CXX) $(filename) $(VERSIONFLAGS) $(CXXFLAGS) -DGMENG_COMPILING_SCRIPT -shared -fPIC -o $(basename $(filename))$(suffix .dylib)
	@echo "Written out to: $(basename $(filename))$(suffix .dylib)"

build:
	@echo "BUILDING YOUR GAME"
	@echo "..."
	@echo "target file: $(TARGET_NAME)"
	@echo "..."
	@echo "COMPILING SCRIPTS"
	sh ./scripts/sh/COMPILE_SCRIPTS.sh
	@echo "..."
	@echo "COMPILING ENGINE & SOURCE TARGET"
	@echo "the game executable will be written to: ./game.out"
	$(CXX) $(TARGET_NAME) $(VERSIONFLAGS) $(CXXFLAGS) -o game.out
	@echo "..."
	@echo "COMPILATION COMPLETE"

setup:
	@echo "setting up & installing binaries for gmeng..."
	@echo "(you may need root permissions, try sudo make setup if the installation fails)"
	@if [ "$(UNAME_S)" = "Linux" ]; then\
        echo "---------------------------------------------------------------------------------";\
		echo "GMENG: detected linux-based operating system";\
		echo "(if this is wrong, run the command make setup UNAME_S=<OS name>)";\
		echo "---------------------------------------------------------------------------------";\
		echo "[gmeng-setup] updating the apt database...";\
		apt-get update;\
		echo "------------------------------";\
		echo "[gmeng-setup]: installing pkgconf (pkg-config)...";\
		apt-get -y install pkgconf;\
		apt-get -y install pkg-config;\
		echo;\
		echo "[gmeng-setup] install libncurses-dev (ncurses, ncursesw)...";\
		apt-get -y install libncurses-dev;\
		echo;\
		echo "[gmeng-setup] installing libcurl4-openssl-dev...";\
		apt-get -y install libcurl4-openssl-dev;\
		echo;\
		echo "[gmeng-setup] installing libasound2 & libasound2-dev...";\
		apt-get -y install libasound2;\
		apt-get -y install libasound2-dev;\
		echo;\
	fi
	@if [ "$(UNAME_S)" = "Darwin" ]; then\
        echo "---------------------------------------------------------------------------------";\
		echo "GMENG: detected darwin-based (macOS) operating system";\
		echo "(if this is wrong, run the command make setup UNAME_S=<OS name>)";\
		echo "---------------------------------------------------------------------------------";\
		echo "[gmeng-setup] this setup utility uses homebrew, the setup will install it if you haven't already.";\
		sh ./scripts/sh/install_homebrew.sh;\
		echo "[gmeng-setup] installing pkgconf (pkg-config)";\
		brew install pkgconf;\
		brew install pkg-config;\
		echo;\
		echo "[gmeng-setup] installing libncurses-dev...";\
		brew install ncurses;\
		echo;\
		echo "[gmeng-setup] installing libcurl4-openssl-dev...";\
		brew install curl;\
		echo;\
		echo "[gmeng-setup] Apple/ApplicationServices (default installed with macOS).";\
		echo;\
	fi
	@echo "[gmeng-setup] installation process complete."
	@echo "[gmeng-setup] if there were setup process errors in the output, try running the command as root (sudo make setup)."
	@echo "------------------------------------------------------------------------------------------------------------------"
	@echo "[gmeng-setup] initializing git submodules..."
	git submodule update --init --recursive
	@echo "[gmeng-setup] setup process complete"

# Phony targets
.PHONY: setup build all compile-script test test2 debug no-ncurses warnings configure compile compile-windows compile-file compile-file-windows
