#pragma once 
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif
#include "vulkan/vulkan.h"
class ImageView
{
public: 
    VkImageView create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

