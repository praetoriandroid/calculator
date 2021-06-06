#include <iostream>
#include <cstdlib>

#include "engine.h"

int main() {
    std::string formula;
    std::getline(std::cin, formula);

    try {
        double result = calculate(formula);
        std::cout << result << std::endl;
    } catch (const parse_exception &e) {
        std::cerr << "Invalid input:" << std::endl;
        std::cerr << '"' << formula << '"' << std::endl;
        std::cerr << std::string(e.start_position + 1, ' ') << '^' << std::endl;
        std::cerr << e.message << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
