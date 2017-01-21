#include "board.hpp"
#include "game.hpp"

#include "../cursed/include/matrix_display.hpp"

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
