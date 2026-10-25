#pragma once
#include <vector>

#include "RendererTypes.h"
#include "config.h"
class WindowingBackend;
class VulkanContext;

class Renderer {
   public:
    Renderer() { Construct(); }
    // API Context
    VulkanContext* m_vulkanContext = 0;
    std::vector<LoadedObject> m_loadedObjects = {};

    void Construct(ERenderingMode renderingMode = ERenderingMode::Standard);
    void InitRenderingAPI();
    // void SetWindowRef(WindowHandler* window);
    void RenderFrame(float deltaTime);
    void Destroy();
    void SetWindowingBackend(WindowingBackend* windowingBackend);
    ERenderingMode m_renderingMode;

   private:
    std::vector<UnloadedObject> m_objectsToLoad;
};
