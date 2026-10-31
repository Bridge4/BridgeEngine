#include <GLFW/glfw3.h>

#include <chrono>

#include "Engine.h"
#include "FileSystem/FileLoader.h"
#include "Renderer/API/Vulkan/VulkanContext.h"
#include "Renderer/RendererTypes.h"
#include "Window/WindowingBackend.h"

int main() {
    WindowingBackend* windowingBackend = new WindowingBackend();
    windowingBackend->CreateWindowVkGLFW(1920, 1080);

    Engine* engine = new Engine();
    engine->StartUp(ERenderingMode::Bindless, windowingBackend);

    std::vector<ObjAsset> objectsToLoad;

    ObjAsset testObj2;
    testObj2.objFilePath = MODELS_DIR "dwarf_sword/dwarf.obj";
    // testObj2.materialTexFilePaths[ALBEDO] = MODELS_DIR
    // "dwarf_sword/dwarf.png";
    testObj2.materialTexFilePaths[ALBEDO] =
        MODELS_DIR "EngineDefault/DefaultTile.png";
    // testObj2.materialTexFilePaths[METALLIC] = MODELS_DIR
    // "dwarf_sword/metallic.png";
    testObj2.materialTexFilePaths[ORM] = MODELS_DIR "dwarf_sword/rough.png";
    // testObj2.materialTexFilePaths[AO] = MODELS_DIR "axe/ao.png";
    // testObj2.materialTexFilePaths[NORMAL] = MODELS_DIR
    // "dwarf_sword/norm.png";
    // testObj2.materialTexFilePaths[EMISSIVE] = MODELS_DIR "axe/emissive.png";
    objectsToLoad.push_back(testObj2);
    // This is how we submit loaded objects objectsToLoad.push_back(testObj2);

    std::vector<ObjResource> loadedObjects = {};
    // Load Models
    for (auto& obj : objectsToLoad) {
        ObjResource loadedObject = FileLoader::LoadObject(obj);
        loadedObjects.push_back(loadedObject);
    }
    engine->m_loadedObjects = loadedObjects;
    Light light0;
    light0.position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    light0.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light0.intensity.x = 100.0f;
    std::vector<Light> lightsToRender;
    lightsToRender.push_back(light0);

    std::chrono::high_resolution_clock::time_point lastFrameTime;
    lastFrameTime = std::chrono::high_resolution_clock::now();
    // Right now we are describing a scene and then rendering it, we cannot
    // change it after we describe it
    // The intention is to be able to modify any scene we are in, meaning we
    // need to be able to add or delete meshes, lights, etc
    // At the moment we can only load meshes from files but we want to be able
    // to programmatically describe meshes
    // We will need primitives that we can programmatically modify
    // TODO: Paramaterized Perlin Noise landscape generation vertex shader
    // TODO: Create shape primitives
    engine->m_renderer->DescribeScene(loadedObjects, lightsToRender);

    while (!windowingBackend->ShouldCloseGLFW()) {
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime =
            std::chrono::duration<float>(currentFrameTime - lastFrameTime)
                .count();
        lastFrameTime = currentFrameTime;
        windowingBackend->PollGLFW();
        engine->Tick(deltaTime);
    }

    return EXIT_SUCCESS;
}
