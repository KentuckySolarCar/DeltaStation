/* date = September 13, 2025 11:12 AM */


#include "Graph.h"

#include <ranges>

#include "implot.h"
#include "expr/Parser.h"

namespace DS {
    Graph::Graph(const std::string &name, const std::string &formula, double data_width) {
        std::vector<Expr::Token> tokens;
        Expr::Lexer().lex(formula, tokens);
        Expr::AST *ast = Expr::Parser().parse(tokens);

        this->name = name;
        this->formula = ast;
        this->data_width = data_width;
    }

    void Graph::display() {
        std::vector<double> x0{};
        std::vector<double> y0{};
        double x_min = 0, x_max = data_width,
                y_min = std::numeric_limits<double>::max(),
                y_max = std::numeric_limits<double>::min();

        if (!history.empty()) {
            auto last = history.back().first;
            auto range = std::ranges::views::filter(history, [this, last](auto &p) {
                return p.first >= last - this->data_width;
            });

            for (auto &[xd, yd]: range) {
                x_min = std::fmax(0, xd - data_width);
                x_max = std::fmax(data_width, xd);
                if (yd < y_min) y_min = yd;
                if (yd > y_max) y_max = yd;
                x0.push_back(xd);
                y0.push_back(yd);
            }
        } else {
            y_min = 0;
            y_max = 1;
        }

        double width = y_max - y_min;

        // TODO: specify graph axes in config
        ImPlot::SetNextAxesLimits(x_min, x_max, y_min - width * 0.1, y_max + width * 0.1, ImPlotCond_Always);

        if (ImPlot::BeginPlot(this->get_name())) {
            ImPlot::PlotLine(
                name.c_str(),
                x0.data(),
                y0.data(),
                static_cast<int>(x0.size())
            );
            ImPlot::EndPlot();
        }
    }

    void Graph::update(const Dashboard &db, double x_val) {
        if (!tractable(db)) return;

        std::unordered_map<std::string, double> values;
        for (const std::string &ident: this->formula->idents) {
            values[ident] = *db.get_value<float>(ident);
        }

        auto data_point = this->formula->evaluate(values);
        if (data_point.has_value()) {
            this->history.emplace_back(x_val, data_point.value());
        }
    }

    /**
     *
     * @param db
     * @return If the supplied Dashboard can provide all variables used in this graph's formula.
     */
    bool Graph::tractable(const Dashboard &db) const {
        for (const std::string &ident: this->formula->idents) {
            if (!db.has_key(ident)) return false;
        }
        return true;
    }

    const char *Graph::get_name() const {
        return name.c_str();
    }
} // DS
