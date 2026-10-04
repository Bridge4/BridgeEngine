#pragma once
#include <vector>

#include "RendererTypes.h"
#include "config.h"
class WindowHandler;
class VulkanContext;

class Renderer {
   public:
    Renderer(WindowHandler* window) {
        m_windowRef = window;
        UnloadedObject testObj;
        testObj.objFilePath = MODELS_DIR "dwarf_sword/dwarf.obj";
        testObj.materialTexFilePaths[ALBEDO] =
            MODELS_DIR "dwarf_sword/dwarf.png";
        testObj.materialTexFilePaths[METALLIC] =
            MODELS_DIR "dwarf_sword/norm.png";
        testObj.materialTexFilePaths[ROUGHNESS] =
            MODELS_DIR "dwarf_sword/rough.png";
        testObj.materialTexFilePaths[AO] = MODELS_DIR "axe/ao.png";
        testObj.materialTexFilePaths[NORMAL] =
            MODELS_DIR "dwarf_sword/norm.png";
        testObj.materialTexFilePaths[EMISSIVE] = MODELS_DIR "axe/emissive.png";

        UnloadedObject testObj2;
        testObj2.objFilePath = MODELS_DIR "axe/axe.obj";
        testObj2.materialTexFilePaths[ALBEDO] = MODELS_DIR "axe/albedo.png";
        testObj2.materialTexFilePaths[METALLIC] = MODELS_DIR "axe/metallic.png";
        testObj2.materialTexFilePaths[ROUGHNESS] =
            MODELS_DIR "axe/roughness.png";
        testObj2.materialTexFilePaths[AO] = MODELS_DIR "axe/ao.png";
        testObj2.materialTexFilePaths[NORMAL] = MODELS_DIR "axe/normals.png";
        testObj2.materialTexFilePaths[EMISSIVE] = MODELS_DIR "axe/emissive.png";

        m_objectsToLoad.push_back(testObj);
        m_objectsToLoad.push_back(testObj2);
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
