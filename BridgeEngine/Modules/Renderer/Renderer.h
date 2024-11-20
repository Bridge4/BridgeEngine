#pragma once

#include "API/Vulkan/VulkanBridge.h"




/*
*   For now, this Renderer is actually the Vulkan Instance class that will be placed in Modules/Renderer/API/Vulkan
*/
class Renderer {
public:

    Window window;
    void run() {
        vulkanBridge.window = window;
        vulkanBridge.Construct();
        vulkanBridge.renderLoop();
    }
private:
    VulkanBridge vulkanBridge;
};
