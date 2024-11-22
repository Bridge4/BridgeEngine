#pragma once
class Window;
class VulkanBridge;

class Renderer {
public:

    Renderer(Window* window) {
        windowRef = window;
    }
    // Window Ref
    Window* windowRef;
    // API Context
    VulkanBridge* vulkanContext;

    void CreateAPIContext();
    void SetWindowRef(Window* window);
    void run();
};
