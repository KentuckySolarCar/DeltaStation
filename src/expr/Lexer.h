/* date = September 02, 2025 5:53 PM */


#ifndef DELTASTATION_LEXER_H
#define DELTASTATION_LEXER_H
#include <iostream>
#include <string>
#include <vector>

namespace DS {
    namespace Expr {
        enum TokenType {
            Identifier,
            Literal,
            UnaryMinus,
            Multiply,
            Divide,
            Add,
            Subtract,
            OpenParens,
            CloseParens,
        };

        struct Token {
            TokenType ty;
            std::string data;

            void print() const {
                std::string s;
                switch (ty) {
                    case Identifier:
                        s = "Identifier(" + data + ")";
                        break;
                    case Literal:
                        s = "Literal(" + data + ")";
                        break;
                    case UnaryMinus:
                        s = "UnaryMinus";
                        break;
                    case OpenParens:
                        s = "OpenParens";
                        break;
                    case CloseParens:
                        s = "CloseParens";
                        break;
                    case Add:
                        s = "Add";
                        break;
                    case Subtract:
                        s = "Subtract";
                        break;
                    case Multiply:
                        s = "Multiply";
                        break;
                    case Divide:
                        s = "Divide";
                        break;
                    default:
                        s = "Unknown";
                }

                std::cout << s << " ";
            }
            [[nodiscard]] int compare(const Token &other) const {
                if (this->ty == other.ty) {
                    return 0;
                }
                switch (this->ty) {
                    case Add:
                    case Subtract:
                        if (other.ty == Add || other.ty == Subtract) {
                            return 0;
                        }
                    case Multiply:
                    case Divide:
                        if (other.ty == Multiply || other.ty == Divide) {
                            return 0;
                        }
                    default: return this->ty - other.ty;
                }
            }
        };

        class Lexer {
        public:
            void lex(const std::string &str, std::vector<Token> &tokens);
        private:
            size_t pos = 0;
            size_t mark = 0;

            void consume(size_t i) {
                pos += i;
                if (mark < pos) {
                    mark = pos;
                }
            }
            static bool is_expr_char(const char mark) {
                return '0' <= mark && mark <= '9' ||
                       'a' <= mark && mark <= 'z' ||
                       'A' <= mark && mark <= 'Z' ||
                       mark == '.' ||
                       mark == '_';
            }

            static bool is_numeric(const char mark) {
                return '0' <= mark && mark <= '9';
            }

            void parse_literal(const std::string &str, std::vector<Token> &tokens) {
                while (mark != str.length() && is_expr_char(str[mark])) {
                    mark++;
                }
                const size_t len = mark - pos;
                const std::string data = str.substr(pos, len);
                tokens.push_back(Token(Literal, data));
                consume(len);
            }
        };

        void test_lexer();
    } // Expr
} // DS

#endif //DELTASTATION_LEXER_H
