#include <boost/optional.hpp>
#include <iomanip>
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
        const auto cell_width = 7;
		if (value) {
            if (*value == 0) {
                print_centered(".", cell_width);
            } else {
                print_centered(to_string(*value), cell_width);
            }
        } else {
            print_centered("", cell_width);
        }
    }
    void print_centered(string content, size_t width) {
        if (content.length() >= width) {
            cout << content;
        } else {
            auto padding = (width - content.length()) / 2;
            for (size_t i = 0; i < padding; ++i) {
                cout << ' ';
            }
            cout << setw(width - padding) << left << content;
        }
    }
    void print_sep() { cout << '|'; }
    void new_line() { cout << '\n'; }
};

class Game {
   public:
    unique_ptr<Matrix> initialize(int n_rows, int n_cols) {
        auto data = make_unique<Matrix>(Matrix(n_rows, Row(n_cols, 0)));
        randomly_insert(2, *data);
        randomly_insert(2, *data);
        return data;
    }
    void play(const Matrix& data) {
        human_play(data);
        computer_play(data);
    }

   private:
    void randomly_insert(int value, Matrix& data) {
        // TODO: random and empty cell
        data[0][2] = value;
    }
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
