#include "board.hpp"
#include "game.hpp"

using namespace std;

int main() {
    game_2048::Game game;
    game_2048::Board board;
    auto data = *game.initialize(4, 4);
    auto status = game_2048::Status::ongoing;
    while (status == game_2048::Status::ongoing) {
        board.print(data, game.score);
        status = game.play(data);
        if (status == game_2048::Status::won) {
            board.print_victory(data, game.score);
            status = game.prompt_for_exit();
        } else if (status == game_2048::Status::lost) {
            board.print_defeat(data, game.score);
            status = game.prompt_for_exit();
        }
    }
    return 0;
}
