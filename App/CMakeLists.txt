cmake_minimum_required(VERSION 3.7.0)


add_executable(App "${CMAKE_CURRENT_SOURCE_DIR}/Source/App.cpp")

target_link_libraries(App PUBLIC Core)

target_include_directories(App PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/Core/Source/Renderer"
)
#target_include_directories(App Core)
