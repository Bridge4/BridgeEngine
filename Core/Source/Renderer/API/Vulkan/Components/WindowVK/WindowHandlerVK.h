#pragma once
#include "../ComponentDeclarations.h"
// class VulkanContext;
#include "vulkan/vulkan.h"
struct GLFWwindow;

// API Specific Window handler
// Should carry a reference to the windowing front end so we decouple vulkan
// from whatever windowing library we are using

struct FrameBufferData {
    int height;
    int width;
};

class WindowHandlerVK {
   public:
    WindowHandlerVK(WindowingBackend* windowingBackend) {
        m_windowingBackend = windowingBackend;
    };

    void SetApiContext(VulkanContext* apiVulkan) { vulkanContext = apiVulkan; }

    void Create();

    bool framebufferResized = false;

    void Destroy();

    void PollWindowNotMinimized();

    void GetFramebufferSize(int* width, int* height);

    int ShouldClose();

    void Poll();

    VkResult CreateSurface();

    void Create(uint32_t width = 800, uint32_t height = 600);

    // void key_callback(GLFWwindow* m_window, int key, int scancode, int
    // action, int mods);
    //
    VulkanContext* vulkanContext;
    GLFWwindow* m_window;

    uint32_t m_width, m_height;
    FrameBufferData m_frameBufferData;

   private:
    // HELPERS

    // Used by createWindow();
    static void FramebufferResizeCallback(GLFWwindow* window, int width,
                                          int height);
    WindowingBackend* m_windowingBackend;
};
