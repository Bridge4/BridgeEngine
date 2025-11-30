#pragma once
#include "Source/Renderer/FileLoader.h"
#include <vector>
#include <string>
class WindowHandler;
class VulkanContext;


class Renderer {
public:

    Renderer(WindowHandler* window) {
        m_windowRef = window;
    }
    // Window Ref
    WindowHandler* m_windowRef = 0;
    // API Context
    VulkanContext* m_vulkanContext = 0;
    std::vector<LoadedObject> m_loadedObjects = {};

    void CreateAPIContext();
    void SetWindowRef(WindowHandler* window);
    void RunRenderer();
    void LoadSceneObjects();
private: 
    std::vector<UnloadedObject> m_objectsToLoad = {
        {"C:/Source/Engines/BridgeEngine/Models/VikingRoom/VikingRoom.obj", "C:/Source/Engines/BridgeEngine/Models/VikingRoom/VikingRoom.png"},
        {"C:/Source/Engines/BridgeEngine/Models/Ship/ship.obj", "C:/Source/Engines/BridgeEngine/Models/Ship/ship.png"}
    };
};
