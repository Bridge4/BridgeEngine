#pragma once
#include <vector>

#include "EngineAPI.h"
#include "Renderer/Renderer.h"
#include "Renderer/RendererTypes.h"
#include "config.h"

class WindowingBackend;

class Engine {
   public:
    Engine() {}
    Renderer* m_renderer = 0;
    std::vector<ObjResource> m_loadedObjects = {};

    void StartUp(ERenderingMode renderingMode, WindowingBackend* windowingBackend);

    // void SetWindowRef(WindowHandler* window);
    void Tick(float deltaTime);
    void Destroy();
    void set_clear_color(float r, float g, float b);
    void generate_landscape();
    void gen_box(GenBoxOptions gen_options = {});

    ERenderingMode m_renderingMode;
};

namespace bridge_engine {}  // namespace bridge_engine
