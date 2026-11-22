#include "Engine.h"

#include <imgui.h>

#include "Renderer/API/Vulkan/VulkanContext.h"
#include "Source/Renderer/Mesh/Mesh3D.h"
#include "Source/Renderer/Renderer.h"
#include "Source/Renderer/RendererTypes.h"
#include "Window/WindowingBackend.h"

void Engine::StartUp(ERenderingMode renderingMode,
                     WindowingBackend* windowingBackend) {
    m_renderer = new Renderer();
    m_renderer->StartUp(renderingMode, windowingBackend);
}

void Engine::Tick(float deltaTime) {
    m_renderer->Tick(m_loadedObjects, deltaTime);
}

void Engine::Destroy() { m_renderer->Destroy(); }

void Engine::set_clear_color(float r, float g, float b) {
    m_renderer->set_clear_color(r, g, b);
}

void Engine::generate_landscape() {
    // The engine should have a draw list, generating a landscape will insert
    // the landscape mesh into the draw list and return the index of the mesh to
    // the caller. The index of the mesh will be an opaque handle for the game
    // dll to do stuff with

    // List of Meshes loaded into the engine
    // Each mesh should have a draw flag
    // if true, it will be added to the backend_vk draw list
    // else if will not
    // Every engine tick we go through the mesh list and add the ones with the
    // draw flag to the backend draw list
    Mesh3D mesh;
    m_renderer->generate_landscape();
}
