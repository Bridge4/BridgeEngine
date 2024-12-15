#ifndef RENDERER_H
#define RENDERER_H
class WindowHandler;
class VulkanContext;


class Renderer {
public:

    Renderer(WindowHandler* window) {
        windowRef = window;
    }
    // Window Ref
    WindowHandler* windowRef = 0;
    // API Context
    VulkanContext* vulkanContext = 0;

    void CreateAPIContext();
    void SetWindowRef(WindowHandler* window);
    void RenderLoop();
};
#endif
