#include <iostream>

#include "API/Vulkan/Components/Window/WindowHandler.h"
#include "Renderer.h"

int main() {
    try {
        WindowHandler* window = new WindowHandler();
        window->Create(1920, 1080);

        // TODO: Add multiple window support
        // We can achieve this by passing an array of windows to the Renderer,
        // if the array is empty it doesn't render, if it does then it draws to
        // the windows
        Renderer* renderer = new Renderer(window);
        renderer->CreateAPIContext();

        window->SetApiContext(renderer->m_vulkanContext);

        renderer->RunRenderer();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
