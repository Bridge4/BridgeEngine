#pragma once
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <vector>
#include "DataStructures.h"
#include "Includes/Window.h"
#include "Includes/Initializer.h"
#include "Includes/ImageView.h"
#include "Includes/SwapChain.h"

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
    *   init.init(&window);
    *
    *
    */

    // Swap Chain
    /*swapChain.create(init, window);
    swapChain.createImageViews(m_logical_device, imageView);
    swapChain.assign(&m_swapChain, &m_swapChainImageFormat, &m_swapChainExtent, &m_swapChainImageViews);
    */

    /*
    * we need a VkSwapchainKHR m_swapChain;
    */
public:
    // These getters return pointers to the VulkanBridge's private members
    // INSTANCES
    Initializer init;
    //Window window;
    SwapChain swapChain;
    ImageView imageView;

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

};