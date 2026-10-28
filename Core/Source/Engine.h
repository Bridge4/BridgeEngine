#pragma once
#include <vector>

#include "Renderer/Renderer.h"
#include "Renderer/RendererTypes.h"
#include "config.h"
class WindowingBackend;

class Engine {
   public:
    Engine() {}
    Renderer* m_renderer = 0;
    std::vector<LoadedObject> m_loadedObjects = {};

    void StartUp(ERenderingMode renderingMode,
                 WindowingBackend* windowingBackend);

    // void SetWindowRef(WindowHandler* window);
    void Tick(float deltaTime);
    void Destroy();
    ERenderingMode m_renderingMode;
};
