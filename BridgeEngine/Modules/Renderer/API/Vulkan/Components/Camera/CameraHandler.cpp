#include "CameraHandler.h"
#include <chrono>
// GLM INCLUDES

#include "../SwapChain/SwapChainHandler.h"
#include "../Buffers/BufferHandler.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include "../../../../DataStructures.h"
#include "../Window/WindowHandler.h"
#include "GLFW/glfw3.h"

void CameraHandler::Move(uint32_t currentImage)
{
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_W)) {
        x += 0.001;
    }
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_S)) {
        x -= 0.001;
    }
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_A)) {
        y += 0.001;
    }
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_D)) {
        y -= 0.001;
    }
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), /*time * */glm::radians(70.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.model = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(90.0f), swapChainHandler->SwapChainExtent.width / (float)swapChainHandler->SwapChainExtent.height, 0.1f, 100.0f);
    // IMPORTANT: VULKAN HAS INVERTED Y AXIS TO OPENGL AND GLM WAS DESIGNED FOR OPENGL. THIS CONVERTS TO VULKAN.
    ubo.proj[1][1] *= -1;

    memcpy(bufferHandler->UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}
