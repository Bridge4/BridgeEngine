#include <GLFW/glfw3.h>

#include <chrono>
#include <iostream>

#include "API/Vulkan/Components/Window/WindowingBackend.h"
#include "FileLoader.h"
#include "Renderer.h"
#include "RendererTypes.h"

int main() {
    WindowingBackend* windowingBackend = new WindowingBackend();
    windowingBackend->CreateWindowVkGLFW(1920, 1080);

    Renderer* renderer = new Renderer();

    // TODO:
    // renderer->SetWindowContext();
    //
    //
    // NOTE: What does our graphics API backend need from our windowing system?
    // 1. SwapchainHandler needs it for HandleMinimization function call so it
    // can rebuild swapchain
    //  a. HandleMinimization runs a while loop that repeatedly checks if the
    //  width and height are > 0 to determine if the window is still minimized
    //    What can we do to get a better api for this?
    // 2.
    renderer->SetWindowingBackend(windowingBackend);
    renderer->InitRenderingAPI();

    // windowingBackend->SetApiContext(renderer->m_vulkanContext);
    std::vector<UnloadedObject> objectsToLoad;

    UnloadedObject testObj;
    testObj.objFilePath = MODELS_DIR "dwarf_sword/dwarf.obj";
    testObj.materialTexFilePaths[ALBEDO] = MODELS_DIR "dwarf_sword/dwarf.png";
    testObj.materialTexFilePaths[METALLIC] = MODELS_DIR "dwarf_sword/norm.png";
    testObj.materialTexFilePaths[ROUGHNESS] =
        MODELS_DIR "dwarf_sword/rough.png";
    testObj.materialTexFilePaths[AO] = MODELS_DIR "axe/ao.png";
    testObj.materialTexFilePaths[NORMAL] = MODELS_DIR "dwarf_sword/norm.png";
    testObj.materialTexFilePaths[EMISSIVE] = MODELS_DIR "axe/emissive.png";

    // UnloadedObject testObj2;
    // testObj2.objFilePath = MODELS_DIR "axe/axe.obj";
    // testObj2.materialTexFilePaths[ALBEDO] = MODELS_DIR "axe/albedo.png";
    // testObj2.materialTexFilePaths[METALLIC] = MODELS_DIR "axe/metallic.png";
    // testObj2.materialTexFilePaths[ROUGHNESS] = MODELS_DIR
    // "axe/roughness.png"; testObj2.materialTexFilePaths[AO] = MODELS_DIR
    // "axe/ao.png"; testObj2.materialTexFilePaths[NORMAL] = MODELS_DIR
    // "axe/normals.png"; testObj2.materialTexFilePaths[EMISSIVE] = MODELS_DIR
    // "axe/emissive.png";

    objectsToLoad.push_back(testObj);
    // objectsToLoad.push_back(testObj2);

    std::vector<LoadedObject> loadedObjects = {};
    // Load Models
    for (auto& obj : objectsToLoad) {
        LoadedObject loadedObject = FileLoader::LoadObject(obj);
        loadedObjects.push_back(loadedObject);
    }
    renderer->m_loadedObjects = loadedObjects;

    std::chrono::high_resolution_clock::time_point lastFrameTime;
    lastFrameTime = std::chrono::high_resolution_clock::now();
    while (!windowingBackend->ShouldCloseGLFW()) {
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime =
            std::chrono::duration<float>(currentFrameTime - lastFrameTime)
                .count();
        lastFrameTime = currentFrameTime;
        windowingBackend->PollGLFW();
        renderer->RenderFrame(deltaTime);
    }

    return EXIT_SUCCESS;
}
