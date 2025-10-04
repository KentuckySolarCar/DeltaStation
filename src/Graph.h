/* date = September 13, 2025 11:12 AM */


#ifndef DELTASTATION_GRAPH_H
#define DELTASTATION_GRAPH_H
#include <string>
#include <vector>

#include "Dashboard.h"

namespace DS {
    namespace Expr {
        struct AST;
    }

    class Graph {
    public:
        explicit Graph(const std::string &name, const std::string &formula, double data_width);

        void display();
        void update(const Dashboard &db, double x_val);

        [[nodiscard]] bool tractable(const Dashboard &db) const;

        const char *get_name() const;

    private:
        Expr::AST *formula = nullptr;
        std::vector<std::pair<double, double>> history;
        std::string name;
        double data_width;
    };
} // DS

#endif //DELTASTATION_GRAPH_H