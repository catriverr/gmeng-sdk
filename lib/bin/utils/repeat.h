#pragma once

#include <codecvt>
#include <functional>
#include <locale>
#include <string>
#include <vector>

/// Gmeng : Repeats a given string `times` number of times.
/// The string is referenced but is not modified,
/// the function returns the generated string.
static std::string repeatString(const std::string& str, int times) {
    //__functree_call__(repeatString);
    std::string result = "";
    for (int i = 0; i < times; i++) {
        result += str;
    }
    return result;
}

/// Gmeng : Repeats any `Thing` of any type `times` number of times.
template<typename Thing>
static std::vector<Thing> repeatThing(Thing obj, int times) {
    std::vector<Thing> Things;
    for (int i = 0; i < times; i++) Things.push_back(obj);
    return Things;
};

/// Gmeng : Splits a vector of any type with a user-provided `checker` function.
/// If the checker function returns true, the vector is split.
/// Returns a vector of a vector of things.
template<typename Thing>
static std::vector<std::vector<Thing>> splitThing(std::vector<Thing>& obj, std::function<bool(Thing)> checker) {
    std::vector<std::vector<Thing>> Things;
    int vec_c = 0;
    for (int i = 0; i < obj.size(); i++) {
        Thing lndx = obj[i];
        if (vec_c == Things.size()) Things.push_back(std::vector<Thing>{});
        if (!checker(lndx)) Things[vec_c].push_back(lndx);
        else vec_c++;
    };
    return Things;
};

static void repeat(int count, std::function<void()> func) {
    for (int i = 0; i < count; i++) func();
};

static void repeat(int count, std::function<void(int count)> func) {
    for (int i = 0; i < count; i++) func(i);
};

static wchar_t* repeat_wstring(wchar_t* wc, int times) {
    if (times <= 0) {
        return L"";
    };
    wchar_t* str = new wchar_t[times + 1];
    for (int i = 0; i < times; ++i) {
        str[i] = *wc;
    }
    str[times] = L'\0'; // Null-terminate the string

    return str;
};

/// Repeats a wide string `times` amount of times
static wchar_t* repeat_wstring(const wchar_t* wc, int times) {
    if (times <= 0) {
        return L"";
    };
    wchar_t* str = new wchar_t[times + 1];
    for (int i = 0; i < times; ++i) {
        str[i] = *wc;
    }
    str[times] = L'\0'; // Null-terminate the string

    return str;
};

/// converts a wide string to a normal string.
static std::string ws2s(const std::wstring& wstr) {
    using convert_typeX = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_typeX, wchar_t> converterX;
    return converterX.to_bytes(wstr);
};
