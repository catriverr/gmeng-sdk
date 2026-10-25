#pragma once

#include <string>

namespace Gmeng {
    /// current version of the engine.
    /// "-d" suffix means the version is a developer version, high unstability level
    /// "-b" suffix means the version is a beta version, low unstability level but unpolished
    /// "-c" suffix means the version is a coroded version, low to medium unstability level but specific methods will not perform as expected
    static std::string version = "14.0.0";
};
