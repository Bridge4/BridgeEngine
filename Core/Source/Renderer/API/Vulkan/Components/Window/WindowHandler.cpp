#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "WindowHandler.h"
#include "../../VulkanContext.h"
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
#include <string>

VkResult WindowHandler::CreateSurface()
{
    return glfwCreateWindowSurface(vulkanContext->m_vulkanInstanceManager->m_instance, m_window, nullptr, &vulkanContext->m_vulkanInstanceManager->m_surface);
}

//void key_callback(GLFWwindow* m_window, int key, int scancode, int action, int mods)
//{
//}

void WindowHandler::Create(uint32_t w, uint32_t h) {
    m_width = w, m_height = h;
    // GLFW creates an OpenGL context so we need to undo that
    glfwInit();
    // Undoing OpenGL context creation
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    m_window = glfwCreateWindow(w, h, "Bridge Engine", nullptr, nullptr);
    // Very important line, if you forget this then the callback with cause an access violation due to nullptr
    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
    //glfwSetKeyCallback(m_window, key_callback);

}


void WindowHandler::handleMinimization()
{
    int w = 0, h = 0;
    glfwGetFramebufferSize(m_window, &w, &h);

    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(m_window, &w, &h);
        glfwWaitEvents();
    }
    
}

void WindowHandler::getFramebufferSize(int* width, int* height)
{
    glfwGetFramebufferSize(m_window, width, height);
}

void WindowHandler::Destroy() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

int WindowHandler::ShouldClose() {
    return glfwWindowShouldClose(m_window);
}

void WindowHandler::Poll() {
    glfwPollEvents();
}
// Checks if the framebuffer has been resized using GLFW function
void WindowHandler::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    // glfwWindowUserPointer returns a reference to our app, so we cast it to HelloTriangleApplication then set that app object's 
    // framebufferResized variable to true
    auto wnd = reinterpret_cast<WindowHandler*>(glfwGetWindowUserPointer(window));
    wnd->framebufferResized = true;
}