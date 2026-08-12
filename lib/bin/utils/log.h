#pragma once
#include <string>
#include <vector>

#include "global.h"
#include "assert.h"

/// writes to a log file (&name) with content (&content).
/// `Gmeng::outfile` is streamed for this. To disable this,
/// set `GMENG_LOG` to `FALSE` with: `#define GMENG_LOG false`
/// before importing gmeng with `#include <gmeng/gmeng.h>`
static void __gmeng_write_log__(const std::string& name, const std::string& content, bool append = true) {
    Gmeng::outfile << content;
};

#define vl_get_name(x) #x
#define vl_filename(path) (strrchr(path, '/') ? strrchr(path, '/') + 1 : path)



/// Gmeng : Splits a string with a given delimeter.
/// If no occurences of the delimiter is found, the
/// returned vector will only have 1 entry that contains
/// the original string without any modifications.
static std::vector<std::string> g_splitStr(std::string s, std::string delimiter) {
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
        token = s.substr (pos_start, pos_end - pos_start);
        pos_start = pos_end + delim_len;
        res.push_back (token);
    }

    res.push_back (s.substr (pos_start));
    return res;
}

/// Returns a filename from a filepath.
static std::string get_filename(string filepath) {
    vector<string> fd = g_splitStr(filepath, "/");
    return fd[fd.size()-1];
};


// Gmeng's logging method.
// Many internal systems, however this function shouldn't be called directly.
// Use the gm_log() macro for automatic filename, code line and other useful
// log info to be parsed into your message.
static void _gm_log(const char* file_, int line, const char* func, std::string _msg, bool use_endl = true) {
    #if __GMENG_DISABLE_LOG__ == true
        return;
    #endif
    if ((IS_DISABLED GET_PREF("pref.log", func))
    && !Gmeng::global.dont_hold_back) {
        if (Gmeng::global.weird_ass) __gmeng_write_log__("gmeng.log", "GET_PREF(" + std::string(func) + ":pref.log) :: " + v_str( (int) GET_PREF("pref.log", func) ) + "\n");
        return;
    };

    std::string file = get_filename(std::string(file_)); // remove path, only use filename
    std::string msg = file + ":" + v_str(line) + " [" + std::string(func) + "] " + _msg;

        if (Gmeng::global.log_stout) std::cout << msg << std::endl;

        std::string _uthread = _uget_thread();
        std::string time_text = "(" + get_curtime() + ")";
        std::string __vl_log_message__ = time_text + " " + std::string(Gmeng::global.executable) + ":" + _uthread + " >> " + msg + (use_endl ? "\n" : "");




        Gmeng::logstream << __vl_log_message__;
        __gmeng_write_log__("gmeng.log", __vl_log_message__);

};

#define gm_log(x) _gm_log(__FILE__, __LINE__, __FUNCTION__, x)


#define GMENG_LOG
