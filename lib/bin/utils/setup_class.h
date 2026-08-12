#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
#include <typeinfo>
#include <cxxabi.h>

// 1. Demangle helper
inline std::string get_demangled_name(const char* mangledName) {
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled) ? demangled : mangledName;
    free(demangled);
    return result;
}

//
inline std::vector<std::string> parse_variable_names(const std::string& namesList) {
    std::vector<std::string> names;
    size_t start = 0, end = 0;
    while ((end = namesList.find(',', start)) != std::string::npos) {
        std::string name = namesList.substr(start, end - start);
        size_t first = name.find_first_not_of(" \t\n\r");
        size_t last = name.find_last_not_of(" \t\n\r");
        if (first != std::string::npos) names.push_back(name.substr(first, last - first + 1));
        start = end + 1;
    }
    std::string name = namesList.substr(start);
    size_t first = name.find_first_not_of(" \t\n\r");
    size_t last = name.find_last_not_of(" \t\n\r");
    if (first != std::string::npos) names.push_back(name.substr(first, last - first + 1));
    return names;
}

/// (Gmeng 13.2.0): Gmeng Internal Type Initializer
/// `GMENG_INIT_TYPE( ... ) -> Class::gmeng_variables`
///
/// This macro sets up internal Gmeng types and structures with a
/// list of parameter names so they can be accessed externally
/// without implicit calls to the parameter. For example;
///
/// ```
/// class Gmeng::SomeClass {
///     int some_value = 42;
///     std::string some_string = "hello there";
///
///     GMENG_INIT_TYPE( some_value, some_string )
/// };
/// ```
///
/// With the above class definition, we have a static definition of all variables
/// within the class and their type names. This allows us to access parameter by
/// name in the variable map rather than implicit calls to the parameter.
///
/// `gmeng_variables` is an unordered_map that contains void* pointers to the
/// parameters within the class that were referenced in GMENG_INIT_TYPE( ... ).
/// So,
/// ```
/// Gmeng::SomeClass some_instance;
/// /// Access the some_string parameter without implicit call to the variable.
/// auto the_string = (std::string*)some_instance.gmeng_variables["some_string"];
/// /// If the value is not a nullptr we can assign to the parameter a new value.
/// if (the_string != nullptr) *the_string = "new value";
/// ```
/// in this case, `some_instance.some_string` will now carry `"new value"` since
/// the pointer in the `gmeng_variables` map references `&Gmeng::SomeClass::some_string`.
#define GMENG_INIT_TYPE(...) \
public: \
    /* gmeng_variables: map of variables' names to pointers to their values */ \
    std::unordered_map<std::string, void*> gmeng_variables; \
    \
    /* GMENG_INTERNAL_INIT_STATIC_TYPE_MAP */ \
    static const std::unordered_map<std::string, std::string>& get_static_type_map() { \
        static std::unordered_map<std::string, std::string> type_map; \
        return type_map; \
    } \
    \
    /* GMENG_INTERNAL_INIT_VARIABLE_LIST */ \
    template <typename... Args> \
    bool _gmeng_init_impl(const std::string& namesList, Args&... args) { \
        std::vector<std::string> names = parse_variable_names(namesList); \
        auto& sMap = const_cast<std::unordered_map<std::string, std::string>&>(get_static_type_map()); \
        bool initStatic = sMap.empty(); \
        size_t i = 0; \
        \
        /* very dark C++ variadic expansion magic */ \
        int dummy[] = { 0, ( \
            this->gmeng_variables[names[i]] = (void*)&args, \
            (initStatic ? (sMap[names[i]] = get_demangled_name(typeid(args).name()), 0) : 0), \
            ++i, 0 \
        )... }; \
        (void)dummy; /* suppress warning */ \
        \
        return true; \
    } \
    \
    /* Default member initializer: Evaluates when the object is created */ \
    bool _gmeng_initializer = _gmeng_init_impl(#__VA_ARGS__, __VA_ARGS__);

