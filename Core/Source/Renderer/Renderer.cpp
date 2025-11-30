#include "Renderer.h"
#include "API/Vulkan/VulkanContext.h"
#include "API/Vulkan/Components/Window/WindowHandler.h"
#include "FileLoader.h"
#include <iostream>

void Renderer::CreateAPIContext()
{
    this->m_vulkanContext = new VulkanContext();

    m_windowRef->vulkanContext = m_vulkanContext;
    m_vulkanContext->m_windowHandler = m_windowRef;
    m_vulkanContext->CreateVulkanContext();
}

void Renderer::SetWindowRef(WindowHandler* window) 
{
    this->m_windowRef = window;
}

void Renderer::RunRenderer()
{
    std::vector<LoadedObject> loadedObjects = {};
    // Load Models
    for(auto &obj: m_objectsToLoad) {
        loadedObjects.push_back(FileLoader::LoadObject(obj));
    }


    m_vulkanContext->RunVulkanRenderer(loadedObjects);
}
