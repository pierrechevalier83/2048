#include <boost/optional.hpp>
#include <iostream>
#include <vector>

using namespace std;

class Board {
   public:
    void print(const vector<vector<int>>& data) {
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
    void value_row(const vector<int>& row) {
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

int main() {
    // clang-format off
    vector<vector<int>> data{
        {0, 0, 0, 0},
		{0, 2, 2, 0},
		{4, 0, 0, 0},
		{0, 0, 0, 0}
	};
	// clang-format on
    Board b;
    b.print(data);
}
