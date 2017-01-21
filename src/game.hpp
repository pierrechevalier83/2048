#pragma once

#include "../signed_size/include/signed_size.h"

#include <cstdlib>
#include <memory>
#include <vector>

#include <boost/range/algorithm/transform.hpp>

using namespace std;
using Row = vector<int>;
using Matrix = vector<Row>;

namespace game_2048 {

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
        }
        return status;
    }
    Status prompt_for_exit() {
        while (true) {
            auto input = getch();
            if (input == 'n' || input == 'N') {
                clear();
                return Status::ongoing;
            } else if (input == 'y' || input == 'Y') {
                return Status::interrupted;
            }
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
        for (int i = 0; i < signed_size(data); ++i) {
            for (int j = 0; j < signed_size(data[i]); ++j) {
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

}  // namespace game_2048
