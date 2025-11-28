#include "Renderer.h"
#include "API/Vulkan/VulkanContext.h"
#include "API/Vulkan/Components/Window/WindowHandler.h"

void Renderer::CreateAPIContext()
{
    this->vulkanContext = new VulkanContext();

    windowRef->vulkanContext = vulkanContext;
    vulkanContext->windowHandler = windowRef;
    vulkanContext->Create();
}

void Renderer::SetWindowRef(WindowHandler* window) 
{
    this->windowRef = window;
}

void Renderer::RenderLoop()
{
    vulkanContext->RenderLoop();
}
