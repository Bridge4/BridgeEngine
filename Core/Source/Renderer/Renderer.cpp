#include "Renderer.h"

#include <imgui.h>

#include <cstdio>

#include "API/Vulkan/Components/Window/WindowingBackend.h"
#include "API/Vulkan/VulkanContext.h"
#include "FileLoader.h"

void Renderer::Construct() { this->m_vulkanContext = new VulkanContext(); }
void Renderer::InitRenderingAPI() {
    if (m_vulkanContext->m_windowingBackend == nullptr) {
        printf("no windowing backend set, cannot init rendering api");
        return;
    }

    m_vulkanContext->InitVulkan();
}

// void Renderer::SetWindowRef(WindowHandler* window) {
//     // this->m_windowRef = window;
// }

void Renderer::RenderFrame(float deltaTime) {
    // std::vector<LoadedObject> loadedObjects = {};
    //// Load Models
    // for (auto& obj : m_objectsToLoad) {
    //     LoadedObject loadedObject = FileLoader::LoadObject(obj);
    //     loadedObjects.push_back(loadedObject);
    // }

    // ImGui::ShowDemoWindow();
    m_vulkanContext->RenderFrame(m_loadedObjects, deltaTime);
}

void Renderer::SetWindowingBackend(WindowingBackend* windowingBackend) {
    m_vulkanContext->m_windowingBackend = windowingBackend;
}
void Renderer::Destroy() { m_vulkanContext->Destroy(); }
