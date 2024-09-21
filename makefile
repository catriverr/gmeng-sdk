# Compiler and flags
CXX := g++
CXXWARNINGS := -Wno-deprecated-declarations -Wno-writable-strings -Wno-switch-bool -Wno-format-security
CXXFLAGS := -Linclude -Iinclude --std=c++2a -pthread `pkg-config --libs --cflags libcurl`
OUTFILE := -o gmeng

USE_NCURSES := true
USE_EXTERNAL := false
TARGET_NAME := all
DEBUG_MODE := false

UNAME_S := $(shell uname -s)

# Check if any arguments were passed to make
ifneq ($(filter-out $@, $(MAKECMDGOALS)),)
    # If arguments were passed, skip the warning and inclusion
    skip_warning := true
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
	CXXFLAGS += -framework ApplicationServices
endif

ifeq ($(UNAME_S), Linux)
	CXXFLAGS += -DGMENG_NO_CURSES -Wno-write-strings
	USE_NCURSES := false
else
	CXXFLAGS += `pkg-config --libs --cflags ncursesw`
endif

ifeq ($(filter debug,$(MAKECMDGOALS)), debug)
    CXXFLAGS += -fsanitize=address
	CXXFLAGS += -g
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
	$(CXX) $(OUTFILE) lib/bin/src/index.cpp $(CXXFLAGS)

# Target for test, builds test.cpp
test: test.cpp
	$(CXX) -o test test.cpp $(CXXFLAGS)

# Target for test2, builds tests/test.cpp
test2: tests/test.cpp
	$(CXX) -o tests/out/test.o tests/test.cpp $(CXXFLAGS)

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
	CXXFLAGS += -g
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

# Phony targets
.PHONY: all test test2 debug no-ncurses warnings configure
