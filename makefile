# Compiler and flags
CXX := g++
CXXFLAGS := -lcurl -g --std=c++20 -pthread `pkg-config --libs --cflags ncursesw` -Wno-deprecated-declarations -Wno-writable-strings -Wno-switch-bool -Wno-format-security -framework ApplicationServices
OUTFILE := -o gmeng
# Default target builds lib/bin/src/index.cpp
all: lib/bin/out/gmeng

# Rule to build lib/bin/src/index.cpp
lib/bin/out/gmeng: lib/bin/src/index.cpp
	$(CXX) $(CXXFLAGS) lib/bin/src/index.cpp $(OUTFILE)

# Target for test, builds test.cpp
test: test.cpp
	$(CXX) $(CXXFLAGS) -o test test.cpp

# Target for test2, builds tests/test.cpp
test2: tests/test.cpp
	$(CXX) $(CXXFLAGS) -o tests/out/test.o tests/test.cpp

# Phony targets
.PHONY: all test test2