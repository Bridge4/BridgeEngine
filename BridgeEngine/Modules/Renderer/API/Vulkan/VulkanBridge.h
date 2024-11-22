#pragma once

// VULKAN PLATFORM DEFINITION
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

// NOT QUITE SURE WHAT THIS DOES 
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif // !GLM_FORCE_DEPTH_ZERO_TO_ONE

// VULKAN INCLUDE
#include <vulkan/vulkan.h>

// GLM INCLUDES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

// C++ STANDARD INCLUDES
#include <cstdint> // Necessary for uint32_t
#include <fstream>
#include <vector>

// VULKAN BRIDGE RENDERER COMPONENTS INCLUDES
#include "../../DataStructures.h"
#include "Components/Initializer.h"
#include "Components/ImageView.h"
#include "Components/SwapChain.h"
#include "Components/Window.h"

class VulkanBridge {
    /*
    *
    *
    *
    * The renderer draws things to the screen, different APIs have different ways of drawing things to the screen, therefore,
    * the renderer draw call should be a virtual function implemented by the API instance we have created.
    *
    * GRAPHICS PIPELINE
    *
    * The graphics pipeline needs a reference to several things:
    *   - Swap Chain Extent
    *   - Descriptor Set Layout
    *   - Logical Device
    *   - Pipeline Layout
    *   - Render Pass
    *
    *   We will get this info by accessing the Vulkan instance
    *
    *   The Graphics Pipeline needs a pointer to a Vulkan Instance.
    *   When the Pipeline is initialized, the Instance Pointer is pointed towards the current Vulkan Instance.
    *   From there we can then access all the information we need to setup the pipeline.
    *
    *   This also means that our Vulkan Instance needs to have a pointer to its constituent parts
    *
    *   We can have many Graphics Pipelines, each time a Pipeline is created Vulkan will need to add it to a data structure
    *   that will keep track of all the active Pipelines.
    *
    *   As a base case we will only be creating one pipeline, keeping in mind that in the future we will need to store pipelines
    *   in a data structure.
    *
    *
    *   Start app -> Initialize Renderer -> Vulkan API selected -> Spin up an instance of Vulkan
    *
    *   Vulkan Init -> RenderPassCreator.Create()
    *               -> Vulkan Instance RenderPass set to value returned by RenderPassCreator.Create()
    *               -> RenderPassCreator.Clean();
    *
    *   Repeat process with each member
    *
    *   TODO: Proof of concept using existing abstractions.
    *
    *   Attempting first with Swap Chains.
    *
    *   vulkanInitializer.vulkanInitializer(&window);
    *
    *
    */

    // Swap Chain
    /*swapChainBuilder.create(vulkanInitializer, window);
    swapChainBuilder.createImageViews(m_logical_device, imageViewBuilder);
    swapChainBuilder.assign(&m_swapChain, &m_swapChainImageFormat, &m_swapChainExtent, &m_swapChainImageViews);
    */

    /*
    * we need a VkSwapchainKHR m_swapChain;
    */
public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    // These getters return pointers to the VulkanBridge's private members
    // INSTANCES
    Initializer* vulkanInitializer;
    //Window window;
    SwapChain* swapChainBuilder;
    ImageView* imageViewBuilder;

    VkSurfaceKHR m_surface;
    VkPhysicalDevice m_physicalDevice;
    VkDevice m_logicalDevice;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    VkSwapchainKHR m_swapChain;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;

    VkRenderPass m_renderPass;

    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    VkPipelineLayout m_pipelineLayout;

    VkPipeline m_graphicsPipeline;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    VkCommandPool m_commandPool;

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;

    VkBuffer m_vertexBuffer;
    VkDeviceMemory m_vertexBufferMemory;

    VkBuffer m_indexBuffer;
    VkDeviceMemory m_indexBufferMemory;

    VkImage m_textureImage;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;
    VkDeviceMemory m_textureImageMemory;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VkDeviceMemory> m_uniformBuffersMemory;
    std::vector<void*> m_uniformBuffersMapped;

    VkImage m_depthImage;
    VkDeviceMemory m_depthImageMemory;
    VkImageView m_depthImageView;

    // Frames in flight require their own command buffers, semaphores and fences
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame = 0;

    Window window;
    uint32_t WIDTH = 800;
    uint32_t HEIGHT = 600;
    std::string MODEL_PATH = "Models/VikingRoom/VikingRoom.obj";
    std::string TEXTURE_PATH = "Models/VikingRoom/Textures/VikingRoom.png";
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    int MAX_FRAMES_IN_FLIGHT = 2;


    void Construct();

    void renderLoop();
private:

    /*
    * Window is polled and frames are drawn until window is closed
    */
    

    void createInstance();
    void recreateSwapChain();

    // RENDER PASSES
    void createRenderPass();

    void createDescriptorSetLayout();

    // GRAPHICS PIPELINE
    /*
    *   Create a graphics pipeline to render
    *
    */
    void createGraphicsPipeline();

    // SWAPCHAIN FRAME BUFFERS
    void createFramebuffers();

    // COMMAND POOL
    /*
        Command pools manage the memory used to store command buffers
        Allows for multithreaded command recording since all commands are available together in the buffers
    */
    void createCommandPool();

    void createDepthResources();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    VkFormat findDepthFormat();

    // TEXTURE IMAGE
    void createTextureImage();

    // TEXTURE IMAGE VIEW
    void createTextureImageView();

    // TEXTURE SAMPLER
    void createTextureSampler();


    // helper function used in createTextureImage();
    void createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory) {

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;

        // Be sure to match the image format with pixel format in the buffer otherwise copy operation will fail
        imageInfo.format = format;

        imageInfo.tiling = tiling;

        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.flags = 0; // Optional

        if (vkCreateImage(m_logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(m_logicalDevice, image, imageMemory, 0);
    }

    // TODO: Figure out what the fuck this does
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);


    void loadModel();

    // VERTEX BUFFER
    void createVertexBuffer();

    // INDEX BUFFER
    void createIndexBuffer();

    // UNIFORM BUFFERS
    void createUniformBuffers();

    // DESCRIPTOR POOL
    void createDescriptorPool();

    // DESCRIPTOR SETS
    void createDescriptorSets();

    // Buffer creation helper
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    // beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
    VkCommandBuffer beginSingleTimeCommands();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    // COPY BUFFER
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    // TODO: Move this to the Texture class as well as the Image class if I end up creating that (the Image class)
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // COMMAND BUFFER
    void createCommandBuffers();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        /*
            If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
            It's not possible to append commands to a buffer at a later time.
        */
        VkCommandBufferBeginInfo beginInfo{};
        /*
            VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
            VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : This is a secondary command buffer that will be entirely within a single render pass.
            VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : The command buffer can be resubmitted while it is also already pending execution.
        */
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        // DEBUG
        //printf("imageIndex DEBUG: %d\n", imageIndex);
        renderPassInfo.framebuffer = m_swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChainExtent;

        // VK_ATTACHMENT_LOAD_OP_CLEAR clear values for color and depth stencil
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();


        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Binding the graphics pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

        // We set the viewport and scissor state as dynamic in the pipeline 
        // We need to set those up in the command buffer now  
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_swapChainExtent.width);
        viewport.height = static_cast<float>(m_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { m_vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1,
            &m_descriptorSets[m_currentFrame], 0, nullptr);

        /*
            vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance)
            vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
            instanceCount: Used for instanced rendering, use 1 if you're not doing that.
            firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
            firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
        */
        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

    }

    // SEMAPHORES AND FENCES
    // Creates semaphores and fences for each frame in flight
    void createSyncObjects();

    // DRAW FRAME
    void drawFrame();

    void updateUniformBuffer(uint32_t currentImage);

    // SHADER MODULES
    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    // Reading in SPIRV shaders
    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

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

    void destroySwapChain();

    // CLEANUP
    void Destroy();
};