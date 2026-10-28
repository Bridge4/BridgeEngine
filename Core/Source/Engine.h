#pragma once
#include <vector>

#include "Renderer/RendererTypes.h"
#include "config.h"
class WindowingBackend;
class VulkanContext;

class Engine {
   public:
    Engine() {}
    // API Context
    VulkanContext* m_vulkanContext = 0;
    std::vector<LoadedObject> m_loadedObjects = {};

    void StartUp(ERenderingMode renderingMode,
                 WindowingBackend* windowingBackend);

    // void SetWindowRef(WindowHandler* window);
    void RenderFrame(float deltaTime);
    void Destroy();
    ERenderingMode m_renderingMode;

   private:
    std::vector<UnloadedObject> m_objectsToLoad;
};
