/* date = September 08, 2025 5:11 PM */


#ifndef DELTASTATION_PARSER_H
#define DELTASTATION_PARSER_H
#include "Lexer.h"

namespace DS::Expr {
    struct AST {
        Token t;
        AST *left{}, *right{};

        void apply(const std::string &ident, double value);
        void fold();
    };

    /**
     * Some invariants that the parser is guaranteed to hold:
     * - No literal or expression tokens should have children.
     * - Multiply and Divide must have two children.
     * - Add and Subtract can either have a right child, or have exactly two children.
     * - Open and Close Parentheses should not appear in the AST.
     */
    class Parser {
    public:
        AST *parse(const std::vector<Token> &tokens);
    };
} // DS::Expr

#endif //DELTASTATION_PARSER_H