#pragma once
#include <vector>

#include "Source/Renderer/API/Vulkan/Components/Mesh/Mesh3D.h"
#include "Source/Renderer/FileLoader.h"
#include "config.h"
class WindowHandler;
class VulkanContext;

class Renderer {
   public:
    Renderer(WindowHandler* window) {
        m_windowRef = window;
        UnloadedObject testObj;
        testObj.objFilePath = MODELS_DIR "axe/axe.obj";
        testObj.materialTexFilePaths[ALBEDO] = MODELS_DIR "axe/albedo.png";
        testObj.materialTexFilePaths[METALLIC] = MODELS_DIR "axe/metallic.png";
        testObj.materialTexFilePaths[ROUGHNESS] =
            MODELS_DIR "axe/roughness.png";
        testObj.materialTexFilePaths[AO] = MODELS_DIR "axe/ao.png";
        testObj.materialTexFilePaths[NORMAL] = MODELS_DIR "axe/normals.png";
        testObj.materialTexFilePaths[EMISSIVE] = MODELS_DIR "axe/emissive.png";
        m_objectsToLoad.push_back(testObj);
    }
    // Window Ref
    WindowHandler* m_windowRef = 0;
    // API Context
    VulkanContext* m_vulkanContext = 0;
    std::vector<LoadedObject> m_loadedObjects = {};

    void CreateAPIContext();
    void SetWindowRef(WindowHandler* window);
    void RunRenderer();

   private:
    std::vector<UnloadedObject> m_objectsToLoad;
};
