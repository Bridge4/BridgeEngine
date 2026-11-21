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
    std::vector<ObjResource> m_loadedObjects = {};

    void StartUp(ERenderingMode renderingMode,
                 WindowingBackend* windowingBackend);

    // void SetWindowRef(WindowHandler* window);
    void Tick(float deltaTime);
    void Destroy();
    void set_clear_color(float r, float g, float b);

    ERenderingMode m_renderingMode;
};
