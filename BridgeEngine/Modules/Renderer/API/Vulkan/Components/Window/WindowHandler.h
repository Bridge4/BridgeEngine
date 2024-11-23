#pragma once
#include <iostream>

class VulkanContext;
struct GLFWwindow;
enum VkResult;

class Window
{

public:

    Window() {};

    void SetApiContext(VulkanContext* apiVulkan) {
        vulkanContext = apiVulkan;
    }

    void Create();

    bool framebufferResized = false;

    void Destroy();

    void handleMinimization();

    void getFramebufferSize(int* width, int* height);

    int shouldClose();

    void poll();

    GLFWwindow* getWindow() { return r_window; }

    VkResult CreateSurface();

    void Create(uint32_t width = 800, uint32_t height = 600);

    //void GLFWCALL glfwKeyCalback(GLFWwindow* r_window, int key, int scancode, int action, int mods);
    VulkanContext* vulkanContext;
private:
    GLFWwindow* r_window;

    uint32_t m_width, m_height;
    
    // HELPERS

    // Used by createWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};