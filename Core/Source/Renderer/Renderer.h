#pragma once
#include <vector>

#include "RendererTypes.h"
#include "config.h"
class WindowingBackend;
class VulkanContext;

class Renderer {
   public:
    Renderer() { Construct(); }
    // Window Ref
    // WindowHandler* m_windowRef = 0;
    // API Context
    VulkanContext* m_vulkanContext = 0;
    std::vector<LoadedObject> m_loadedObjects = {};

    void Construct();
    void InitRenderingAPI();
    // void SetWindowRef(WindowHandler* window);
    void RenderFrame(float deltaTime);
    void Destroy();
    void SetWindowingBackend(WindowingBackend* windowingBackend);

   private:
    std::vector<UnloadedObject> m_objectsToLoad;
};
