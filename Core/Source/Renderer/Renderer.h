#ifndef RENDERER_H
#define RENDERER_H
#include "RendererTypes.h"
#include "Source/Renderer/API/Vulkan/VulkanContext.h"
#include "Source/Window/WindowingBackend.h"
class Renderer {
   public:
    Renderer() {};
    ~Renderer() {};
    void StartUp(ERenderingMode renderingMode,
                 WindowingBackend* windowingBackend);

    void DescribeScene(std::vector<ObjResource> objectsToRender,
                       std::vector<Light> lightsToRender);

    void Tick(std::vector<ObjResource> loadedObjects, float deltaTime);
    void Destroy();
    void set_clear_color(float r, float g, float b);
    void generate_landscape();
    ERenderingMode m_renderingMode;
    std::vector<ObjResource> m_loadedObjects = {};

    // API Context
    VulkanContext* m_vulkanContext = 0;
};
#endif
