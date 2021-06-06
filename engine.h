#ifndef CALCULATOR_ENGINE_H
#define CALCULATOR_ENGINE_H

#include <string>

class parse_exception : public std::exception {
public:
    const int start_position;
    const std::string message;

    parse_exception(int _start_position, const std::string &_message)
            : start_position(_start_position), message(_message) {}

    const char *what() const noexcept override {
        return message.c_str();
    }
};

double calculate(std::string formula);

#endif //CALCULATOR_ENGINE_H
