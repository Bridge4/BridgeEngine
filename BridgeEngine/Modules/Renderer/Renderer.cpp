#include "Renderer.h"
#include "API/Vulkan/VulkanBridge.h"
#include "API/Vulkan/Components/Window.h"

void Renderer::CreateAPIContext()
{
    // TODO: Add actual checks if fail
    this->vulkanContext = new VulkanBridge();
    
    windowRef->vulkanContext = vulkanContext;
    vulkanContext->windowRef = windowRef;
    vulkanContext->Construct();
}

void Renderer::SetWindowRef(Window* window) {
    this->windowRef = window;
}

void Renderer::RenderLoop()
{
    vulkanContext->RenderLoop();
}


/*void run() {
        vulkanContext.windowRef = &window;
        vulkanContext.Construct();
        vulkanContext.renderLoop();
    }*/