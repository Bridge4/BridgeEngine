#pragma once
#include "API/Vulkan/VulkanBridge.h"

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
