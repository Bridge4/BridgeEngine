#pragma once 
#include "vulkan/vulkan.h"
#include "../ComponentDeclarations.h"
//class VulkanContext;
class ImageHandler
{
public: 
    ImageHandler(VulkanContext* vulkanContext) {
        this->vulkanContext = vulkanContext;
    }

    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
private:
    VulkanContext* vulkanContext;

};

