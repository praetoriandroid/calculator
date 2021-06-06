#include <string>
#include <vector>
#include <cmath>
#include <stdexcept>

#include "engine.h"

enum class token_type {
    number,
    opening_parenthesis,
    closing_parenthesis,
    multiply,
    divide,
    plus,
    minus
};

enum class _operator {
    multiply,
    divide,
    plus,
    minus,
};

class broken_parser_exception : std::runtime_error {
public:
    broken_parser_exception(const std::string &message): std::runtime_error("Broken parser: " + message) {}
};

class token {
    double _value;
public:
    const token_type type;
    const int start_position;

    token(token_type _type, int _start_position, double value = 0)
            : type(_type), start_position(_start_position), _value(value) {}

    // TODO: need something more elegant than the field and the method that are working only for one token type...
    double value() const {
        if (type != token_type::number) {
            throw broken_parser_exception("Unsupported operation");
        }

        return _value;
    }
};

enum class priority {
    lowest, // expression root
    first, // "+", "-"
    second // "*", "/"
};

class expression {
public:
    virtual ~expression() = default;

    virtual double calc() const = 0;

    virtual int consumed_tokens() const = 0;
};

class two_operand_expression : public expression {
protected:
    const expression *left;
    const expression *right;

public:
    two_operand_expression(const expression *_left, const expression *_right) : left(_left), right(_right) {}

    virtual ~two_operand_expression() {
        delete left;
        delete right;
    }

    int consumed_tokens() const override {
        return left->consumed_tokens() + right->consumed_tokens() + 1;
    }
};

class add : public two_operand_expression {
public:
    add(const expression *_left, const expression *_right) : two_operand_expression(_left, _right) {}

    double calc() const override {
        return left->calc() + right->calc();
    }
};

class subtract : public two_operand_expression {
public:
    subtract(const expression *_left, const expression *_right) : two_operand_expression(_left, _right) {}

    double calc() const override {
        return left->calc() - right->calc();
    }
};

class multiply : public two_operand_expression {
public:
    multiply(const expression *_left, const expression *_right) : two_operand_expression(_left, _right) {}

    double calc() const override {
        return left->calc() * right->calc();
    }
};

class divide : public two_operand_expression {
public:
    divide(const expression *_left, const expression *_right) : two_operand_expression(_left, _right) {}

    double calc() const override {
        return left->calc() / right->calc();
    }
};

class parentheses : public expression {
    const expression *content;

public:
    parentheses(const expression *e) : content(e) {}

    ~parentheses() {
        delete content;
    }

    double calc() const override {
        return content->calc();
    }

    int consumed_tokens() const override {
        return content->consumed_tokens() + 2;
    }
};

class number : public expression {
    double value;

public:
    number(double _value) : value(_value) {}

    double calc() const override {
        return value;
    }

    int consumed_tokens() const override {
        return 1;
    }
};

class negative : public expression {
    const expression *content;

public:
    negative(const expression *_content): content(_content) {}

    ~negative() {
        delete content;
    }

    double calc() const override {
        return -1 * content->calc();
    }

    int consumed_tokens() const override {
        return content->consumed_tokens() + 1;
    }
};

class parser {
    const std::string formula;
    std::vector <token> tokens;

public:
    parser(const std::string &_formula) : formula(_formula) {}

    expression *parse() {
        tokens = tokenize();
        if (tokens.empty()) {
            throw parse_exception(0, "Empty input");
        }
        return parse_range(0, tokens.size() - 1, priority::lowest);
    }

private:
    expression *parse_range(int start, int end, priority parent_priority) {
        const int available_tokens = end - start + 1;

        expression *result = parse_operand(start, end);
        int consumed_tokens = result->consumed_tokens();

        while (available_tokens > consumed_tokens) {
            int next_operator_index = start + result->consumed_tokens();
            _operator next_operator = parse_operator(tokens[next_operator_index]);
            consumed_tokens++;
            priority operator_priority = get_priority(next_operator);

            if (operator_priority <= parent_priority) {
                return result;
            }

            expression *next_operand = parse_range(start + consumed_tokens, end, operator_priority);
            result = create_two_operand_expression(result, next_operator, next_operand);
            consumed_tokens = result->consumed_tokens();
        }

        return result;
    }

    priority get_priority(_operator _operator) {
        switch (_operator) {
            case _operator::minus:
            case _operator::plus:
                return priority::first;
            case _operator::multiply:
            case _operator::divide:
                return priority::second;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
            default:
                throw broken_parser_exception("Unknown operator");
#pragma clang diagnostic pop
        }
    }

    _operator parse_operator(const token &token) {
        switch (token.type) {
            case token_type::plus:
                return _operator::plus;
            case token_type::minus:
                return _operator::minus;
            case token_type::multiply:
                return _operator::multiply;
            case token_type::divide:
                return _operator::divide;
            default:
                throw parse_exception(token.start_position, "Unexpected token: operator needed");
        }
    }

    // either number, or parenthesis, or minus number/parenthesis
    expression *parse_operand(int start, int end) {
        const token &first_token = tokens[start];
        switch (first_token.type) {
            case token_type::opening_parenthesis:
                return parse_parentheses(start, end);

            case token_type::minus:
                if (start == end) {
                    throw parse_exception(first_token.start_position, "Orphan minus");
                }
                return new negative(parse_operand(start + 1, end));

            case token_type::number:
                return new number(first_token.value());

            default:
                throw parse_exception(first_token.start_position, "Unexpected token");
        }
    }

    expression *parse_parentheses(int start, int end) {
        int closing_parenthesis_index = find_closing_parenthesis(start + 1, end);
        if (closing_parenthesis_index == -1) {
            throw parse_exception(tokens[start].start_position, "Unclosed parenthesis");
        }
        if (closing_parenthesis_index == start + 1) {
            throw parse_exception(tokens[start].start_position, "Empty parentheses");
        }
        expression *content = parse_range(start + 1, closing_parenthesis_index - 1, priority::lowest);
        return new parentheses(content);
    }

    int find_closing_parenthesis(int start, int end) {
        int depth = 0;
        for (int i = start; i <= end; i++) {
            if (tokens[i].type == token_type::closing_parenthesis) {
                if (depth == 0) {
                    return i;
                } else if (depth > 0) {
                    depth--;
                } else {
                    throw broken_parser_exception("Missed closing parenthesis");
                }
            } else if (tokens[i].type == token_type::opening_parenthesis) {
                depth++;
            }
        }
        return -1;
    }

    expression *create_two_operand_expression(expression *left, _operator _operator, expression *right) {
        switch (_operator) {
            case _operator::multiply:
                return new multiply(left, right);
            case _operator::divide:
                return new divide(left, right);
            case _operator::plus:
                return new add(left, right);
            case _operator::minus:
                return new subtract(left, right);
#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnreachableCode"
            default:
                throw broken_parser_exception("Unknown operator");
#pragma clang diagnostic pop
        }
    }

    double parse_number(const std::string &number, int number_start) {
        char *end;
        try {
            double value = std::strtod(number.c_str(), &end);
            if (end - number.c_str() != number.size() || !std::isnormal(value)) {
                throw parse_exception(number_start, "Number too long");
            }

            return value;
        } catch (const std::logic_error &e) {
            throw parse_exception(number_start, std::string("Invalid number: ") + e.what());
        }
    }

    std::vector <token> tokenize() {
        std::vector <token> tokens;
        std::string number_token = "";
        int number_start = 0;
        for (int i = 0; i < formula.size(); i++) {
            char symbol = formula[i];

            if (symbol >= '0' && symbol <= '9' || symbol == '.') {
                if (number_token.empty()) {
                    number_start = i;
                }

                number_token += symbol;

                if (i == formula.size() - 1) {
                    double value = parse_number(number_token, number_start);
                    tokens.push_back(token(token_type::number, number_start, value));
                }

                continue;
            } else if (!number_token.empty()) {
                double value = parse_number(number_token, number_start);
                tokens.push_back(token(token_type::number, number_start, value));
                number_token = "";
            }

            switch (symbol) {
                case ' ':
                    // everything is already done above
                    break;

                case '(':
                    tokens.push_back(token(token_type::opening_parenthesis, i));
                    break;

                case ')':
                    tokens.push_back(token(token_type::closing_parenthesis, i));
                    break;

                case '+':
                    tokens.push_back(token(token_type::plus, i));
                    break;

                case '-':
                    tokens.push_back(token(token_type::minus, i));
                    break;

                case '*':
                    tokens.push_back(token(token_type::multiply, i));
                    break;

                case '/':
                    tokens.push_back(token(token_type::divide, i));
                    break;

                default:
                    throw parse_exception(i, "Unexpected symbol");
            }
        }
        return tokens;
    }
};

double calculate(std::string formula) {
    parser parser(formula);
    expression *expression = parser.parse();
    double result = expression->calc();
    delete expression;
    return result;
}
