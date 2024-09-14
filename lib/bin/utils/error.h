#pragma once
#include <exception>
#include <string>

class script_syntax_exception : public std::exception {
  private:
    std::string message;
  public:
    explicit script_syntax_exception(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

class script_type_exception : public std::exception {
  private:
    std::string message;
  public:
    explicit script_type_exception(const std::string& msg) : message(msg) {}

    const char* what() const noexcept override {
        return message.c_str();
    }
};

