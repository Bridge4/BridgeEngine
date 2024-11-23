#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;
class DeviceHandler;
class ImageViewBuilder;
class Window;


class SwapChainHandler
{
public: 
    SwapChainHandler(VulkanContext* context, DeviceHandler* dHandler, Window* targetWindow, ImageViewBuilder* imgViewBldr) 
    {
        vulkanContext = context;
        devices = dHandler;
        windowRef = targetWindow;
        imageViewBuilder = imgViewBldr;
    };
    
    void Initialize();

    VkSwapchainKHR SwapChain = nullptr;
    VkFormat SwapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D SwapChainExtent = {};
    std::vector<VkImageView> SwapChainImageViews = {};
    
private:
    VulkanContext* vulkanContext = 0;
    DeviceHandler* devices = 0;
    Window* windowRef = 0;
    ImageViewBuilder* imageViewBuilder = 0;

    VkSwapchainKHR m_swapChain = nullptr;
    VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapChainExtent = {};
    std::vector<VkImageView> m_swapChainImageViews = {};
    std::vector<VkImage> m_swapChainImages = {};

    
    // HELPERS
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

