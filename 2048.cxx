#include <curses.h>
#include <boost/optional.hpp>
#include <cstdlib>
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
    Board() {
	    // initialize ncurses
	    initscr();
		// don't buffer characters
		cbreak();
		// don't print input
        noecho();
		// capture arrow keys
		keypad(stdscr, true);
	}
    void print(const Matrix& data) {
	    refresh();
		move(0,0);
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
            //cout << setw(width - padding) << left <<
			printw("%-*s", width - padding, content.c_str()); // TODO: format
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

enum class Status { ongoing, interrupted, lost, won };

class Game {
   public:
    Game() {
        srand(time(0));
	}
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
		}
		return status;
    }

   private:
    bool randomly_insert(int value, Matrix& data) {
		vector<pair<size_t, size_t>> zeroes;
		for (size_t ii = 0; ii < data.size(); ++ii) {
		    for (size_t jj = 0; jj < data[ii].size(); ++ jj) {
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
    Status human_play(const Matrix&) {
	    //cout << flush;
		//char input;
		int input = getch();
		//cin >> input;
		if (input == 'q') {
		    printw("See you later!\n");
			return Status::interrupted;
		}
		cin.clear();
        // TODO: take input from cin and update data appropriately
		return Status::ongoing;
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
