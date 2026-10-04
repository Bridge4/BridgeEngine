cmake_minimum_required(VERSION 3.7.0)

if(WIN32)
  set(EXE_ICON "app.rc")
endif()

add_executable(App "${CMAKE_CURRENT_SOURCE_DIR}/Source/App.cpp" ${EXE_ICON})

set_target_properties(App PROPERTIES OUTPUT_NAME "BridgeEngine")

target_link_libraries(App PUBLIC Core)

target_include_directories(App PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/Core/Source/Renderer"
)
#target_include_directories(App Core)
