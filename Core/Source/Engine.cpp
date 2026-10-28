#include "Engine.h"

#include <imgui.h>

#include "Renderer/API/Vulkan/VulkanContext.h"
#include "Source/Renderer/RendererTypes.h"
#include "Window/WindowingBackend.h"

void Engine::StartUp(ERenderingMode renderingMode,
                     WindowingBackend* windowingBackend) {
    this->m_vulkanContext = new VulkanContext();
    m_vulkanContext->m_windowingBackend = windowingBackend;
    this->m_renderingMode = renderingMode;
    m_vulkanContext->InitVulkan(renderingMode);
}

void Engine::RenderFrame(float deltaTime) {
    m_vulkanContext->RenderFrame(m_loadedObjects, deltaTime);
}

void Engine::Destroy() { m_vulkanContext->Destroy(); }
