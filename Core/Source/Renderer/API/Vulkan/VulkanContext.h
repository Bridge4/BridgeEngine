#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H



// VULKAN PLATFORM DEFINITION
#include "Source/Renderer/API/Vulkan/Components/Camera/CameraController.h"
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif // !VK_USE_PLATFORM_WIN32_KHR

// GLM FORCE USE RADIANS
#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif // !GLM_FORCE_RADIANS

// GLM VULKAN DATA ALIGNMENT 
#ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#endif // !GLM_FORCE_DEFAULT_ALIGNED_GENTYPES


#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif // !GLM_FORCE_DEPTH_ZERO_TO_ONE

// VULKAN INCLUDE
#include <vulkan/vulkan.h>

// GLM INCLUDES
#include <../glm/glm.hpp>
#include <../glm/gtc/matrix_transform.hpp>
#include <../glm/gtx/hash.hpp>

// C++ STANDARD INCLUDES
#include <cstdint> // Necessary for uint32_t
#include <fstream>
#include <vector>

#include "../../DataStructures.h"

// Forward Declarations
class DeviceHandler;
class SwapChainHandler;
class ImageHandler;
class BufferHandler;
class WindowHandler;
class RenderPassHandler;
class CameraController;
class VulkanInstanceManager;
class Mesh3D;

class VulkanContext {
public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    void CreateVulkanContext();

    void RunVulkanRenderer();

    void LoadSceneObjects();
    void UnloadSceneObjects();

    VulkanInstanceManager* m_vulkanInstanceManager;
    WindowHandler* m_windowHandler;

    const std::vector<const char*> m_validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

private:
    DeviceHandler* m_deviceHandler;
    SwapChainHandler* m_swapChainHandler;
    BufferHandler* m_bufferHandler;
    RenderPassHandler* m_renderPassHandler;
    CameraController* m_cameraController;

    ImageHandler* m_imageHandler;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    uint32_t m_windowWidth = 800;
    uint32_t m_windowHeight = 600;
    std::vector<std::tuple<std::string, std::string>> m_objList = {
        {"C:/Source/Engines/BridgeEngine/Models/VikingRoom/VikingRoom.obj", "C:/Source/Engines/BridgeEngine/Models/VikingRoom/VikingRoom.png"},
        {"C:/Source/Engines/BridgeEngine/Models/Ship/ship.obj", "C:/Source/Engines/BridgeEngine/Models/Ship/ship.png"}
    };

    VkDebugUtilsMessengerEXT m_debugMessenger;
    int m_maxFramesInFlight = 2;
    /*
    * Window is polled and frames are drawn until window is closed
    */
    

    void CreateDescriptorSetLayout();

    // GRAPHICS PIPELINE
    /*
    *   Create a graphics pipeline to render
    *
    */
    void CreateGraphicsPipeline();

    // COMMAND POOL
    /*
        Command pools manage the memory used to store command buffers
        Allows for multithreaded command recording since all commands are available together in the buffers
    */
    void CreateCommandPool();

    //VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    // TEXTURE IMAGE
    void CreateTextureImage(std::string textureImage, Mesh3D *mesh);

    // TEXTURE IMAGE VIEW
    void CreateTextureImageView(Mesh3D *mesh);

    // TEXTURE SAMPLER
    void CreateTextureSampler(Mesh3D *mesh);


    // helper function used in createTextureImage();
    void CreateImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory);

    // TODO: Figure out what the fuck this does
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


    void LoadModel(std::string modelPath, glm::vec3 scenePosition = glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3 objectRotation = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 objectScale = glm::vec3(1.0f, 1.0f, 1.0f));

    // DESCRIPTOR POOL
    void CreateDescriptorPool();

    // DESCRIPTOR SETS
    void CreateDescriptorSets();

    // beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    // COPY BUFFER
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    // SEMAPHORES AND FENCES
    // Creates semaphores and fences for each frame in flight
    void CreateSyncObjects();

    // DRAW FRAME
    void DrawFrame(float deltaTime);

    //void updateUniformBuffer(uint32_t currentImage);

    // SHADER MODULES
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    // Reading in SPIRV shaders
    static std::vector<char> readFile(const std::string& fileName) {
        std::ifstream file(fileName, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        //std::cout << buffer.size() << std::endl;

        return buffer;
    }

    // CLEANUP
    void Destroy();
};
#endif // !VULKANCONTEXT_H
