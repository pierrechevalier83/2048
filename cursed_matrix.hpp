#include <curses.h>

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

namespace ncurses {

class ColorScheme {
   public:
    ColorScheme(const std::vector<int>& scheme_) : scheme(scheme_) {
        start_color();
        int index = 0;
        for (const auto& color : scheme) {
            init_pair(index++, COLOR_BLACK, color);
        }
    }
    std::vector<int> scheme;
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
    Cell(const std::string& s, int c) : content(s), color_code(c) {}
    std::string content;
    int color_code;
};

void n_chars(int n, char c) {
    while (n-- > 0) {
        addch(c);
    }
}
void end_line() { addch('\n'); }
void positioned(const std::string& content, size_t width, int offset) {
    if (content.length() >= width) {
        printw(content.c_str());
    } else {
        n_chars(offset, ' ');
        printw("%-*s", width - offset, content.c_str());
    }
}
void centered(const std::string& content, size_t width) {
    auto offset = (width - content.length()) / 2;
    positioned(content, width, offset);
}
void aligned_right(const std::string& content, size_t width) {
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
    int width_in_chars(const std::vector<std::vector<Cell>>& data) {
        return (style.cell_width + 1) * n_rows(data);
    }
    void print(const std::vector<std::vector<Cell>>& data) {
        int n = n_rows(data);
        sep_row(n);
        for (const auto& row : data) {
            values(row);
            sep_row(n);
        }
    }

   private:
    int n_rows(const std::vector<std::vector<Cell>>& data) {
        assert(!data.empty());
        return data[0].size();
    }
    void sep_row(int n) {
        corner();
        while (n-- > 0) {
            n_chars(style.cell_width, style.row_sep);
            corner();
        }
        end_line();
    }
    void corner() { addch(style.corner_sep); }
    void values(const std::vector<Cell>& row) {
        const auto line_height = 1;  // TODO: count \n characters
        const auto previous_pad = (style.cell_height - line_height) / 2;
        value_padding(previous_pad, row);
        value(row);
        const auto next_pad = style.cell_height - (previous_pad + line_height);
        value_padding(next_pad, row);
    }
    void value_padding(int height, const std::vector<Cell>& row) {
        std::vector<Cell> padding;
        boost::transform(row, back_inserter(padding), [](const Cell& cell) {
            auto padding_cell = cell;
            padding_cell.content = "";
            return padding_cell;
        });
        while (height-- > 0) {
            value(padding);
        }
    }
    void value(const std::vector<Cell>& row) {
        sep_col();
        for (const auto& cell : row) {
            {
                Color scoped(cell.color_code);
                centered(cell.content, style.cell_width);
            }
            sep_col();
        }
        end_line();
    }
    void sep_col() { addch(style.col_sep); }

   private:
    const MatrixStyle style;
};
}  // namespace ncurses
