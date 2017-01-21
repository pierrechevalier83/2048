#pragma once

#include "../cursed/include/matrix_display.hpp"

#include <boost/optional.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/algorithm/count.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <boost/range/algorithm/transform.hpp>

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

using namespace std;
using Row = vector<int>;
using Matrix = vector<Row>;

namespace game_2048 {

class Board {
   public:
    void print(const Matrix& data, int score) {
        clear();
        const auto style = ncurses::MatrixStyle(7, 3, ' ', ' ', ' ');
        auto display = ncurses::MatrixDisplay(style);
        auto data_view = view(data);
        print_title_bar(score, display.width_in_chars(data_view));
        display.print(data_view);
        print_footer();
    }

   private:
    int n_cols(const Matrix& data) {
        assert(!data.empty());
        return data[0].size();
    }
    void print_title_bar(int score, int width) {
        const auto title = "2048 [pierrec.tech]"s;
        addstr(title.c_str());
        const auto remaining_space = width - title.length();
        ncurses::aligned_right(to_string(score), remaining_space);
        addch('\n');
    }
    void print_footer() {
        const auto title = L"    [ ← ↑ → ↓ ], q for quit\n\n"s;
        addwstr(title.c_str());
    }
    vector<vector<ncurses::Cell>> view(const Matrix& data) {
        vector<vector<ncurses::Cell>> data_view;
        boost::transform(data, back_inserter(data_view), [](const auto& row) {
            vector<ncurses::Cell> row_view;
            boost::transform(row, back_inserter(row_view), [](const int value) {
                return ncurses::Cell(value == 0 ? "." : to_string(value),
                                     static_cast<int>(log2(value)));
            });
            return row_view;
        });
        return data_view;
    }
};

} // namepsace game_2048
