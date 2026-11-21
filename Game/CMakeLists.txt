cmake_minimum_required(VERSION 3.15)
project(Game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(Game SHARED
    Source/game.cpp
)

target_include_directories(Game
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>
        $<INSTALL_INTERFACE:Source>
)

target_compile_definitions(Game PRIVATE GAME_EXPORTS)
