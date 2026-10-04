cmake_minimum_required(VERSION 3.7.0)
project(Core)
set(CMAKE_CXX_STANDARD 20)
set(PROJECT_BINARY_DIR "Build/")
find_package(Vulkan REQUIRED)

add_library(Core)

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

target_sources(Core PRIVATE
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Renderer.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Renderer.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/FileLoader.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/RendererTypes.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Camera/CameraController.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Camera/CameraController.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Camera/OrbitCamera.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Camera/OrbitCamera.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Camera/NoClipCamera.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Camera/NoClipCamera.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Mesh/Mesh3D.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/VkTypes.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/VulkanContext.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/VulkanContext.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Buffers/BufferHandler.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Buffers/BufferHandler.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/DescriptorSets/DescriptorSetHandler.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/DescriptorSets/DescriptorSetHandler.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Devices/DeviceHandler.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Devices/DeviceHandler.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Images/ImageHandler.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Images/ImageHandler.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/RenderPass/RenderPassHandler.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/RenderPass/RenderPassHandler.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/SwapChain/SwapChainHandler.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/SwapChain/SwapChainHandler.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.cpp"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Window/WindowHandler.h"
               "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Window/WindowHandler.cpp")

target_link_libraries(imgui PRIVATE glfw Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan glm imgui)
