#pragma once
// class VulkanContext;
#include "vulkan/vulkan.h"
struct GLFWwindow;

class WindowingBackend {
   public:
    WindowingBackend() {};

    void SetApiContext() {}

    void Create();

    bool framebufferResized = false;

    void DestroyGLFW();

    void PollWindowNotMinimizedGLFW();

    void get_framebuffer_size_glfw(int* width, int* height);

    int ShouldCloseGLFW();

    void PollGLFW();

    VkResult create_surface_vk_glfw(VkInstance instance, VkSurfaceKHR* surface);

    void CreateWindowVkGLFW(uint32_t width = 800, uint32_t height = 600);

    // void key_callback(GLFWwindow* m_window, int key, int scancode, int
    // action, int mods);
    // VulkanContext* vulkanContext;
    GLFWwindow* m_window;

    uint32_t m_width, m_height;

   private:
    // HELPERS

    // Used by createWindow();
    static void FramebufferResizeCallbackGLFW(GLFWwindow* window, int width,
                                              int height);
};
