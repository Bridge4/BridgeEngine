cmake_minimum_required(VERSION 3.7.0)
project(Core)
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_BINARY_DIR "Build/")
find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

add_library(Core)

if(WIN32)
  target_link_options(Core PUBLIC "LINKER:/ignore:4099")
endif()
target_include_directories(Core PUBLIC
                                        "${VULKAN_INCLUDE_DIRS}"
                                        "../Dependencies/stb-master/stb"
                                        "../Dependencies/tinyobjloader-release/"
                                        )



add_subdirectory("../Dependencies/GLFW/"
                 "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Dependencies/GLFW")
add_subdirectory("../Dependencies/glm/"
                 "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Dependencies/glm")
add_subdirectory("../Dependencies/stb-master/"
                 "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Dependencies/stb-master")


set(IMGUI_DIR "../Dependencies/imgui")

set(IMGUI_SOURCES
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp
  ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

# Attach include directories directly to the target
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)
file(GLOB_RECURSE CORE_SOURCES CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
     "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")

target_sources(Core PRIVATE ${CORE_SOURCES})

target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan glm imgui Vulkan::shaderc_combined)
