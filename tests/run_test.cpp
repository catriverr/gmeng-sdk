#include <atomic>
#include <condition_variable>
#include <csignal>
#include <future>
#include <iostream>
#include <dlfcn.h>
#include "../lib/bin/gmeng.h"
#include <mutex>
#include <string>
#include <system_error>
#include <thread>
#include "./test_runner.cpp"

using namespace Gmeng;

static std::string lib_name;
static std::string self = "./testrunner.so";

template<typename Func>
bool from_library(const std::string& func_name) {
    if (lib_name.empty()) std::cout << colorformat("cannot call function " + func_name + " from library, lib_name is NULL.") << std::endl;
    void* handle = dlopen(lib_name.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        return false;
    };
    auto loaded_func = reinterpret_cast<Func*>(dlsym(handle, func_name.c_str()));
    if (!loaded_func) {
        dlclose(handle);
        return false;
    }
    (*loaded_func)();
    dlclose(handle);
    return true;
}

template<typename Func>
bool from_self(const std::string& func_name) {
    void* handle = dlopen(self.c_str(), RTLD_NOW | RTLD_GLOBAL);
    if (!handle) {
        return false;
    };
    auto loaded_func = reinterpret_cast<Func*>(dlsym(handle, func_name.c_str()));
    if (!loaded_func) {
        dlclose(handle);
        return false;
    }
    (*loaded_func)();
    dlclose(handle);
    return true;
}

std::mutex mtx;
std::condition_variable cv;
std::atomic<bool> stop_flag {false};


std::function<void()> do_async_task(std::function<void()> asyncFunction) {
    std::promise<void> exitSignal;
    std::future<void> exitFuture = exitSignal.get_future();

    // Start the async task
    std::thread thread = std::thread([&]() {
        asyncFunction();
    });
    std::thread thread2 = std::thread([&]() {
        while (!stop_flag.load());
        pthread_t t_native = static_cast<pthread_t>(thread.native_handle());
        pthread_cancel(t_native);
    });
    thread.detach();
    thread2.detach();
    // Return a function to stop the execution
    return [&exitSignal]() {
        std::unique_lock<std::mutex> lock(mtx);
        stop_flag.store(true);
        std::cout << "just stop.\n";
        exitSignal.set_value(); // Signal the task to exit
    };
};

volatile sig_atomic_t running_test_file = 0;
volatile sig_atomic_t next_dont_stop = 0;

void sigint_handle(int) {
    if (next_dont_stop == 0) { next_dont_stop = 1; stop_flag.store(true); SAY("aborting " + v_str(next_dont_stop)); } else exit(0);
    auto _________________should_just_be_null_but_whatever = std::async(std::launch::async, [&]() {
        std::this_thread::sleep_for(ms(3000));
        next_dont_stop = 0;
        stop_flag.store(false);
    });
};

int main(int argc, char** argv) {
    INF("~Br~\x1F~h~~y~GMENG Test Running Utility~n~\n");
    INF("~h~~p~'help'~w~~n~ for help\n");
    if (argc > 1 && (!file_exists(argv[1]) || !endsWith(std::string(argv[1]), ".so"))) INF("~Br~ERROR~n~ ~h~~r~(e_unknwn_lib)~o~ Could not find SO Library ~y~'" + std::string(argv[1]) + "'~n~\n"), 0;
    if (argc > 1 && ( file_exists(argv[1]) ||  endsWith(std::string(argv[1]), ".so"))) SAY("~Bb~INFO~n~ Library set to ~y~'" + std::string(argv[1]) +"'~n~\n"), lib_name = std::string(argv[1]);
    while (lib_name.empty()) {
        std::string input = prompt("~h~~c~library name (must end with *.so)>>");
        if (input == "help") { from_self<void()>("help"); continue; };
        if (!file_exists(input) || !endsWith(input, ".so")) { INF("~Br~ERROR~n~ invalid SO file name ~y~'" + input + "'~n~\n"); continue; };
        lib_name = input;
    };
    std::signal(SIGINT, sigint_handle);
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        if (stop_flag.load()) break;
            /// todo: switch to kill-child version
            /// 
        if (!running_test_file) {
            running_test_file = true;
            do_async_task([&]() {
                std::unique_lock<std::mutex> lock(mtx);
                do_async_task([&]() {
                    while(!stop_flag.load());
                    dlclose("");
                });
                bool lib = from_library<void()>("run");
                if (!lib) std::cout << colorformat("~Br~ERROR~n~ ~y~could not find entry-point void-return-type run() function in library.");
                if (next_dont_stop == 0) SAY("press any key to re-run test, ctrl+c to exit"), std::cin.get();                
            });
        };
    };
};

