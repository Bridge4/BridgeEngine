#pragma once
class Window;
class VulkanContext;


class Renderer {
public:

    Renderer(Window* window) {
        windowRef = window;
    }
    // Window Ref
    Window* windowRef = 0;
    // API Context
    VulkanContext* vulkanContext = 0;

    void CreateAPIContext();
    void SetWindowRef(Window* window);
    void RenderLoop();
};
