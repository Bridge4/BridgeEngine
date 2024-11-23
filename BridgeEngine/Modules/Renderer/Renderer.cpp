#include "Renderer.h"
#include "API/Vulkan/VulkanContext.h"
#include "API/Vulkan/Components/Window/WindowHandler.h"

void Renderer::CreateAPIContext()
{
    this->vulkanContext = new VulkanContext();
    
    windowRef->vulkanContext = vulkanContext;
    vulkanContext->windowRef = windowRef;
    vulkanContext->Construct();
}

void Renderer::SetWindowRef(Window* window) 
{
    this->windowRef = window;
}

void Renderer::RenderLoop()
{
    vulkanContext->RenderLoop();
}