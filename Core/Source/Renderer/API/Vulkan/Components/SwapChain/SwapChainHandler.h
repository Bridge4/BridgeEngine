#ifndef SWAPCHAINHANDLER_H
#define SWAPCHAINHANDLER_H

#include <vulkan/vulkan.h>

#include <vector>

#include "../ComponentDeclarations.h"

class SwapChainHandler {
   public:
    SwapChainHandler(VulkanContext* context, DeviceHandler* deviceHandler,
                     WindowHandler* targetWindow, ImageHandler* imgViewHandler,
                     VulkanInstanceManager* vulkanInstanceManager) {
        this->m_vulkanContext = context;
        this->m_deviceHandler = deviceHandler;
        this->m_windowHandler = targetWindow;
        this->m_imageViewHandler = imgViewHandler;
        this->m_vulkanGlobalState = vulkanInstanceManager;
    };

    void Initialize();

    void Rebuild();

    void Destroy();

    VkSwapchainKHR m_swapChain = nullptr;
    VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapChainExtent = {};
    std::vector<VkImageView> m_swapChainImageViews = {};

   private:
    VulkanContext* m_vulkanContext = 0;
    VulkanInstanceManager* m_vulkanGlobalState = 0;
    DeviceHandler* m_deviceHandler = 0;
    WindowHandler* m_windowHandler = 0;
    RenderPassHandler* m_renderPassHandler = 0;
    ImageHandler* m_imageViewHandler = 0;

    // HELPERS
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(
        const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR ChooseSwapPresentMode(
        const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

#endif  // !SWAPCHAINHANDLER_H
