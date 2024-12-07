#ifndef VULKANINSTANCEMANAGER_H
#define VULKANINSTANCEMANAGER_H
#include "vulkan/vulkan.h"
#include "../ComponentDeclarations.h"
#include <vector>

class VulkanInstanceManager {
public:
	// The VulkanInstanceManager will hold the resources created by the various handlers 	
	// The goal is to move instance creation to this class, 
	// Move member variables to this class, and establish a new naming standard: m_member_name <-- I'm going with the underscores because I use too much camelcase at wor	// Move member variables to this class, and establish a new naming standard: m_member_name <-- I'm going with the underscores because I use too much camelcase at workk
	
	
	VulkanInstanceManager(VulkanContext* vulkanContext) {
		this->m_vulkanContext = vulkanContext;
	}



	void CreateInstance();
    // Getters
    VkPhysicalDevice GetPhysicalDevice() { return m_physicalDevice; }
    // Setters
    void SetPhysicalDevice(VkPhysicalDevice physicalDevice) { this->m_physicalDevice = physicalDevice; }

    VkDevice* GetRefLogicalDevice() { return &m_logicalDevice; }


	VkInstance m_vulkanInstance = 0;
	VulkanContext* m_vulkanContext = 0;

    VkPhysicalDevice m_physicalDevice = nullptr;
    VkDevice m_logicalDevice = nullptr;

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
