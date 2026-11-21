cmake_minimum_required(VERSION 3.15)
project(Game LANGUAGES CXX)

#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_CXX_STANDARD 20)

add_library(Game SHARED
    Source/game.cpp
)

target_include_directories(Game
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/Source"
        "${CMAKE_CURRENT_SOURCE_DIR}/../Core/Source"
)

target_compile_definitions(Game PRIVATE GAME_EXPORTS)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
  set(ROOT_BUILD_DIR "${ROOT_DIR}/build")

  add_custom_command(TARGET Game POST_BUILD
        COMMAND ${CMAKE_COMMAND} -S "${ROOT_DIR}" -B "${ROOT_BUILD_DIR}"
        COMMENT "Refreshing root compile_commands.json..."
    )
endif()
