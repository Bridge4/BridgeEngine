#include <GLFW/glfw3.h>
#include <windows.h>

#include <chrono>

#include "Engine.h"
#include "EngineAPI.h"
#include "FileSystem/FileLoader.h"
#include "Renderer/API/Vulkan/VulkanContext.h"
#include "Renderer/RendererTypes.h"
#include "Window/WindowingBackend.h"
#include "config.h"

using InitGameFunc = void (*)(EngineAPI*);
using UpdateGameFunc = void (*)(EngineAPI*);
Engine* g_engine = nullptr;

void engine_set_clear_color(float r, float g, float b) {
    g_engine->set_clear_color(r, g, b);
}

int main() {
    WindowingBackend* windowingBackend = new WindowingBackend();
    windowingBackend->CreateWindowVkGLFW(1920, 1080);

    g_engine = new Engine();
    g_engine->StartUp(ERenderingMode::Bindless, windowingBackend);

    EngineAPI engine_api = {};
    engine_api.set_clear_color = &engine_set_clear_color;
    // Since we are loading Game.dll, we can't make changes to it
    // We need to copy Game.dll into a new file and add a timestamp to the
    // filename
    // We will load the temp copy of Game.dll so the original can be recompiled
    // We will watch the original file for changes, and if there are any,
    // copy the file into a new temp file and reload the functions
    const char* game_dll_dir = GAME_DLL_DIR "Game.dll";
    HMODULE h_dll = LoadLibraryA(game_dll_dir);
    if (!h_dll) {
        std::cout << "FAILED TO LOAD GAME DLL\n";
        return 1;
    }

    InitGameFunc game_init =
        reinterpret_cast<InitGameFunc>(GetProcAddress(h_dll, "game_init"));

    if (!game_init) {
        std::cout << "COULD NOT FIND INIT GAME FUNCTION IN DLL\n";
        return 1;
    }
    UpdateGameFunc game_update =
        reinterpret_cast<UpdateGameFunc>(GetProcAddress(h_dll, "game_update"));

    if (!game_update) {
        std::cout << "COULD NOT FIND UPDATE GAME FUNCTION IN DLL\n";
        return 1;
    }

    std::vector<ObjAsset> objectsToLoad;

    ObjAsset testObj2;
    testObj2.objFilePath = MODELS_DIR "dwarf_sword/dwarf.obj";
    testObj2.materialTexFilePaths[ALBEDO] = MODELS_DIR "dwarf_sword/dwarf.png";
    // testObj2.materialTexFilePaths[ALBEDO] =
    //     MODELS_DIR "EngineDefault/DefaultTile.png";
    //  testObj2.materialTexFilePaths[METALLIC] = MODELS_DIR
    //  "dwarf_sword/metallic.png";
    testObj2.materialTexFilePaths[ORM] = MODELS_DIR "dwarf_sword/rough.png";
    // testObj2.materialTexFilePaths[AO] = MODELS_DIR "axe/ao.png";
    // testObj2.materialTexFilePaths[NORMAL] = MODELS_DIR
    // "dwarf_sword/norm.png";
    // testObj2.materialTexFilePaths[EMISSIVE] = MODELS_DIR "axe/emissive.png";
    objectsToLoad.push_back(testObj2);
    // This is how we submit loaded objects objectsToLoad.push_back(testObj2);

    std::vector<ObjResource> loadedObjects = {};
    // Load Models
    // for (auto& obj : objectsToLoad) {
    //    ObjResource loadedObject = FileLoader::LoadObject(obj);
    //    loadedObjects.push_back(loadedObject);
    //}
    g_engine->m_loadedObjects = loadedObjects;
    Light light0;
    light0.position = glm::vec4(0.0f, 100.0f, 0.0f, 1.0f);
    light0.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light0.intensity.x = 1000.0f;
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
    g_engine->m_renderer->DescribeScene(loadedObjects, lightsToRender);

    windowingBackend->ShowWindow();

    game_init(&engine_api);
    while (!windowingBackend->ShouldCloseGLFW()) {
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime =
            std::chrono::duration<float>(currentFrameTime - lastFrameTime)
                .count();
        game_update(&engine_api);
        // engine->set_clear_color(1.0, 1.0, 1.0);
        lastFrameTime = currentFrameTime;
        windowingBackend->PollGLFW();
        g_engine->Tick(deltaTime);
    }
    FreeLibrary(h_dll);

    return EXIT_SUCCESS;
}
