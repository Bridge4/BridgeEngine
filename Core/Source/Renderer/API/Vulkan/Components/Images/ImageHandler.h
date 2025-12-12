#pragma once
#include "../ComponentDeclarations.h"
#include "vulkan/vulkan.h"
// class VulkanContext;
class ImageHandler {
   public:
    ImageHandler(VulkanContext* vulkanContext,
                 VulkanInstanceManager* vulkanGlobalState) {
        this->vulkanContext = vulkanContext;
        this->m_vulkanGlobalState = vulkanGlobalState;
    }

    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format,
                                VkImageAspectFlags aspectFlags);

    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    void CreateImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& imageMemory);

   private:
    VulkanContext* vulkanContext;
    VulkanInstanceManager* m_vulkanGlobalState;
};
