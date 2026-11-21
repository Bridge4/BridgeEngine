#pragma once
#ifdef _WIN32
#ifdef GAME_EXPORTS
#define GAME_API extern "C" __declspec(dllexport)
#else
#define GAME_API extern "C"
#endif
#else
#define GAME_API extern "C"
#endif

#include "EngineAPI.h"

GAME_API void game_init(EngineAPI* engine) {
    engine->set_clear_color(0.0, 0.0, 0.0);
}

GAME_API void game_update(EngineAPI* engine) {
    engine->set_clear_color(0.0, 0.0, 0.0);
}
