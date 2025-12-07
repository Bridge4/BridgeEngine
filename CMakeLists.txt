cmake_minimum_required(VERSION 3.7.0)
project(BridgeEngine VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

find_package(Vulkan REQUIRED)

add_subdirectory(Core "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Core")

add_subdirectory(App "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/App")


set(SHADERS_DIR "${CMAKE_SOURCE_DIR}/Core/Shaders/")
set(MODELS_DIR  "${CMAKE_SOURCE_DIR}/Core/Models/")

configure_file(
    "${CMAKE_SOURCE_DIR}/Core/config.h.in"
    "${CMAKE_BINARY_DIR}/config.h"
    @ONLY
)

# Make sure both Core and App can find the generated header
target_include_directories(Core PUBLIC "${CMAKE_BINARY_DIR}")
target_include_directories(App  PUBLIC "${CMAKE_BINARY_DIR}")
