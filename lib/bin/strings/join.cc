#pragma once
#include <string>
#include <vector>
#include <sstream>

std::string join_string(const std::vector<std::string>& vec, char delimiter) {
    std::ostringstream oss;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        if (it != vec.begin()) oss << delimiter;
        oss << *it;
    }
    return oss.str();
}