#pragma once
#include <vulkan/vulkan.h>
#include <vector>

class VulkanBridge;
class Initializer;
class ImageView;
class Window;


class SwapChainBuilder
{
public: 
    SwapChainBuilder(VulkanBridge* context, Initializer* initializer, Window* targetWindow, ImageView* imgView) 
    {
        vulkanContext = context;
        vulkanInitializer = initializer;
        windowRef = targetWindow;
        imageView = imgView;
    };
    
    void Build();


    VulkanBridge* vulkanContext = 0;
    Initializer* vulkanInitializer = 0;
    Window* windowRef = 0;
    ImageView* imageView = 0;
private:

    // HELPERS
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
};

