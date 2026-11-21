#include "game.h"

#include <iostream>

namespace Game {
GAME_API void new_landscape() {}

GAME_API void update() { std::cout << "HELLO FROM THE GAME\n"; }
}  // namespace Game
