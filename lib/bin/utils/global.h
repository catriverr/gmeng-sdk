#pragma once
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <chrono>
#include <random>
#include <vector>
#include <array>
#include <thread>

#include "setup_class.h"

/// in-text number variable input like `"hi user"$(id)""`
#define $(x) + v_str(x) +


#ifndef __GMENG_LOG_TO_COUT__

#if GMENG_SDL
    #define __GMENG_LOG_TO_COUT__ true
#else
    #define __GMENG_LOG_TO_COUT__ false
#endif

#endif

#define time_rn std::chrono::system_clock::now().time_since_epoch()
#define GET_TIME() ( std::chrono::duration_cast<std::chrono::milliseconds>(time_rn).count() )

/// Gmeng : Returns a 7-digit random number.
/// Based on the mersenne twister 19937 generator.
static int g_mkid() {
    //__functree_call__(g_mkid);
    std::random_device rd; // random device to seed the generator
    std::mt19937 gen(rd()); // mersenne twister 19937 generator
    std::uniform_int_distribution<int> distribution(1000000, 9999999); // 7-digit range

    return distribution(gen);
};

/// Gmeng : Returns the current time (in HH:MM:SS)
static std::string get_curtime() {
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);

    // Format the time as hour:minute:second
    std::ostringstream time_stream;
    time_stream << std::setw(2) << std::setfill('0') << local_time->tm_hour << ":"
                << std::setw(2) << std::setfill('0') << local_time->tm_min << ":"
                << std::setw(2) << std::setfill('0') << local_time->tm_sec;

    return time_stream.str();
};

/// Gmeng : Returns the current date in (MM.DD).
static std::string get_curdate() {
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm* local_time = std::localtime(&now);

    // Format the date as month:day
    std::ostringstream date_stream;
    date_stream << std::setw(2) << std::setfill('0') << (local_time->tm_mon + 1) << "."
                << std::setw(2) << std::setfill('0') << local_time->tm_mday;

    return date_stream.str();
};

#ifndef GMENG_BUILD_NO
    #define GMENG_BUILD_NO "(UNKNOWN_BUILD)"
#endif

#define v_str std::to_string

/// Returns the thread ID of the current thread as a string.
inline std::string _uget_thread() {
    /// __functree_call__(_uget_thread);
    static std::atomic<int> counter(0);
    thread_local int threadId = counter.fetch_add(1);
    return std::to_string(threadId);
}

/// Main Gmeng namespace
namespace Gmeng {
    /// Gmeng Log outfile. This is opened by gm_log()
    /// to stream all Logs to the file. Alternatively,
    /// set `GMENG_LOG` to `false` to stop this with:
    /// `#define GMENG_LOG false`.
    static std::ofstream outfile;
    /// Logstream. Copy of the `Gmeng::outfile`.
    /// Both game logs and engine logs are available
    /// in this stringstream instance.
    static std::stringstream logstream;

    struct __global_object__ {
        std::vector<int> indexes;
        std::vector<std::string> containers;

        bool dev_console; bool debugger;
        bool log_stout; bool dev_mode;
        bool dont_hold_back; bool shush;
        bool weird_ass; bool restarted_instance;
        bool ignore_assert;


        std::string raw_arguments;

        std::string executable;
        std::string raw_executable_name;

        std::string user;
        std::string pwd;

        int prog_argc;
        char** prog_argv;

        bool window_control;

        GMENG_INIT_TYPE( indexes, containers, dev_console, debugger,
                         log_stout, dev_mode, dont_hold_back, shush,
                         weird_ass, restarted_instance, ignore_assert,
                         raw_arguments, executable, raw_executable_name,
                         user, pwd, prog_argc, prog_argv, window_control )

    };
    /// GMENG's global variables
    static __global_object__ global = {
        .dev_console = true, .debugger = false,
        .log_stout = __GMENG_LOG_TO_COUT__, .dev_mode = false,
        .dont_hold_back = false, .shush = false,
        .weird_ass = false, .window_control = false
    };
};

/// Joins a string vector with a given delimeter.
static std::string g_joinStr(std::vector<std::string> v, std::string delimiter) {
	std::string result;
    /// don't add the delimeter if there is only one item.
    if ( v.size() == 1 ) return v.at(0);

	for ( auto i : v) { result += i + delimiter; };
	return result;
};


#define GMENG_GLOBAL
