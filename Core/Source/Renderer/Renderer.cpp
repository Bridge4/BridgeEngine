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


    m_vulkanContext->RunVulkanRenderer();
}


void VulkanContext::LoadSceneObjects() {
    std::cout << "LOADING SCENE OBJECTS \n";
    int meshCount = 0;
    float xPos = 0.0f;
    for (const auto &obj: m_objList){
        std::string modelPath = std::get<0>(obj);
        std::string texturePath = std::get<1>(obj);
        LoadModel(modelPath, glm::vec3(xPos, 0.0f, 0.0f));
        std::cout << "OBJECT LOADED...\n";
        //CreateTextureImage(texturePath, &m_vulkanInstanceManager->m_meshList[meshCount]);
        //CreateTextureImageView(&m_vulkanInstanceManager->m_meshList[meshCount]);
        //CreateTextureSampler(&m_vulkanInstanceManager->m_meshList[meshCount]);
        meshCount++;
        xPos += 5.0f;
    }

    //m_bufferHandler->CreateVertexBuffer(m_vertices);
    //m_bufferHandler->CreateIndexBuffer(m_indices);
    //m_bufferHandler->CreateUniformBuffers();

    //CreateDescriptorPool();
    //CreateDescriptorSets();
}
