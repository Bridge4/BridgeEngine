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

namespace fs = std::filesystem;

using InitGameFunc = void (*)(EngineAPI*);
using UpdateGameFunc = void (*)(EngineAPI*);
Engine* g_engine = nullptr;

void engine_set_clear_color(float r, float g, float b) {
    g_engine->set_clear_color(r, g, b);
}

void engine_generate_landscape() { g_engine->generate_landscape(); }

const char* create_or_update_live_game_dll(const char* game_dll_dir) {
    auto now = std::chrono::floor<std::chrono::seconds>(
        std::chrono::system_clock::now());
    std::chrono::zoned_time local_time{std::chrono::current_zone(), now};

    // Use filename-safe characters (no colons) and include the .dll extension
    fs::path dest_path =
        fs::path(GAME_DLL_DIR) /
        std::format("LIVE_Game_{:%Y-%m-%d_%H-%M-%S}.dll", local_time);

    fs::copy_file(game_dll_dir, dest_path,
                  fs::copy_options::overwrite_existing);
    static std::string live_game_dll_path;
    live_game_dll_path = dest_path.string();
    return live_game_dll_path.c_str();
}

struct GameCode {
    const char* game_code_dir;
    const char* game_dll_dir;
    const char* live_game_dll_dir;
    std::filesystem::file_time_type current_write_time;
};

bool check_for_game_code_recompilation(GameCode& game_code) {
    std::error_code ec;
    fs::file_time_type last_write =
        fs::last_write_time(game_code.game_code_dir, ec);
    if (ec) {
        return false;
    }
    if (last_write != game_code.current_write_time) {
        // We need to mark for recompile this shader and mark its
        // pipeline as dirty
        game_code.current_write_time = last_write;
        return true;
    } else {
        return false;
    }
}

bool compile_game_code() {
    PROCESS_INFORMATION procInfo = {};
    std::string cmd = "cmake --build \"" + std::string(GAME_BUILD_DIR) + "\"";

    std::cout << "[HotReload] Compiling Game.dll..." << std::endl;

    // Blocks until completion and returns the exit code
    int exitCode = std::system(cmd.c_str());
    std::cout << "[HotReload] Compilation Complete\n";

    return true;
}

int main() {
    WindowingBackend* windowingBackend = new WindowingBackend();
    windowingBackend->CreateWindowVkGLFW(1920, 1080);

    g_engine = new Engine();
    g_engine->StartUp(ERenderingMode::Bindless, windowingBackend);

    EngineAPI engine_api = {};
    engine_api.set_clear_color = &engine_set_clear_color;
    engine_api.generate_landscape = &engine_generate_landscape;
    // Since we are loading Game.dll, we can't make changes to it
    // We need to copy Game.dll into a new temp file
    // We will load the temp copy of Game.dll so the original can be recompiled
    // We will watch the original file for changes, and if there are any,
    // copy the file into a new temp file and reload the functions

    // NOTE: On startup we are assuming the existence of a compiled Game.dll
    const char* game_dll_dir = GAME_DLL_DIR "Game.dll";
    GameCode game_code = {};
    game_code.game_code_dir = GAME_CODE_DIR;
    game_code.game_dll_dir = game_dll_dir;
    // Create the live copy of the game dll that we will load functions from
    game_code.live_game_dll_dir =
        create_or_update_live_game_dll(game_code.game_dll_dir);

    std::error_code ec;
    game_code.current_write_time =
        fs::last_write_time(game_code.game_code_dir, ec);
    std::cout << "LIVE CODE PATH:::::: " << game_code.game_code_dir << "\n";

    HMODULE h_dll = LoadLibraryA(game_code.live_game_dll_dir);
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
    g_engine->m_renderer->DescribeScene(loadedObjects, lightsToRender);

    windowingBackend->ShowWindow();

    game_init(&engine_api);
    while (!windowingBackend->ShouldCloseGLFW()) {
        if (check_for_game_code_recompilation(game_code)) {
            if (compile_game_code()) {
                game_code.live_game_dll_dir =
                    create_or_update_live_game_dll(game_code.game_dll_dir);
                FreeLibrary(h_dll);
                h_dll = LoadLibraryA(game_code.live_game_dll_dir);
                if (!h_dll) {
                    std::cout << "FAILED TO LOAD GAME DLL\n";
                    return 1;
                }

                game_init = reinterpret_cast<InitGameFunc>(
                    GetProcAddress(h_dll, "game_init"));

                if (!game_init) {
                    std::cout << "COULD NOT FIND INIT GAME FUNCTION IN DLL\n";
                    return 1;
                }
                game_update = reinterpret_cast<UpdateGameFunc>(
                    GetProcAddress(h_dll, "game_update"));

                if (!game_update) {
                    std::cout << "COULD NOT FIND UPDATE GAME FUNCTION IN DLL\n";
                    return 1;
                }
                // Reload functions
            }
        }
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
