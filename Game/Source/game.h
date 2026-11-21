#pragma once
#ifdef GAME_EXPORTS
#define GAME_API __declspec(dllexport)
#else
#define GAME_API __declspec(dllimport)
#endif

namespace Game {
extern "C" GAME_API void update();
extern "C" GAME_API void new_landscape();
}  // namespace Game
