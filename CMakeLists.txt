cmake_minimum_required(VERSION 3.7.0)
project(BridgeEngine VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)

add_subdirectory(Core "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Core")

add_subdirectory(App "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/App")



