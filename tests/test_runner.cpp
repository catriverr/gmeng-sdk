#include <chrono>
#include <iostream>
#include <string>
#include "../lib/bin/gmeng.h"

std::string prompt(std::string __p = "") {
    std::cout << colorformat(__p.empty() ? ">>" : __p);
    std::string input;
    std::getline(std::cin, input); // Read a line of input
    return input;
}

using namespace Gmeng;

extern "C" {
    void help() {
        INF("~Br~\x1F~h~~y~GMENG Test Running Utility~n~\n");
        SAY("~w~This utility uses ~y~SO (Shared Object)~n~ library files to run tests.\n");
        SAY("~w~This allows for users to recompile their program while it is running and reload the code from the library.\n");
        SAY("\nto create an ~y~SO~n~ file, use the flags '-fPIC -shared'\nwith your C++ code compiler, and set the output as '-o yourfile.so'");
        SAY("\nWhen writing test SO files, create a void run() method that calls its members.\n");
        SAY("The run method ~r~must NOT~n~ be any other return type. it has to be ~h~~y~void~n~.\n");
        SAY("the run() method will be called from the library in the same way the main() method is called.\n");
        SAY("the run() method ~r~can NOT have parameters~n~. it must be ~y~void ~g~main~w~() { ~h~\x1F~B1~/*...*/~n~~w~ };\n");
        SAY("TOM_TOM_FOOLERY\n");
    };
    void run() {
        while (true) {
            std::cout << "hellow, orld!" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        };
    };
};
