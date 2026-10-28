#define GLFW_INCLUDE_VULKAN
#include "WindowingBackend.h"

#include <GLFW/glfw3.h>

// Create Surface
// 1. Get reference to API
// 2. Call API-specific windowing setup

// NOTE: VULKAN WINDOWING
VkResult WindowingBackend::CreateSurfaceVkGLFW(VkInstance instance,
                                               VkSurfaceKHR* surface) {
    return glfwCreateWindowSurface(instance, m_window, nullptr, surface);
}

void WindowingBackend::CreateWindowVkGLFW(uint32_t w, uint32_t h) {
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
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallbackGLFW);
    // glfwSetKeyCallback(m_window, key_callback);
}

void WindowingBackend::PollWindowNotMinimizedGLFW() {
    int w = 0, h = 0;
    glfwGetFramebufferSize(m_window, &w, &h);

    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(m_window, &w, &h);
        glfwWaitEvents();
    }
}

void WindowingBackend::GetFramebufferSizeGLFW(int* width, int* height) {
    glfwGetFramebufferSize(m_window, width, height);
}

void WindowingBackend::DestroyGLFW() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

int WindowingBackend::ShouldCloseGLFW() {
    return glfwWindowShouldClose(m_window);
}

void WindowingBackend::PollGLFW() { glfwPollEvents(); }
// Checks if the framebuffer has been resized using GLFW function
void WindowingBackend::FramebufferResizeCallbackGLFW(GLFWwindow* window,
                                                     int width, int height) {
    auto wnd =
        reinterpret_cast<WindowingBackend*>(glfwGetWindowUserPointer(window));
    wnd->framebufferResized = true;
}
