#include <curses.h>
#include <boost/optional.hpp>

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <vector>

namespace game_2048 {
using namespace std;

using Row = vector<int>;
using Matrix = vector<Row>;

class ColorScheme {
   public:
    ColorScheme() {
        start_color();
        for (int i = 0; i < 256; ++i) {
            init_pair(i, i, COLOR_BLACK);
        }
    }
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

class Board {
   public:
    Board() {
        // initialize ncurses
        initscr();
        ColorScheme scheme;
        cbreak();
        noecho();
        keypad(stdscr, true);
    }
    void print(const Matrix& data) {
        refresh();
        move(0, 0);
        size_t n_rows = 0;
        for (const auto& row : data) {
            n_rows = row.size();
            sep_row(n_rows);
            padding_row(n_rows);
            value_row(row);
            padding_row(n_rows);
        }
        sep_row(n_rows);
    }

   private:
    void sep_row(size_t n) {
        addch('+');
        for (size_t i = 0; i < n; ++i) {
            print_n_chars(cell_width, '-');
            addch('+');
        }
        addch('\n');
    }
    void padding_row(size_t n) {
        print_sep();
        for (size_t i = 0; i < n; ++i) {
            print_value(boost::none);
            print_sep();
        }
        new_line();
    }
    void value_row(const Row& row) {
        print_sep();
        for (const auto& value : row) {
            print_value(value);
            print_sep();
        }
        new_line();
    }
    void print_value(boost::optional<int> value) {
        if (value) {
            if (*value == 0) {
                print_centered(".", cell_width);
            } else {
                Color scoped_color(static_cast<int>(log2(*value)));
                print_centered(to_string(*value), cell_width);
            }
        } else {
            print_centered("", cell_width);
        }
    }
    void print_centered(string content, size_t width) {
        if (content.length() >= width) {
            printw(content.c_str());
        } else {
            auto padding = (width - content.length()) / 2;
            print_n_chars(padding, ' ');
            printw("%-*s", width - padding, content.c_str());
        }
    }
    void print_n_chars(size_t n, char c) {
        for (size_t i = 0; i < n; ++i) {
            addch(c);
        }
    }
    void print_sep() { addch('|'); }
    void new_line() { addch('\n'); }
    const int cell_width = 7;
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
        auto status = human_play(data);
        if (status == Status::ongoing) {
            status = computer_play(data);
        } else if (status == Status::invalid_move) {
            flash();
            status = Status::ongoing;
        } else if (status == Status::interrupted) {
            printw("Do you really want to quit? (y/n)");
            auto input = getch();
            if (input == 'n' || input == 'N') {
                clear();
                return Status::ongoing;
            } else {
                return Status::interrupted;
            }
        }

        return status;
    }

   private:
    bool randomly_insert(int value, Matrix& data) {
        vector<pair<size_t, size_t>> zeroes;
        for (size_t ii = 0; ii < data.size(); ++ii) {
            for (size_t jj = 0; jj < data[ii].size(); ++jj) {
                if (data[ii][jj] == 0) {
                    zeroes.push_back(make_pair(ii, jj));
                }
            }
        }
        if (zeroes.empty()) {
            return false;
        }
        auto& pos = zeroes[rand() % zeroes.size()];
        data[pos.first][pos.second] = value;
        return true;
    }
    Status human_play(Matrix& data) {
        auto new_data = data;
        int input = getch();
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
            return Status::invalid_move;
        } else {
            data = new_data;
        }
        return Status::ongoing;
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
                *(it - 1) = 0;
                rotate(row.begin(), it - 1, it);
            }
        }
        return row;
    }
    Status computer_play(Matrix& data) {
        int value = 2;
        if (rand() % 10 == 1) {
            value = 4;
        }
        if (!randomly_insert(value, data)) {
            printw("Game over!\n");
            return Status::interrupted;
        }
        return Status::ongoing;
    }
};

}  // namespace game_on

int main() {
    game_2048::Game game;
    game_2048::Board board;
    auto data = *game.initialize(4, 4);
    auto status = game_2048::Status::ongoing;
    while (status == game_2048::Status::ongoing) {
        board.print(data);
        status = game.play(data);
    }
    return 0;
}
