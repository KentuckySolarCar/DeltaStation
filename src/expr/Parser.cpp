/* date = September 08, 2025 5:11 PM */


#include "Parser.h"

namespace DS::Expr {
    void AST::apply(const std::string &ident, double value) {
        std::vector<AST *> applicants;
        applicants.push_back(this);
        while (!applicants.empty()) {
            AST *curr = applicants.back();
            applicants.pop_back();
            switch (curr->t.ty) {
                case Expression:
                    if (curr->t.data == ident) {
                        curr->t.ty = Literal;
                        curr->t.data = std::to_string(value);
                    }
                    break;
                case Add:
                case Subtract:
                case Multiply:
                case Divide:
                    if (curr->left)
                        applicants.push_back(curr->left);
                case UnaryMinus:
                    if (curr->right)
                        applicants.push_back(curr->right);
                    break;
                case Literal:
                    break;
                default:
                    // invariants of "parse" imply we should not get here.
                    std::cout << "Invalid token type detected" << '\n';
                    exit(-1);
            }
        }
    }

    void _fold_helper(AST *node) {
        if (!node->left && !node->right)
            return;
        if (node->left)
            _fold_helper(node->left);
        if (node->right)
            _fold_helper(node->right);
        switch (node->t.ty) {
            case UnaryMinus:
                if (node->right) {
                    if (node->right->t.ty == Literal) {
                        double right = std::stod(node->right->t.data);
                        node->t.ty = Literal;
                        node->t.data = std::to_string(-right);
                        delete node->right;
                        node->right = nullptr;
                    }
                }
            case Add:
                if (node->left && node->right) {
                    if (node->left->t.ty == Literal && node->right->t.ty == Literal) {
                        double left = std::stod(node->left->t.data), right = std::stod(node->right->t.data);
                        node->t.ty = Literal;
                        node->t.data = std::to_string(left + right);
                        delete node->left;
                        delete node->right;
                        node->left = nullptr;
                        node->right = nullptr;
                    }
                }
                break;
            case Subtract:
                if (node->left && node->right) {
                    if (node->left->t.ty == Literal && node->right->t.ty == Literal) {
                        double left = std::stod(node->left->t.data), right = std::stod(node->right->t.data);
                        node->t.ty = Literal;
                        node->t.data = std::to_string(left - right);
                        delete node->left;
                        delete node->right;
                        node->left = nullptr;
                        node->right = nullptr;
                    }
                }
                break;
            case Multiply:
                if (node->left && node->right) {
                    if (node->left->t.ty == Literal && node->right->t.ty == Literal) {
                        double left = std::stod(node->left->t.data), right = std::stod(node->right->t.data);
                        node->t.ty = Literal;
                        node->t.data = std::to_string(left * right);
                        delete node->left;
                        delete node->right;
                        node->left = nullptr;
                        node->right = nullptr;
                    }
                }
                break;
            case Divide:
                if (node->left && node->right) {
                    if (node->left->t.ty == Literal && node->right->t.ty == Literal) {
                        double left = std::stod(node->left->t.data), right = std::stod(node->right->t.data);
                        node->t.ty = Literal;
                        node->t.data = std::to_string(left / right);
                        delete node->left;
                        delete node->right;
                        node->left = nullptr;
                        node->right = nullptr;
                    }
                }
                break;
            case Literal:
            case Expression:
                std::cout <<
                        "Warning: attempt to fold literal or expression subtree. These nodes should have no subchildren.\n";
                break;
            default:
                std::cout << "Invalid token found in AST!\n";
                exit(-1);
        }
    }

    void AST::fold() {
        _fold_helper(this);
    }

    AST *_parse_helper(const std::vector<Token> &tokens, size_t curr, size_t mark) {
        if (curr == mark)
            return nullptr;

        // drop surrounding parentheses
        while (tokens[curr].ty == OpenParens) {
            size_t s = curr + 1;
            size_t parens_depth = 1;
            while (s < mark && parens_depth) {
                if (tokens[s].ty == CloseParens) {
                    parens_depth--;
                } else if (tokens[s].ty == OpenParens) {
                    parens_depth++;
                }
                s++;
            }
            if (parens_depth) {
                std::cout << "Unclosed open parentheses\n";
                exit(-1);
            }
            if (s == mark) {
                // we've found full surrounding parentheses. drop em.
                curr++;
                mark--;
            } else {
                break;
            }
        }

        const Token *root = nullptr;
        ssize_t root_pos = -1;

        for (size_t i = curr; i < mark; i++) {
            if (tokens[i].ty == OpenParens) {
                // search for closing parens
                size_t parens_depth = 1;
                size_t pos = i + 1;
                while (pos < mark && parens_depth) {
                    if (tokens[pos].ty == OpenParens) {
                        parens_depth++;
                    } else if (tokens[pos].ty == CloseParens) {
                        parens_depth--;
                    }
                    pos++;
                }
                if (parens_depth) {
                    std::cout << "Unclosed open parentheses detected.\n";
                    exit(-1);
                }
                i = pos - 1;
            } else if (tokens[i].ty == CloseParens) {
                std::cout << "Invalid token: CloseParens\n";
                exit(-1);
            } else {
                if (root) {
                    if (root->compare(tokens[i]) < 0) {
                        root = &tokens[i];
                        root_pos = i;
                    }
                } else {
                    root = &tokens[i];
                    root_pos = i;
                }
            }
        }

        if (!root) {
            std::cout << "No root node detected.\n";
            exit(-1);
        }
        // now we recurse
        AST *root_node = new AST();
        root_node->t = *root;
        root_node->left = _parse_helper(tokens, curr, root_pos);
        root_node->right = _parse_helper(tokens, root_pos + 1, mark);

        return root_node;
    }
    AST *Parser::parse(const std::vector<Token> &tokens) {
        return _parse_helper(tokens, 0, tokens.size());
    }
} // DS::Expr
