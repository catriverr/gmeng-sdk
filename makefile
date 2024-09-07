# Compiler and flags
CXX := g++
CXXFLAGS := --std=c++2a -pthread -Wno-deprecated-declarations -Wno-writable-strings -Wno-switch-bool -Wno-format-security `pkg-config --libs --cflags libcurl`
OUTFILE := -o gmeng

UNAME_S := $(shell uname -s)


ifeq ($(UNAME_S), Darwin)
	CXXFLAGS += -framework ApplicationServices
endif

ifeq ($(UNAME_S), Linux)
	CXXFLAGS += -DGMENG_NO_CURSES -Wno-write-strings
else
	CXXFLAGS += `pkg-config --libs --cflags ncursesw`
endif

ifeq ($(filter debug,$(MAKECMDGOALS)), debug)
    CXXFLAGS += -fsanitize=address
	CXXFLAGS += -g
endif

ifeq ($(filter no-ncurses,$(MAKECMDGOALS)), no-ncurses)
    CXXFLAGS += -DGMENG_NO_CURSES
endif

ifeq ($(filter use-external,$(MAKECMDGOALS)), use-external)
	CXXFLAGS += `pkg-config --libs --cflags sdl2_ttf`
    CXXFLAGS += -DGMENG_SDL
endif


# Default target builds lib/bin/src/index.cpp
all: lib/bin/out/gmeng

# Rule to build lib/bin/src/index.cpp
lib/bin/out/gmeng: lib/bin/src/index.cpp
	$(CXX) $(OUTFILE) $(CXXFLAGS) lib/bin/src/index.cpp

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

# Phony targets
.PHONY: all test test2 debug no-ncurses
