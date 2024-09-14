#pragma once
#include <string>

bool string_includes(const std::string& str, const std::string& substring) {
    return str.find(substring) != std::string::npos;
}
