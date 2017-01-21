#include "board.hpp"
#include "game.hpp"

using namespace std;

int main() {
    game_2048::Game game;
    game_2048::Board board;
    auto data = *game.initialize(4, 4);
    auto status = game_2048::Status::ongoing;
    while (status == game_2048::Status::ongoing) {
        board.print_ongoing(data, game.score);
        status = game.play(data);
        if (status == game_2048::Status::won) {
            board.print_won(data, game.score);
            status = game.prompt_for_exit();
        } else if (status == game_2048::Status::lost) {
            board.print_lost(data, game.score);
            status = game.prompt_for_exit();
        } else if (status == game_2048::Status::interrupted) {
            board.print_interrupted(data, game.score);
            status = game.prompt_for_exit();
        }
    }
    return 0;
}
