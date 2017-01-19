#include "cursed_matrix.hpp"

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
        using boost::for_each;
        vector<int*> zeroes;
        for_each(data, [&](auto& row) {
            for_each(row, [&](auto& x) {
                if (x == 0) {
                    zeroes.push_back(&x);
                }
            });
        });
        if (!zeroes.empty()) {
            *zeroes[rand() % zeroes.size()] = value;
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
