/* date = September 02, 2025 5:53 PM */


#include "Lexer.h"

#include <iostream>

#include "Parser.h"

namespace DS::Expr {
    void Lexer::lex(const std::string &str, std::vector<Token> &tokens) {
        while (pos != str.length()) {
            const auto curr = str[pos];
            switch (curr) {
                case '(': {
                    tokens.push_back(Token(OpenParens));
                    consume(1);
                    break;
                }
                case ')': {
                    tokens.push_back(Token(CloseParens));
                    consume(1);
                    break;
                }
                case '+': {
                    tokens.push_back(Token(Add));
                    consume(1);
                    break;
                }
                case '-': {
                    if (is_numeric(str[mark])) {
                        parse_literal(str, tokens);
                    } else {
                        auto [ty, _] = tokens.back();
                        switch (ty) {
                            case Expression:
                            case Literal:
                            case CloseParens:
                                tokens.push_back(Token(Subtract));
                                break;
                            case Multiply:
                            case Divide:
                            case Add:
                            case Subtract:
                            case OpenParens:
                                tokens.push_back(Token(UnaryMinus));
                                break;
                            default:
                                std::cout << "Parse Error: Consecutive unary minuses.\n";
                                exit(-1);
                        }
                        consume(1);
                    }
                    break;
                }
                case '*': {
                    tokens.push_back(Token(Multiply));
                    consume(1);
                    break;
                }
                case '/': {
                    tokens.push_back(Token(Divide));
                    consume(1);
                    break;
                }
                case ' ':
                case '\r':
                case '\n':
                case '\t': {
                    consume(1);
                    break;
                }
                case '0' ... '9': {
                    parse_literal(str, tokens);
                    break;
                }
                case 'a' ... 'z':
                case 'A' ... 'Z':
                case '.':
                case '_': {
                    // consume chars until non-expression char found
                    while (mark != str.length() && is_expr_char(str[mark])) {
                        mark++;
                    }
                    // get substring containing those characters
                    const size_t len = mark - pos;
                    const std::string data = str.substr(pos, len);
                    tokens.push_back(Token(Expression, data));
                    consume(len);
                    break;
                }
                default: {
                    std::cerr << "Lexer Error: Invalid character '" << str[pos] << "' (hex code " << static_cast<int>(
                        str[pos]) << ")" << std::endl;
                    exit(-1);
                };
            }
        }
    }

    void test_lexer() {
        // TODO: we need function application
        Lexer lexer{};
        std::vector<Token> tokens{};
        const std::string to_parse = "((mta.current + 2 * mtb.current) / (2.0 * -some_const))";

        lexer.lex(to_parse, tokens);

        for (const Token &t: tokens) {
            t.print();
        }

        Parser parser{};
        auto foo = parser.parse(tokens);

        foo->apply("mta.current", 30.0);
        foo->apply("mtb.current", 3.0);

        foo->fold();

        std::cout << foo->t.data << '\n';

        foo->apply("some_const", -3.0);

        foo->fold();

        std::cout << foo->t.data << '\n';
    }
} // DS::Expr
