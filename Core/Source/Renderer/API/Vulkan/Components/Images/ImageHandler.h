#pragma once
#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/Components/Devices/DeviceHandler.h"
#include "Source/Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.h"
#include "vulkan/vulkan.h"
// class VulkanContext;

namespace backend_vk::images {

VkImageView create_image_view(VkDevice device, VkImage image, VkFormat format,
                              VkImageAspectFlags aspectFlags,
                              bool is_cube_map = false);

void transition_image_layout(VkDevice& logical_device,
                             VkCommandPool& command_pool,
                             VkQueue& graphics_queue, VkImage image,
                             VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout);

void create_image(VkPhysicalDevice& physical_device, VkDevice& logical_device,
                  uint32_t width, uint32_t height, VkFormat format,
                  VkImageTiling tiling, VkImageUsageFlags usage,
                  VkMemoryPropertyFlags properties, VkImage& image,
                  VkDeviceMemory& imageMemory, bool is_cube_map = false);

bool has_stencil_component(VkFormat format);

void copy_buffer_to_image(VkDevice& logical_device, VkCommandPool& command_pool,
                          VkQueue& graphics_queue, VkBuffer buffer,
                          VkImage image, uint32_t width, uint32_t height,
                          VkDeviceSize layer_size = 0, int layer_count = 1);
}  // namespace backend_vk::images
