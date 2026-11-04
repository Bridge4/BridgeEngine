#pragma once
#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/Components/Devices/DeviceHandler.h"
#include "Source/Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.h"
#include "vulkan/vulkan.h"
// class VulkanContext;

namespace backend_vk::images {

VkImageView create_image_view(VkDevice device, VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags);

void transition_image_layout(VkDevice& logical_device,
                             VkCommandPool& command_pool,
                             VkQueue& graphics_queue, VkImage image,
                             VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout);

void create_image(VkPhysicalDevice& physical_device, VkDevice& logical_device,
                  uint32_t width, uint32_t height, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkImage& image,
                  VkDeviceMemory& imageMemory);

bool has_stencil_component(VkFormat format);
}  // namespace backend_vk::images

class ImageHandler {
   public:
    ImageHandler(VulkanContext* vulkanContext,
                 VulkanGlobalState* vulkanGlobalState,
                 DeviceHandler* deviceHandler) {
        this->m_vulkanContext = vulkanContext;
        this->m_vulkanGlobalState = vulkanGlobalState;
        this->m_deviceHandler = deviceHandler;
    }

    bool HasStencilComponent(VkFormat format);

   private:
    VulkanContext* m_vulkanContext;
    VulkanGlobalState* m_vulkanGlobalState;
    DeviceHandler* m_deviceHandler;
};
