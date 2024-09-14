#pragma once
#include <string>

bool contains(const std::string& str, const std::string& substring) {
    return str.find(substring) != std::string::npos;
}