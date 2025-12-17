#pragma once
#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/Components/VulkanInstanceManager/VulkanInstanceManager.h"
#include "vulkan/vulkan.h"
// class VulkanContext;
class ImageHandler {
   public:
    ImageHandler(VulkanContext* vulkanContext,
                 VulkanInstanceManager* vulkanGlobalState) {
        this->m_vulkanContext = vulkanContext;
        this->m_vulkanGlobalState = vulkanGlobalState;
    }

    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format,
                                VkImageAspectFlags aspectFlags);

    void TransitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    void CreateImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& imageMemory);
    uint32_t FindMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    bool HasStencilComponent(VkFormat format);

   private:
    VulkanContext* m_vulkanContext;
    VulkanInstanceManager* m_vulkanGlobalState;
};
