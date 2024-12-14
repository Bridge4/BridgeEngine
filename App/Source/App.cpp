#include <iostream>
#include "Renderer.h"
#include "API/Vulkan/Components/Window/WindowHandler.h"

int main() {
    try {
        WindowHandler* window = new WindowHandler();
        window->Create(600, 600);

        Renderer* renderer = new Renderer(window);
        renderer->CreateAPIContext();

        window->SetApiContext(renderer->vulkanContext);

        renderer->RenderLoop();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}