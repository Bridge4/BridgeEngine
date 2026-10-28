#include "Engine.h"

#include <imgui.h>

#include <cstdio>

#include "Renderer/API/Vulkan/VulkanContext.h"
#include "Source/Renderer/RendererTypes.h"
#include "Window/WindowingBackend.h"

void Engine::Construct(ERenderingMode renderingMode) {
    this->m_vulkanContext = new VulkanContext();
    this->m_renderingMode = renderingMode;
}

void Engine::InitRenderingAPI() {
    if (m_vulkanContext->m_windowingBackend == nullptr) {
        printf("no windowing backend set, cannot init rendering api");
        return;
    }

    m_vulkanContext->InitVulkan(ERenderingMode::Bindless);
}

void Engine::RenderFrame(float deltaTime) {
    m_vulkanContext->RenderFrame(m_loadedObjects, deltaTime);
}

void Engine::SetWindowingBackend(WindowingBackend* windowingBackend) {
    m_vulkanContext->m_windowingBackend = windowingBackend;
}
void Engine::Destroy() { m_vulkanContext->Destroy(); }
