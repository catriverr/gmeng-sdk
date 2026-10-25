#pragma once
#include <string>

std::string trim(const std::string& str, bool last = true) {
    const auto start = str.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos) return ""; // no content
    const auto end = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(start, last ? end - start + 1 : std::string::npos);
}
