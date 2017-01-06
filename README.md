2048 - game
-----------
A simple C++/ncurses implementation of the popular game

Build
-----
clang++ --std=c++14 -Werror -Wall -Wextra -lncurses 2048.cxx -o 2048

Run
---
./2048

TODO
----
- Implement Game over when no more move is available
- Implement optional game over when 2048 tile is generated
- CMake
- Use int consistently
- Remove raw for loops
