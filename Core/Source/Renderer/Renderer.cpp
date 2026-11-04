#include "Renderer.h"

void Renderer::StartUp(ERenderingMode renderingMode,
                       WindowingBackend* windowingBackend) {
    this->m_vulkanContext = new VulkanContext();
    m_vulkanContext->m_windowing_backend = windowingBackend;
    this->m_renderingMode = renderingMode;
    m_vulkanContext->init_vulkan(renderingMode);
}

void Renderer::Tick(std::vector<ObjResource> loadedObjects, float deltaTime) {
    m_vulkanContext->tick(m_loadedObjects, deltaTime);
}

void Renderer::DescribeScene(std::vector<ObjResource> objectsToRender,
                             std::vector<Light> lightsToRender) {
    m_vulkanContext->describe_scene(objectsToRender, lightsToRender);
}

void Renderer::Destroy() {}
