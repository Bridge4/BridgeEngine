cmake_minimum_required(VERSION 3.7.0)
project(Core)
set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

add_library(Core)

target_include_directories(Core PUBLIC  "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Buffers/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Camera/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Devices/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Images/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/RenderPass/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/SwapChain/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/VulkanInstanceManager/"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Window/"
                                        "${VULKAN_INCLUDE_DIRS}"
                                        "../Dependencies/stb-master/stb"
                                        "../Dependencies/tinyobjloader-release/"
                                        )


add_subdirectory("../Dependencies/GLFW/" "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Dependencies/GLFW")
add_subdirectory("../Dependencies/glm/" "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Dependencies/glm")
add_subdirectory("../Dependencies/stb-master/" "${CMAKE_CURRENT_SOURCE_DIR}/Binaries/Dependencies/stb-master")

target_sources(Core PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/Renderer.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/VulkanContext.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Buffers/BufferHandler.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Camera/CameraHandler.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Devices/DeviceHandler.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Images/ImageHandler.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/RenderPass/RenderPassHandler.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/SwapChain/SwapChainHandler.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/VulkanInstanceManager/VulkanInstanceManager.cpp"
                                        "${CMAKE_CURRENT_SOURCE_DIR}/Source/Renderer/API/Vulkan/Components/Window/WindowHandler.cpp"
                                        )

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan glm)




