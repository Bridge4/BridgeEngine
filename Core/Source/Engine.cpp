#include "Engine.h"

#include <imgui.h>

#include "Renderer/API/Vulkan/VulkanContext.h"
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
