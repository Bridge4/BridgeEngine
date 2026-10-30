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

    void DescribeScene(std::vector<LoadedObject> objectsToRender,
                       std::vector<Light> lightsToRender);

    void Tick(std::vector<LoadedObject> loadedObjects, float deltaTime);
    void Destroy();
    ERenderingMode m_renderingMode;
    std::vector<LoadedObject> m_loadedObjects = {};

    // API Context
    VulkanContext* m_vulkanContext = 0;
};
#endif
