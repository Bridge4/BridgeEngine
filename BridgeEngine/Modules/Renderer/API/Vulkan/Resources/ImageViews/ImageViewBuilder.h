#pragma once 
#include "vulkan/vulkan.h"

class ImageViewBuilder
{
public: 
    VkImageView Build(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

