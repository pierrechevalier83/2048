#include <curses.h>
#include <boost/optional.hpp>
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

namespace ncurses {

class ColorScheme {
   public:
    ColorScheme(const vector<int>& scheme_) : scheme(scheme_) {
        start_color();
        int index = 0;
        for (const auto& color : scheme) {
            init_pair(index++, COLOR_BLACK, color);
        }
    }
    vector<int> scheme;
};

class Color {
   public:
    Color(int n) {
        attron(COLOR_PAIR(n));
        attron(A_BOLD);
    }
    ~Color() {
        attroff(COLOR_PAIR(n));
        attroff(A_BOLD);
    }

   private:
    int n;
};

struct Cell {
    Cell(const string& s, int c) : content(s), color_code(c) {}
    string content;
    int color_code;
};

void n_chars(int n, char c) {
    for (int i = 0; i < n; ++i) {
        addch(c);
    }
}
void end_line() { addch('\n'); }
void positioned(const string& content, size_t width, int offset) {
    if (content.length() >= width) {
        printw(content.c_str());
    } else {
        n_chars(offset, ' ');
        printw("%-*s", width - offset, content.c_str());
    }
}
void centered(const string& content, size_t width) {
    auto offset = (width - content.length()) / 2;
    positioned(content, width, offset);
}
void aligned_right(const string& content, size_t width) {
    auto offset = width - content.length();
    positioned(content, width, offset);
}

struct Environment {
    Environment() {
        initscr();
        cbreak();
        noecho();
        keypad(stdscr, true);
    }
    ~Environment() { endwin(); }
};

struct MatrixStyle {
    MatrixStyle(int cell_width_, int cell_height_, char row_sep_, char col_sep_,
                char corner_sep_)
        : cell_width(cell_width_),
          cell_height(cell_height_),
          row_sep(row_sep_),
          col_sep(col_sep_),
          corner_sep(corner_sep_) {}
    int cell_width;
    int cell_height;
    char row_sep;
    char col_sep;
    char corner_sep;
};

class MatrixDisplay {
   public:
    MatrixDisplay(const MatrixStyle& style_) : style(style_) {}
    int width_in_chars(const vector<vector<Cell>>& data) {
        return (style.cell_width + 1) * n_rows(data);
    }
    void print(const vector<vector<Cell>>& data) {
        int n = n_rows(data);
        sep_row(n);
        for (const auto& row : data) {
            values(row);
            sep_row(n);
        }
    }

   private:
    int n_rows(const vector<vector<Cell>>& data) {
        assert(!data.empty());
        return data[0].size();
    }
    void sep_row(int n) {
        corner();
        for (auto i = 0; i < n; ++i) {
            ncurses::n_chars(style.cell_width, style.row_sep);
            corner();
        }
        ncurses::end_line();
    }
    void corner() { addch(style.corner_sep); }
    void values(const vector<Cell>& row) {
        const auto line_height = 1;  // TODO: count \n characters
        const auto previous_pad = (style.cell_height - line_height) / 2;
        value_padding(previous_pad, row);
        value(row);
        const auto next_pad = style.cell_height - (previous_pad + line_height);
        value_padding(next_pad, row);
    }
    void value_padding(int height, const vector<Cell>& row) {
        vector<Cell> padding;
        boost::transform(row, back_inserter(padding), [](const Cell& cell) {
            auto padding_cell = cell;
            padding_cell.content = "";
            return padding_cell;
        });
        for (int i = 0; i < height; ++i) {
            value(padding);
        }
    }
    void value(const vector<Cell>& row) {
        sep_col();
        for (const auto& cell : row) {
            {
                Color scoped(cell.color_code);
                ncurses::centered(cell.content, style.cell_width);
            }
            sep_col();
        }
        ncurses::end_line();
    }
    void sep_col() { addch(style.col_sep); }

   private:
    const MatrixStyle style;
};
}  // namespace ncurses

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
    }

   private:
    int n_cols(const Matrix& data) {
        assert(!data.empty());
        return data[0].size();
    }
    void print_title_bar(int score, int width) {
        const auto title = "2048 [pierre.tech]"s;
        printw(title.c_str());
        const auto remaining_space = width - title.length();
        ncurses::aligned_right(to_string(score), remaining_space);
        addch('\n');
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

enum class Status { ongoing, invalid_move, interrupted, lost, won };

class Game {
   public:
    Game() { srand(time(0)); }
    unique_ptr<Matrix> initialize(int n_rows, int n_cols) {
        auto data = make_unique<Matrix>(Matrix(n_rows, Row(n_cols, 0)));
        randomly_insert(2, *data);
        randomly_insert(2, *data);
        return data;
    }
    Status play(Matrix& data) {
        if (lost(data)) {
            return Status::lost;
        }
        auto status = human_play(data);
        if (status == Status::won) {
        } else if (status == Status::ongoing) {
            computer_play(data);
        } else if (status == Status::invalid_move) {
            status = Status::ongoing;
        } else if (status == Status::interrupted) {
            printw("Do you really want to quit? (y/n)\n");
            status = prompt_for_exit();
        }
        return status;
    }
    Status prompt_for_exit() {
        auto input = getch();
        if (input == 'q' || input == 'n' || input == 'N') {
            clear();
            return Status::ongoing;
        } else {
            return Status::interrupted;
        }
    }
    int score = 0;

   private:
    void randomly_insert(int value, Matrix& data) {
        vector<pair<size_t, size_t>> zeroes;
        for (size_t ii = 0; ii < data.size(); ++ii) {
            for (size_t jj = 0; jj < data[ii].size(); ++jj) {
                if (data[ii][jj] == 0) {
                    zeroes.push_back(make_pair(ii, jj));
                }
            }
        }
        if (!zeroes.empty()) {
            auto& pos = zeroes[rand() % zeroes.size()];
            data[pos.first][pos.second] = value;
        }
    }
    Status human_play(Matrix& data) {
        auto new_data = data;
        auto input = getch();
        if (input == 'q') {
            return Status::interrupted;
        } else if (input == KEY_UP) {
            new_data = up(data);
        } else if (input == KEY_DOWN) {
            new_data = down(data);
        } else if (input == KEY_RIGHT) {
            new_data = right(data);
        } else if (input == KEY_LEFT) {
            new_data = left(data);
        }
        if (new_data == data) {
            return lost(data) ? Status::lost : Status::invalid_move;
        } else {
            data = new_data;
        }
        return won(data) ? Status::won : Status::ongoing;
    }
    bool lost(const Matrix& data) {
        auto d_up = up(data);
        auto d_down = down(data);
        auto d_right = right(data);
        auto d_left = left(data);
        return (d_up == data) && (d_down == data) && (d_right == data) &&
               (d_left == data);
    }
    bool won(const Matrix& data) {
        static bool already_won = false;
        if (already_won) {
            // Only congratulate player once. If they want to keep playing, they
            // don't want to be interrupted by us.
            return false;
        }
        for (const auto& row : data) {
            for (const auto& value : row) {
                if (value >= 2048) {
                    already_won = true;
                    return true;
                }
            }
        }
        return false;
    }
    Matrix up(const Matrix& data) {
        return rotate_left(left(rotate_right(data)));
    }
    Matrix down(const Matrix& data) {
        return rotate_left(right(rotate_right(data)));
    }
    Matrix rotate_left(const Matrix& data) {
        assert(!data.empty());
        Matrix rotated(data[0].size(), Row(data.size()));
        for (size_t i = 0; i < data.size(); ++i) {
            for (size_t j = 0; j < data[i].size(); ++j) {
                rotated[j][i] = data[i][j];
            }
        }
        return rotated;
    }
    Matrix rotate_right(const Matrix& data) {
        return rotate_left(rotate_left(rotate_left(data)));
    }
    Matrix right(const Matrix& data) {
        Matrix copy = data;
        for (auto& row : copy) {
            row = slide(row);
        }
        return copy;
    }
    Matrix left(const Matrix& data) {
        Matrix copy = data;
        for (auto& row : copy) {
            auto reversed = row;
            reverse(reversed.begin(), reversed.end());
            row = slide(reversed);
            reverse(row.begin(), row.end());
        }
        return copy;
    }
    Row slide(const Row& r) {
        auto row = r;
        stable_partition(row.begin(), row.end(),
                         [](auto val) { return val == 0; });
        int previous_value = 0;
        for (auto it = row.end() - 1; it != row.begin(); --it) {
            previous_value = *it;
            if (*(it - 1) == previous_value) {
                *it *= 2;
                score += *it;
                *(it - 1) = 0;
                rotate(row.begin(), it - 1, it);
            }
        }
        return row;
    }
    void computer_play(Matrix& data) {
        int value = 2;
        if (rand() % 10 == 1) {
            value = 4;
        }
        randomly_insert(value, data);
    }
};

}  // namespace game_on

int main() {
    const auto env = ncurses::Environment();
    const auto colorScheme =
        ncurses::ColorScheme({0, 247, 78, 222, 220, 214, 208, 202, 196, 162,
                              160, 126, 90, 88, 54, 52});
    game_2048::Game game;
    game_2048::Board board;
    auto data = *game.initialize(4, 4);
    auto status = game_2048::Status::ongoing;
    while (status == game_2048::Status::ongoing) {
        board.print(data, game.score);
        status = game.play(data);

        if (status == game_2048::Status::won) {
            board.print(data, game.score);
            printw(
                "Congratulations! You won!\nDo you want to stop playing now? "
                "(y/n)\n");
            status = game.prompt_for_exit();
        } else if (status == game_2048::Status::lost) {
            board.print(data, game.score);
            printw("Game over!\nDo you want to quit? (y/n)\n");
            status = game.prompt_for_exit();
        }
    }
    return 0;
}
