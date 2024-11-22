#pragma once
#include "Modules/Renderer/Renderer.h"
#include "Modules/Renderer/API/Vulkan/Components/Window.h"
#include <iostream>

int main() {
    

    try {
        Window* window = new Window();
        window->createWindow(1920, 1080);

        Renderer* renderer = new Renderer(window);
        renderer->CreateAPIContext();
        
        window->SetApiContext(renderer->vulkanContext);
        
        renderer->run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}