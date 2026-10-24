#include "Source/Renderer/API/Vulkan/Components/Window/WindowingBackend.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>

#include "../../VulkanContext.h"
#include "../VulkanGlobalState/VulkanGlobalState.h"
#include "WindowHandlerVK.h"
// Create Surface
// 1. Get reference to API
// 2. Call API-specific windowing setup

// NOTE: VULKAN WINDOWING
VkResult WindowHandlerVK::CreateSurface() {
    return m_windowingBackend->CreateSurfaceVkGLFW(
        vulkanContext->m_vulkanGlobalState->m_instance,
        &vulkanContext->m_vulkanGlobalState->m_surface);
}

// void key_callback(GLFWwindow* m_window, int key, int scancode, int action,
// int mods)
//{
// }

void WindowHandlerVK::Create(uint32_t w, uint32_t h) {
    m_width = w, m_height = h;
    // GLFW creates an OpenGL context so we need to undo that
    glfwInit();
    // NOTE: VULKAN WINDOWING
    // Undoing OpenGL context creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(w, h, "Bridge Engine", nullptr, nullptr);
    // Very important line, if you forget this then the callback with cause an
    // access violation due to nullptr
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
    // glfwSetKeyCallback(m_window, key_callback);
}

void WindowHandlerVK::PollWindowNotMinimized() {
    int w = 0, h = 0;
    glfwGetFramebufferSize(m_window, &w, &h);

    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(m_window, &w, &h);
        glfwWaitEvents();
    }
}

void WindowHandlerVK::GetFramebufferSize(int* width, int* height) {
    glfwGetFramebufferSize(m_window, width, height);
}

void WindowHandlerVK::Destroy() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

int WindowHandlerVK::ShouldClose() { return glfwWindowShouldClose(m_window); }

void WindowHandlerVK::Poll() { glfwPollEvents(); }
// Checks if the framebuffer has been resized using GLFW function
void WindowHandlerVK::FramebufferResizeCallback(GLFWwindow* window, int width,
                                                int height) {
    auto wnd =
        reinterpret_cast<WindowHandlerVK*>(glfwGetWindowUserPointer(window));
    wnd->framebufferResized = true;
}
