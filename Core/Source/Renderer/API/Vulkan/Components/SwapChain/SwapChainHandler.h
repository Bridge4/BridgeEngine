#ifndef SWAPCHAINHANDLER_H
#define SWAPCHAINHANDLER_H

#include <vulkan/vulkan.h>

#include <vector>

#include "../ComponentDeclarations.h"

class SwapChainHandler {
   public:
    SwapChainHandler(VulkanContext* context, DeviceHandler* deviceHandler,
                     ImageHandler* imgViewHandler,
                     VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkanContext = context;
        this->m_deviceHandler = deviceHandler;
        this->m_imageViewHandler = imgViewHandler;
        this->m_vulkanGlobalState = VulkanGlobalState;
    };

    void Initialize(int frameBufferWidth, int frameBufferHeight);

    void Rebuild(int frameBufferWidth, int frameBufferHeight);

    void Destroy();

    VkSwapchainKHR m_swapChain = nullptr;
    VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapChainExtent = {};
    std::vector<VkImageView> m_swapChainImageViews = {};
    RenderPassHandler* m_renderPassHandler = 0;

   private:
    VulkanContext* m_vulkanContext = 0;
    VulkanGlobalState* m_vulkanGlobalState = 0;
    DeviceHandler* m_deviceHandler = 0;
    ImageHandler* m_imageViewHandler = 0;

    // HELPERS
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR ChooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D ChooseSwapExtent(int frameBufferWidth, int frameBufferHeight,
                                const VkSurfaceCapabilitiesKHR& capabilities);
};

#endif  // !SWAPCHAINHANDLER_H
