#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

// VULKAN PLATFORM DEFINITION
#include <vulkan/vulkan_core.h>

#include <iostream>

#include "Source/Renderer/API/Vulkan/Components/DescriptorSets/DescriptorSetHandler.h"
#include "Source/Renderer/API/Vulkan/Components/RenderPass/RenderPassHandler.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/Camera/CameraController.h"
#include "Source/Renderer/Mesh/Mesh3D.h"
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif  // !VK_USE_PLATFORM_WIN32_KHR

//// GLM FORCE USE RADIANS
// #ifndef GLM_FORCE_RADIANS
// #define GLM_FORCE_RADIANS
// #endif  // !GLM_FORCE_RADIANS
//
//// GLM VULKAN DATA ALIGNMENT
// #ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// #endif  // !GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//
// #ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #endif  // !GLM_FORCE_DEPTH_ZERO_TO_ONE

// VULKAN INCLUDE

//// GLM INCLUDES
// #include <../glm/glm.hpp>
// #include <../glm/gtc/matrix_transform.hpp>
// #include <../glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#include "Source/Math/Math.h"

// C++ STANDARD INCLUDES
#include <cstdint>  // Necessary for uint32_t
#include <fstream>
#include <vector>

// Forward Declarations
class SwapChainHandler;
class BufferHandler;
class WindowingBackend;
class CameraController;
class VulkanGlobalState;
class Mesh3D;

namespace backend_vk {
void RenderUI();

VkPipeline CreateComputePipeline(
    std::vector<char> shader, VkDevice& logical_device,
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
    VkPipelineLayout& pipelineLayout);

std::vector<VkFramebuffer> create_swapchain_framebuffers(
    int numberOfFramebuffers, std::vector<VkImageView> perFrameImageViews,
    std::vector<VkImageView> swapChainImageViews, uint32_t framebufferWidth,
    uint32_t framebufferHeight, VkDevice& logical_device,
    render_pass::RenderPassData renderPassData);

VkCommandBuffer begin_single_time_commands(VkDevice& logical_device,
                                           VkCommandPool& commandPool);

void end_single_time_commands(VkDevice& logical_device,
                              VkCommandPool& command_pool,
                              VkQueue& graphics_queue,
                              VkCommandBuffer command_buffer);

uint32_t find_memory_type(VkPhysicalDevice& physical_device,
                          uint32_t type_filter,
                          VkMemoryPropertyFlags properties);
}  // namespace backend_vk

struct InputSSBO {
    float dataIn[1024];
};

struct OutputSSBO {
    Vertex dataOut[1024 * 1024];
};

class VulkanContext {
   public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    VulkanContext() {};
    void init_vulkan(ERenderingMode renderingMode = ERenderingMode::Standard);

    void tick(std::vector<ObjResource> objectsToRender, float deltaTime);

    void load_scene_objects();

    void unload_scene_objects();

    void new_landscape();

    VulkanGlobalState* m_vulkan_global_state;
    WindowingBackend* m_windowing_backend = nullptr;

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    // TODO: Figure out what the fuck this does
    void transition_image_layout(VkImage image, VkFormat format,
                                 VkImageLayout old_layout,
                                 VkImageLayout new_layout);

    // beginSingleTimeCommands and endSingleTimeCommands are helpers for
    // copyBuffer
    VkCommandBuffer begin_single_time_commands();

    void end_single_time_commands(VkCommandBuffer command_buffer);
    // CLEANUP
    void destroy();
    void describe_scene(std::vector<ObjResource> objects_to_render,
                        std::vector<Light> lights_to_render);
    uint32_t register_texture(VkImageView* texture_image_view,
                              VkSampler* texture_sampler);

    void run_compute_shader();
    std::vector<ObjResource> m_objects_to_render = {};

    // Reading in SPIRV shaders
    static std::vector<char> read_file(const std::string& file_name) {
        std::ifstream file(file_name, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

   private:
    // DeviceHandler* m_deviceHandler;
    SwapChainHandler* m_swap_chain_handler;
    BufferHandler* m_buffer_handler;
    CameraController* m_cameraController;
    DescriptorSetHandler* m_descriptor_set_handler;

    int m_textureHeap[22000] = {0};

    uint32_t m_windowWidth = 800;
    uint32_t m_windowHeight = 600;
    float m_timeAccumulator = 0.0f;

    VkDebugUtilsMessengerEXT m_debugMessenger;
    int m_maxFramesInFlight = 2;
    /*
     * Window is polled and frames are drawn until window is closed
     */

    // GRAPHICS PIPELINE
    /*
     *   Create a graphics pipeline to render
     *
     */
    void CreatePBRPipeline(std::vector<char> vertShaderCode,
                           std::vector<char> fragShaderCode,
                           VkPipeline* pipeline);

    void CreateShadowPassPipeline(std::vector<char> vertShaderCode,
                                  std::vector<char> fragShaderCode,
                                  VkPipeline* pipeline);
    // === BINDLESS === //
    void CreatePBRPipelineBindless(std::vector<char> vertShaderCode,
                                   std::vector<char> fragShaderCode,
                                   VkPipeline* pipeline);
    void CreateShadowPassPipelineBindless(std::vector<char> vertShaderCode,
                                          std::vector<char> fragShaderCode,
                                          VkPipeline* pipeline);
    // COMMAND POOL
    /*
        Command pools manage the memory used to store command buffers
        Allows for multithreaded command recording since all commands are
       available together in the buffers
    */
    void create_command_pool();
    VkCommandPool create_command_pool_compute(
        VkDevice& logical_device, QueueFamilyIndices queueFamilyIndices);

    // VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
    // VkImageTiling tiling, VkFormatFeatureFlags features);

    // TEXTURE IMAGE
    void CreateTextureImage(TextureResource props, Mesh3D* mesh,
                            MaterialEnums materialType);

    // TEXTURE IMAGE VIEW
    void CreateTextureImageView(Mesh3D* mesh, MaterialEnums materialType);

    // TEXTURE SAMPLER
    void CreateTextureSampler(Mesh3D* mesh, MaterialEnums materialType);
    void CreateTextureSamplerShadowPass();

    // helper function used in createTextureImage();
    void CreateImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& imageMemory);

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);

    Mesh3D LoadMesh(ObjProperties props,
                    glm::vec3 scenePosition = glm::vec3(1.0f, 1.0f, 1.0f),
                    glm::vec3 objectRotation = glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3 objectScale = glm::vec3(1.0f, 1.0f, 1.0f));

    // COPY BUFFER
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t FindMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    void RecordCommandBuffer(VkCommandBuffer commandBuffer,
                             uint32_t imageIndex);

    void RecordCommandBufferCompute(VkCommandBuffer commandBuffer,
                                    VkPipeline pipeline,
                                    VkPipelineLayout pipelineLayout,
                                    VkDescriptorSet* descriptorSet);

    // SEMAPHORES AND FENCES
    // Creates semaphores and fences for each frame in flight
    void CreateSyncObjects();

    // DRAW FRAME
    void DrawFrame(float deltaTime);

    // void updateUniformBuffer(uint32_t currentImage);

    // SHADER MODULES
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
};
#endif  // !VULKANCONTEXT_H
