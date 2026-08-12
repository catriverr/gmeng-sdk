#pragma once

// place TRACEFUNC at the end of your function parameters
// to get information about the caller of the current function.
// creates the following variables for the scope this util was
// used in:
//
// - func_caller for the name of the calling function,
// - func_caller_file for the name of the calling file,
// - func_caller_line for the name of the calling file line.
//
// Must be placed as a function parameter, will not function
// otherwise.
//
// Usage:
// void func1(int otherparam, int someotherparam, int optionalparam = 0, TRACEFUNC) {
//    std::cout << func_caller << " from file " << func_caller_file << ":" << func_caller_line << " called func1().\n";
// };
//
// void caller() {
//    func2(0, 1);
// }
//
// can also use TRACEFUNC_STR like:
//
//void func1(TRACEFUNC) {
//    std::cout << TRACEFUNC_STR << '\n';
// };
//
// void caller() {
//    func2();
// }
//
#define TRACEFUNC const char* func_caller       = __builtin_FUNCTION(), \
                  const char* func_caller_file  = __builtin_FILE(),     \
                  int         func_caller_line  = __builtin_LINE()

// place TRACEFUNC_STR inside a function that uses a
// TRACEFUNC parameter. Will not work otherwise.
//
// Usage:
//
// void func_with_tracefunc(TRACEFUNC) {
//     std::string func_trace_string = TRACEFUNC_STR;
// }
#define TRACEFUNC_STR std::string(func_caller_file) + ":" + std::to_string(func_caller_line) + " -> " + std::string(func_caller)
