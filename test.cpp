#include <string>
#include <iostream>
#include <cstdlib>

#include "engine.h"

static int overall_tests = 0;
static int passed_tests = 0;
static int failed_tests = 0;

static void mark_passed() {
    passed_tests++;
    std::cout << "PASSED\n";
}

static void mark_failed(const std::string &error_message, const std::string &formula) {
    failed_tests++;
    std::cout << "FAILED\n";
    std::cout << "  " << error_message << std::endl;
    std::cout << "  '" << formula << "'\n";
}

static void success_test(std::string formula, double expected_result) {
    overall_tests++;
    double result = calculate(formula);
    if (result == expected_result) {
        mark_passed();
    } else {
        std::string error = "Wrong result: expected " + std::to_string(expected_result)
                + ", got " + std::to_string(result);
        mark_failed(error, formula);
    }
}

static void parse_error_test(std::string formula, int expected_error_position) {
    overall_tests++;
    try {
        calculate(formula);
        mark_failed("Missed parsing error", formula);
    } catch (const parse_exception &e) {
        if (expected_error_position == e.start_position) {
            mark_passed();
        } else {
            std::string error = "Bad parsing error position: expected " + std::to_string(expected_error_position)
                    + ", got " + std::to_string(e.start_position);
            mark_failed(error, formula);
        }
    }
}

static void run_tests() {
    success_test("5", 5);
    success_test("-5", -5);

    success_test("2 + 3", 5);
    success_test("2 - 3", -1);
    success_test("2 * 3", 6);
    success_test("5 / 2", 2.5);

    success_test("2 * 3 + 4", 10);
    success_test("2 + 3 * 4", 14);

    success_test("(7)", 7);
    success_test("(-2)", -2);
    success_test("(3 * 2)", 6);
    success_test("(3 + 2) * 2", 10);
    success_test("2 * (3 + 2)", 10);

    success_test("(((5)))", 5);
    success_test("((3 + 2) * (1 + 1))", 10);
    success_test("2 * (3 * ((3 + 1) + 1) + 2)", 34);
    success_test("2 * (3 + ((3 + 1) + 1) * 2)", 26);

    success_test("7 + (((5 * 2) + 5) / (2 + 3) + 1) / 2 - 1", 8);

    parse_error_test("", 0);
    parse_error_test("-", 0);
    parse_error_test("*", 0);
    parse_error_test("a", 0);
    parse_error_test("3a", 1);
    parse_error_test("3 + + 2", 4);
    parse_error_test("3 2", 2);
    parse_error_test("(-5)(4)", 4);
    parse_error_test("(5", 0);
    parse_error_test("5)", 1);
    parse_error_test(") * 5)", 0);
    parse_error_test("(5))", 3);
    parse_error_test("((5) - 1", 0);
    parse_error_test("(-)", 1);
    parse_error_test("(*)", 1);
    parse_error_test("()", 0);
    parse_error_test(std::string(500, '3'), 0);
    parse_error_test("3.3.3", 0);
}

int main() {
    run_tests();

    if (passed_tests > 0) {
        std::cout << passed_tests << " out of " << overall_tests << " test(s) passed\n";
    }
    if (failed_tests > 0) {
        std::cout << failed_tests << " out of " << overall_tests << " test(s) passed\n";
    }
    if (passed_tests + failed_tests != overall_tests) {
        std::cerr << "Tests are broken!\n";
        return 1;
    }
    return failed_tests == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
