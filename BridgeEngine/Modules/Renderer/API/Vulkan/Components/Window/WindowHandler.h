#pragma once
#include <iostream>

class VulkanContext;
struct GLFWwindow;
enum VkResult;

class WindowHandler
{

public:

    WindowHandler() {};

    void SetApiContext(VulkanContext* apiVulkan) {
        vulkanContext = apiVulkan;
    }

    void Create();

    bool framebufferResized = false;

    void Destroy();

    void handleMinimization();

    void getFramebufferSize(int* width, int* height);

    int ShouldClose();

    void Poll();

    VkResult CreateSurface();

    void Create(uint32_t width = 800, uint32_t height = 600);

    //void key_callback(GLFWwindow* r_window, int key, int scancode, int action, int mods);
    VulkanContext* vulkanContext;
    GLFWwindow* r_window;

private:

    uint32_t m_width, m_height;
    
    // HELPERS

    // Used by createWindow();
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};