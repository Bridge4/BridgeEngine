#pragma once
class Window;
class VulkanBridge;


class Renderer {
public:

    Renderer(Window* window) {
        windowRef = window;
    }
    // Window Ref
    Window* windowRef = 0;
    // API Context
    VulkanBridge* vulkanContext = 0;

    void CreateAPIContext();
    void SetWindowRef(Window* window);
    void RenderLoop();
};
