#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "WindowHandler.h"
#include "../../VulkanContext.h"
#include <string>

VkResult Window::CreateSurface()
{
    return glfwCreateWindowSurface(vulkanContext->m_instance, r_window, nullptr, &vulkanContext->m_surface);
}

void Window::Create(uint32_t w, uint32_t h) {
    m_width = w, m_height = h;
    // GLFW creates an OpenGL context so we need to undo that
    glfwInit();

    // Undoing OpenGL context creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    r_window = glfwCreateWindow(w, h, "Bridge Engine", nullptr, nullptr);
    // Very important line, if you forget this then the callback with cause an access violation due to nullptr
    glfwSetWindowUserPointer(r_window, this);
    glfwSetFramebufferSizeCallback(r_window, framebufferResizeCallback);
}

void Window::handleMinimization()
{
    int w = 0, h = 0;
    glfwGetFramebufferSize(r_window, &w, &h);

    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(r_window, &w, &h);
        glfwWaitEvents();
    }
    
}

void Window::getFramebufferSize(int* width, int* height)
{
    glfwGetFramebufferSize(r_window, width, height);
}

void Window::Destroy() {
    glfwDestroyWindow(r_window);
    glfwTerminate();
}

int Window::shouldClose() {
    return glfwWindowShouldClose(r_window);
}

void Window::poll() {
    glfwPollEvents();
}
// Checks if the framebuffer has been resized using GLFW function
void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    // glfwWindowUserPointer returns a reference to our app, so we cast it to HelloTriangleApplication then set that app object's 
    // framebufferResized variable to true
    auto wnd = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    wnd->framebufferResized = true;
}