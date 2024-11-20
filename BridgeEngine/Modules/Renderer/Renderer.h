#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
// To meet Vulkan data alignment specs
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>


#include <chrono>
#include <iostream>
#include <array>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <optional>
#include <set>
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Includes/Window.h"
#include "VulkanBridge.h"




/*
*   For now, this Renderer is actually the Vulkan Instance class that will be placed in Modules/Renderer/API/Vulkan
*/
class Renderer {
public:
    
    void run() {
        //window.createWindow();
        initAPI();
        renderLoop();
        cleanup();
    }

    Window window;    
    uint32_t WIDTH = 800;
    uint32_t HEIGHT = 600;
    std::string MODEL_PATH = "Models/VikingRoom/VikingRoom.obj";
    std::string TEXTURE_PATH = "Models/VikingRoom/Textures/VikingRoom.png";

    int MAX_FRAMES_IN_FLIGHT = 2;

private:
    VulkanBridge vulkanBridge;

    void initAPI() {

        // Initialization
        vulkanBridge.init.init(&window);
        //init.assign(&vulkanBridge.m_surface, &vulkanBridge.m_physicalDevice, &vulkanBridge.m_logicalDevice, &vulkanBridge.m_graphicsQueue, &vulkanBridge.m_presentQueue);
        vulkanBridge.init.assign(&vulkanBridge.m_surface, &vulkanBridge.m_physicalDevice, &vulkanBridge.m_logicalDevice, &vulkanBridge.m_graphicsQueue, &vulkanBridge.m_presentQueue);
        // Swap Chain
        //swapChain.Construct(&vulkanBridge);
        vulkanBridge.swapChain.create(vulkanBridge.init, window);
        vulkanBridge.swapChain.createImageViews(vulkanBridge.m_logicalDevice, vulkanBridge.imageView);
        vulkanBridge.swapChain.assign(&vulkanBridge.m_swapChain, &vulkanBridge.m_swapChainImageFormat, &vulkanBridge.m_swapChainExtent, &vulkanBridge.m_swapChainImageViews);

        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();

        createDepthResources();
        createFramebuffers();

        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        
        loadModel();
        
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();

        createDescriptorPool();
        createDescriptorSets();

        createCommandBuffers();
        createSyncObjects();
    }
    
    /*
    * Window is polled and frames are drawn until window is closed
    */
    void renderLoop() {
        while (!window.shouldClose()) {
            window.poll();
            drawFrame();
        }
        vkDeviceWaitIdle(vulkanBridge.m_logicalDevice);
    }


    void recreateSwapChain() {
        // Handling minimization
        window.handleMinimization();

        vkDeviceWaitIdle(vulkanBridge.m_logicalDevice);

        cleanupSwapChain();

        vulkanBridge.swapChain.create(vulkanBridge.init, window);
        vulkanBridge.swapChain.createImageViews(vulkanBridge.m_logicalDevice, vulkanBridge.imageView);
        vulkanBridge.swapChain.assign(&vulkanBridge.m_swapChain, &vulkanBridge.m_swapChainImageFormat, &vulkanBridge.m_swapChainExtent, &vulkanBridge.m_swapChainImageViews);

        createDepthResources();
        createFramebuffers();
    }


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

        if (vkCreateImage(vulkanBridge.m_logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vulkanBridge.m_logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(vulkanBridge.m_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(vulkanBridge.m_logicalDevice, image, imageMemory, 0);
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
        renderPassInfo.renderPass = vulkanBridge.m_renderPass;
        // DEBUG
        //printf("imageIndex DEBUG: %d\n", imageIndex);
        renderPassInfo.framebuffer = vulkanBridge.m_swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = vulkanBridge.m_swapChainExtent;

        // VK_ATTACHMENT_LOAD_OP_CLEAR clear values for color and depth stencil
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();


        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Binding the graphics pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanBridge.m_graphicsPipeline);

        // We set the viewport and scissor state as dynamic in the pipeline 
        // We need to set those up in the command buffer now  
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vulkanBridge.m_swapChainExtent.width);
        viewport.height = static_cast<float>(vulkanBridge.m_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = vulkanBridge.m_swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { vulkanBridge.m_vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, vulkanBridge.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanBridge.m_pipelineLayout, 0, 1,
            &vulkanBridge.m_descriptorSets[vulkanBridge.m_currentFrame], 0, nullptr);

        /*
            vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance)
            vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
            instanceCount: Used for instanced rendering, use 1 if you're not doing that.
            firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
            firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
        */
        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vulkanBridge.m_indices.size()), 1, 0, 0, 0);
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

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkanBridge.m_swapChainExtent.width / (float)vulkanBridge.m_swapChainExtent.height, 0.1f, 10.0f);
        // IMPORTANT: VULKAN HAS INVERTED Y AXIS TO OPENGL AND GLM WAS DESIGNED FOR OPENGL. THIS CONVERTS TO VULKAN.
        ubo.proj[1][1] *= -1;

        memcpy(vulkanBridge.m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    // SHADER MODULES
    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(vulkanBridge.m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
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

    void cleanupSwapChain() {
        vkDestroyImageView(vulkanBridge.m_logicalDevice, vulkanBridge.m_depthImageView, nullptr);
        vkDestroyImage(vulkanBridge.m_logicalDevice, vulkanBridge.m_depthImage, nullptr);
        vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_depthImageMemory, nullptr);

        for (auto framebuffer : vulkanBridge.m_swapChainFramebuffers) {
            vkDestroyFramebuffer(vulkanBridge.m_logicalDevice, framebuffer, nullptr);
        }

        for (auto imageView : vulkanBridge.m_swapChainImageViews) {
            vkDestroyImageView(vulkanBridge.m_logicalDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(vulkanBridge.m_logicalDevice, vulkanBridge.m_swapChain, nullptr);
    }

    // CLEANUP
    void cleanup() {
        cleanupSwapChain();

        vkDestroySampler(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureSampler, nullptr);
        vkDestroyImageView(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureImageView, nullptr);
        vkDestroyImage(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureImage, nullptr);
        vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureImageMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(vulkanBridge.m_logicalDevice, vulkanBridge.m_uniformBuffers[i], nullptr);
            vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(vulkanBridge.m_logicalDevice, vulkanBridge.m_descriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(vulkanBridge.m_logicalDevice, vulkanBridge.m_descriptorSetLayout, nullptr);

        // Buffer cleanup
        vkDestroyBuffer(vulkanBridge.m_logicalDevice, vulkanBridge.m_indexBuffer, nullptr);
        vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_indexBufferMemory, nullptr);
        vkDestroyBuffer(vulkanBridge.m_logicalDevice, vulkanBridge.m_vertexBuffer, nullptr);
        vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_vertexBufferMemory, nullptr);

        vkDestroyPipeline(vulkanBridge.m_logicalDevice, vulkanBridge.m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(vulkanBridge.m_logicalDevice, vulkanBridge.m_pipelineLayout, nullptr);

        vkDestroyRenderPass(vulkanBridge.m_logicalDevice, vulkanBridge.m_renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(vulkanBridge.m_logicalDevice, vulkanBridge.m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanBridge.m_logicalDevice, vulkanBridge.m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(vulkanBridge.m_logicalDevice, vulkanBridge.m_inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(vulkanBridge.m_logicalDevice, vulkanBridge.m_commandPool, nullptr);

        // DEVICE DESTRUCTION
        vulkanBridge.init.destroy();
        // GLFW DESTRUCTION
        window.destroy();
    }
};
