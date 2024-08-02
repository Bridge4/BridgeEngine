cmake_minimum_required(VERSION 3.28)
project(BridgeEngine)

set(CMAKE_CXX_STANDARD 17)

set(GLFW_DIR "./glfw-3.4.bin.WIN64") # This is where the downloaded glfw folder is located
set(GLFW_INCLUDE_DIR ${GLFW_DIR}/include) # The directory where GLFW header files are located
set(GLFW_LIB_DIR ${GLFW_DIR}/lib-mingw-w64) # The directory the LINKER will look for GLFW library. You'd want to change this depending on the compiler you're using.
set(OPENGL_DIR "C:/Windows/System32/opengl32.dll")

find_package(OpenGL REQUIRED)

add_executable(BridgeEngine main.cpp)

target_include_directories(BridgeEngine
    PRIVATE ${GLFW_INCLUDE_DIR} ${OPENGL_DIR}
)

target_link_directories(BridgeEngine PRIVATE ${GLFW_LIB_DIR} )

target_link_libraries(BridgeEngine
        PRIVATE glfw3 ${OPENGL_DIR}
)


