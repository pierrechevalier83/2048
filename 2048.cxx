#include <boost/optional.hpp>
#include <iostream>
#include <memory>
#include <vector>

namespace game_2048 {
using namespace std;

using Row = vector<int>;
using Matrix = vector<Row>;

class Board {
   public:
    void print(const Matrix& data) {
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
        cout << '+';
        for (int i = 0; i < n; ++i) {
            cout << "-------+";
        }
        cout << '\n';
    }
    void padding_row(size_t n) {
        print_sep();
        for (int i = 0; i < n; ++i) {
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
                cout << "   .   ";
            } else {
                // TODO: format centered (7 spaces) instead!
                cout << "   " << *value << "   ";
            }
        } else {
            cout << "       ";
        }
    }
    void print_sep() { cout << '|'; }
    void new_line() { cout << '\n'; }
};

class Game {
   public:
    unique_ptr<Matrix> initialize(int n_rows, int n_cols) {
        // clang-format off
        auto data = make_unique<Matrix>(Matrix({
            {0, 0, 0, 0},
		    {0, 2, 2, 0},
		    {4, 0, 0, 0},
		    {0, 0, 0, 0}
	    }));
        // clang-format on
        // TODO: 2 2s appear randomly in the matrix
        return data;
    }
    void play(const Matrix& data) {
        human_play(data);
        computer_play(data);
    }

   private:
    void human_play(const Matrix& data) {
        // TODO: take input from cin and update data appropriately
    }
    void computer_play(const Matrix& data) {
        // TODO: generate 2 random numbers that can be 2 (proba: .9) or 4
        // (proba: .1) in two random empty cases of the matrix.
    }
};
}  // namespace game_on

int main() {
    game_2048::Game game;
    game_2048::Board board;
    auto data = game.initialize(4, 4);
    board.print(*data);
    return 0;
}
