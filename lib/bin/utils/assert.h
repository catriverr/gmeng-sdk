#pragma once

#include "global.h"

#include <vector>
#include <string>
#include <map>

using std::vector;
using std::string;



// Custom ASSERT system for Gmeng,
// not like traditional ASSERT though.
namespace Gmeng::Assertions {
    typedef struct assert_t {
        enum jWRAP { ON = 0, OFF = 1, NOT_SET = 2 };
        std::map<string, jWRAP> headers;
        const char* bound;
    } vd_assert;

    typedef struct assert_data_t {
        string header;
        assert_t::jWRAP state;
        const char* bound;
    } assertable_t;

    static std::map<const char*, vd_assert> list;

    static vd_assert to_assert_t(assertable_t data) {
        vd_assert obj;
        obj.headers = std::map<string, vd_assert::jWRAP>();
        obj.bound = data.bound;
        obj.headers.emplace(data.header, data.state);
        return obj;
    };

    static void set_assert(assertable_t data) {
        if (!Assertions::list.contains(data.bound)) list.emplace(data.bound, to_assert_t(data));
        Assertions::list.find(data.bound)->second.headers.insert_or_assign(data.header, data.state);
    };

    static vd_assert::jWRAP get_assert(string header, const char* bound) {
        if ( Gmeng::global.ignore_assert ) return assert_t::ON;

        if (!Assertions::list.contains(bound)) return assert_t::NOT_SET;
        auto fd = Assertions::list.find(bound)->second.headers;
        return fd.contains(header) ? fd.find(header)->second : assert_t::NOT_SET;
    };
};

struct GMENG_NULL_T {
    void* content;
};

template<typename T = int>
struct not_nullptr_t {
    T val = 1;
};

static not_nullptr_t<int> not_nullptr_ref = { 1 };
static not_nullptr_t<int>* not_nullptr = &not_nullptr_ref;

/// runs a piece of code only if
/// Gmeng's debug mode is enabled
#define DEBUGGER if (Gmeng::global.debugger)
/// runs a piece of code only if
/// Gmeng's developer mode is enabled
#define DEVMODE if (Gmeng::global.dev_mode)

#define ASSERT(x,y) Gmeng::Assertions::set_assert(  \
            {                                       \
                .header = x,                        \
                .state = y,                         \
                .bound = __FUNCTION__               \
            }                                       \
        )

#define PREF(x) Gmeng::Assertions::get_assert(      \
            x,                                      \
            __FUNCTION__                            \
        )

#define GET_PREF(x, f) Gmeng::Assertions::get_assert( \
            x, f                                      \
        )

#define IS_SET Gmeng::Assertions::vd_assert::ON ==
#define IS_DISABLED Gmeng::Assertions::vd_assert::OFF ==
#define IS_UNKNOWN Gmeng::Assertions::vd_assert::NOT_SET ==

#define DISABLE() Gmeng::Assertions::vd_assert::OFF
#define ENABLE() Gmeng::Assertions::vd_assert::ON

#define p_no DISABLE()
#define p_yes ENABLE()
