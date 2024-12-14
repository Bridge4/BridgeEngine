#ifndef SWAPCHAINHANDLER_H
#define SWAPCHAINHANDLER_H

#include <vulkan/vulkan.h>
#include <vector>
#include "../ComponentDeclarations.h"

class SwapChainHandler
{
public: 
    SwapChainHandler(VulkanContext* context, DeviceHandler* deviceHandler, WindowHandler* targetWindow, ImageHandler* imgViewBldr, VulkanInstanceManager* vulkanInstanceManager) 
    {
        this->vulkanContext = context;
        this->deviceHandler = deviceHandler;
        this->windowHandler = targetWindow;
        this->imageViewBuilder = imgViewBldr;
        this->m_vulkanInstanceManager = vulkanInstanceManager;
    };

    
    void Initialize();

    void Rebuild();

    void AttachRenderPassHandler(RenderPassHandler* renderPassHandler) {
        this->renderPassHandler = renderPassHandler;
    };

    void Destroy();

    void CreateDepthResources();


    void CreateFramebuffers();

    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);


    //void CreateFramebuffers();

    VkSwapchainKHR SwapChain = nullptr;
    VkFormat SwapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D SwapChainExtent = {};
    std::vector<VkImageView> SwapChainImageViews = {};
    std::vector<VkFramebuffer> SwapChainFramebuffers = {};

    VkSwapchainKHR m_swapChain = nullptr;
    VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapChainExtent = {};
    std::vector<VkImageView> m_swapChainImageViews = {};
    std::vector<VkImage> SwapChainImages = {};
    VkImage m_depthImage = 0;
    VkDeviceMemory m_depthImageMemory = 0;
    VkImageView m_depthImageView = 0;
private:
    VulkanContext* vulkanContext = 0;
    VulkanInstanceManager* m_vulkanInstanceManager = 0;
    DeviceHandler* deviceHandler = 0;
    WindowHandler* windowHandler = 0;
    RenderPassHandler* renderPassHandler = 0;
    ImageHandler* imageViewBuilder = 0;
    //BufferHandler* bufferHandler = 0;
    

    
    //VkImageView m_depthImageView = 0;


    
    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

    // HELPERS
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

};

#endif // !SWAPCHAINHANDLER_H
