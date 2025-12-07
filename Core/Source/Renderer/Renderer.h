#pragma once
#include <vector>

#include "Source/Renderer/FileLoader.h"
#include "config.h"
class WindowHandler;
class VulkanContext;

class Renderer {
   public:
    Renderer(WindowHandler* window) { m_windowRef = window; }
    // Window Ref
    WindowHandler* m_windowRef = 0;
    // API Context
    VulkanContext* m_vulkanContext = 0;
    std::vector<LoadedObject> m_loadedObjects = {};

    void CreateAPIContext();
    void SetWindowRef(WindowHandler* window);
    void RunRenderer();

   private:
    std::vector<UnloadedObject> m_objectsToLoad = {
        {MODELS_DIR "VikingRoom/VikingRoom.obj",
         MODELS_DIR "VikingRoom/VikingRoom.png"},
        {MODELS_DIR "Ship/ship.obj", MODELS_DIR "Ship/ship.png"}};
};
