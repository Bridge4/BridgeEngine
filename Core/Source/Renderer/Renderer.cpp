#include "Renderer.h"

#include <imgui.h>

#include <cstdio>

#include "API/Vulkan/Components/Window/WindowingBackend.h"
#include "API/Vulkan/VulkanContext.h"
#include "FileLoader.h"
#include "Source/Renderer/RendererTypes.h"

void Renderer::Construct(ERenderingMode renderingMode) {
    this->m_vulkanContext = new VulkanContext();
    this->m_renderingMode = renderingMode;
}

void Renderer::InitRenderingAPI() {
    if (m_vulkanContext->m_windowingBackend == nullptr) {
        printf("no windowing backend set, cannot init rendering api");
        return;
    }

    m_vulkanContext->InitVulkan(ERenderingMode::Bindless);
}

void Renderer::RenderFrame(float deltaTime) {
    m_vulkanContext->RenderFrame(m_loadedObjects, deltaTime);
}

void Renderer::SetWindowingBackend(WindowingBackend* windowingBackend) {
    m_vulkanContext->m_windowingBackend = windowingBackend;
}
void Renderer::Destroy() { m_vulkanContext->Destroy(); }
