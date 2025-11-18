#ifndef VULKANINSTANCEMANAGER_H
#define VULKANINSTANCEMANAGER_H
#include "vulkan/vulkan.h"
#include "../ComponentDeclarations.h"
#include <vector>

class VulkanInstanceManager {
public:
    // The VulkanInstanceManager will hold the resources created by the various handlers 	
    // The goal is to move instance creation to this class, 

    VulkanInstanceManager(VulkanContext* vulkanContext) {
        this->m_vulkanContext = vulkanContext;
    }



    void CreateInstance();

    // Getters
    VkInstance GetVulkanInstance() { return m_instance; }
    VkPhysicalDevice GetPhysicalDevice() { return m_physicalDevice; }
    //std::vector<VkImage>* GetRefSwapChainImages() { return &m_swapChainImages; }
    std::vector<VkImageView>* GetRefSwapChainImageViews() { return &m_swapChainImageViews; }
    VkFormat GetSwapChainImageFormat() { return m_swapChainImageFormat; }
    VkDevice* GetRefLogicalDevice() { return &m_logicalDevice; }
    VkSwapchainKHR* GetRefSwapChain() { return &m_swapChain; }
    std::vector<VkImage>* GetRefSwapChainImages() { return &m_swapChainImages; }
    VkRenderPass* GetRefRenderPass() { return &m_renderPass; }
    VkExtent2D GetSwapChainExtent() { return m_swapChainExtent; }
    VkSurfaceKHR GetSurface() { return m_surface; }
    VkImageView GetDepthImageView() { return m_depthImageView; }

    // Setters
    void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { this->m_physicalDevice = physicalDevice; }
    void SetSwapChainImageFormat(VkFormat surfaceFormat) { this->m_swapChainImageFormat = surfaceFormat; };
    void SetSwapChainExtent(VkExtent2D extent) { this->m_swapChainExtent = extent; }
    void SetSwapChainImageViews(std::vector<VkImageView> swapChainImageViews) { this->m_swapChainImageViews = swapChainImageViews; }
    void SetSwapChainImages(std::vector<VkImage> swapChainImages) { this->m_swapChainImages = swapChainImages; }

	//VkInstance m_vulkanInstance = 0;
	VulkanContext* m_vulkanContext = 0;

    VkPhysicalDevice m_physicalDevice = nullptr;
    VkDevice m_logicalDevice = nullptr;

    VkSwapchainKHR m_swapChain = nullptr;
    std::vector<VkImage> m_swapChainImages = {};
    VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapChainExtent;

    VkImage m_depthImage = 0;
    VkDeviceMemory m_depthImageMemory = 0;
    VkImageView m_depthImageView = 0;
    std::vector<VkImageView> m_swapChainImageViews = {};

    VkRenderPass m_renderPass; 

    VkSurfaceKHR m_surface;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;

    VkDescriptorSetLayout m_descriptorSetLayout;
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;

    VkPipelineLayout m_pipelineLayout;

    VkPipeline m_graphicsPipeline;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    VkCommandPool m_commandPool;

    VkImage m_textureImage;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;
    VkDeviceMemory m_textureImageMemory;


    VkImage m_textureImage2;
    VkImageView m_textureImageView2;
    VkSampler m_textureSampler2;
    VkDeviceMemory m_textureImageMemory2;

	std::vector<VkBuffer> m_uniformBuffers = {};
	std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
	std::vector<void*> m_uniformBuffersMapped = {};


    // Frames in flight require their own command buffers, semaphores and fences
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    uint32_t m_currentFrame = 0;

    //Window window;
    uint32_t WIDTH = 800;
    uint32_t HEIGHT = 600;
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    int MAX_FRAMES_IN_FLIGHT = 2;

private:

};
#endif
