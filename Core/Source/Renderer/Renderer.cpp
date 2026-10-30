#include "Renderer.h"

void Renderer::StartUp(ERenderingMode renderingMode,
                       WindowingBackend* windowingBackend) {
    this->m_vulkanContext = new VulkanContext();
    m_vulkanContext->m_windowingBackend = windowingBackend;
    this->m_renderingMode = renderingMode;
    m_vulkanContext->InitVulkan(renderingMode);
}

void Renderer::Tick(std::vector<LoadedObject> loadedObjects, float deltaTime) {
    m_vulkanContext->Tick(m_loadedObjects, deltaTime);
}

void Renderer::DescribeScene(std::vector<LoadedObject> objectsToRender,
                             std::vector<Light> lightsToRender) {
    m_vulkanContext->DescribeScene(objectsToRender, lightsToRender);
}

void Renderer::Destroy() {}
