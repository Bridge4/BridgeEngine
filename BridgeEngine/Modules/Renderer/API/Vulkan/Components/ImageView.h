#pragma once 
#include "vulkan/vulkan.h"

class ImageView
{
public: 
    VkImageView create(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
};

