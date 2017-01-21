#pragma once

#include "../cursed/include/matrix_display.hpp"

#include <boost/optional.hpp>
#include <boost/range/algorithm/transform.hpp>

#include <cmath>
#include <memory>
#include <vector>

using namespace std;
using Row = vector<int>;
using Matrix = vector<Row>;

namespace game_2048 {

class Board {
   public:
    Board() {
        const auto env = ncurses::Environment();
        const auto colorScheme =
            ncurses::ColorScheme({0, 247, 78, 222, 220, 214, 208, 202, 196, 162,
                                  160, 126, 90, 88, 54, 52});
    }
    void print(const Matrix& data, int score) {
        clear();
        /* TODO: box this way
          ┏━┳━┓
          ┃ ┃ ┃
          ┣━╋━┫
          ┃ ┃ ┃
          ┗━┻━┛
        */
        const auto style = ncurses::MatrixStyle(7, 3, L" ", L" ", L" ");

        auto display = ncurses::MatrixDisplay(style);
        auto data_view = view(data);
        if (!display_width) {
            display_width =
                boost::make_optional(display.width_in_chars(data_view));
        }
        print_title_bar(score, *display_width);
        display.print(data_view);
        print_footer(*display_width);
    }
    void print_victory(const Matrix& data, int score) {
        print(data, score);
        auto width = *display_width;
        ncurses::centered(L"Congratulations! You won."s, width);
        ncurses::end_line();
        ncurses::centered(L"Do you want to stop playing? (y/n)"s, width);
        ncurses::end_line();
    }
    void print_defeat(const Matrix& data, int score) {
        print(data, score);
        auto width = *display_width;
        ncurses::centered(L"Game over!"s, width);
        ncurses::end_line();
        ncurses::centered(L"Do you want quit? (y/n)"s, width);
        ncurses::end_line();
    }

   private:
    int n_cols(const Matrix& data) {
        assert(!data.empty());
        return data[0].size();
    }
    void print_title_bar(int score, int width) {
        const auto title = L"2048 [pierrec.tech]"s;
        ncurses::aligned_left(title, title.length());
        const auto remaining_space = width - title.length();
        ncurses::aligned_right(to_wstring(score), remaining_space);
        ncurses::end_line();
    }
    void print_footer(int width) {
        const auto title = L"[ ← ↑ → ↓ ], q for quit"s;
        ncurses::centered(title, width);
        ncurses::end_line();
        ncurses::end_line();
    }
    vector<vector<ncurses::Cell>> view(const Matrix& data) {
        vector<vector<ncurses::Cell>> data_view;
        boost::transform(data, back_inserter(data_view), [](const auto& row) {
            vector<ncurses::Cell> row_view;
            boost::transform(row, back_inserter(row_view), [](const int value) {
                return ncurses::Cell(value == 0 ? L"." : to_wstring(value),
                                     static_cast<int>(log2(value)));
            });
            return row_view;
        });
        return data_view;
    }
    boost::optional<int> display_width;
};

}  // namespace game_2048
