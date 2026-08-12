#pragma once

#include "global.h"
#include "assert.h"
#include "log.h"

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

namespace Gmeng {
    static std::map<std::string, std::string> func_annotations;
    static std::ofstream funclog("gmeng-functree.log");
    static bool functree_init = false;
    volatile static bool functree_enabled = true;
    volatile static bool functree_extensive = false;
    static std::vector<std::string> func_last(5000);
    static std::stringstream functree_calls;
};

// annotates a function, like information about a function.
static void _func_annot(const char* func, const char* info) {
    Gmeng::func_annotations.emplace(func, std::string(info));
};

#define __annot__(func, info)      _func_annot(vl_get_name(func), info)
#define __info__                   __annot__
#define __annotation__             __annot__

#define __functree_init__() if (!Gmeng::functree_init) Gmeng::funclog << "-- cleared previous log --\n~~GMENG_FUNCTREE~~\n*** This file is used for diagnostics ***\n", Gmeng::functree_init = true
/// Gmeng : Internal function tracking.
static void _functree_vl(char* file, int line, const char* func, const char* pretty_func) {
    if (!Gmeng::functree_enabled) return;
    if (!Gmeng::functree_init) __functree_init__();
    std::string func_annot = "";
    auto v = Gmeng::func_annotations.find(func);
    if (v != Gmeng::func_annotations.end()) func_annot = "\t\t/// " + v->second;
    std::string pretty_annot = Gmeng::functree_extensive ? std::string(" [ ") + pretty_func + " ]" : "";
    std::string dat = vl_filename(file) + std::string(":") + std::to_string(line) + " >> " + func + pretty_annot + func_annot;
    Gmeng::funclog << dat << std::endl;
    if (Gmeng::func_last.size() >= Gmeng::func_last.max_size()) Gmeng::func_last.clear();
    Gmeng::func_last.push_back(dat);
    Gmeng::functree_calls << dat << '\n';
};

#define __functree_call__(func) _functree_vl(__FILE__, __LINE__, vl_get_name(func), __PRETTY_FUNCTION__)

