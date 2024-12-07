#include "../Images/ImageHandler.h"
#include <stdexcept>


/*
* WAS MOVING CREATE IMAGE CODE TO IMAGEHANDLER CLASS
* Move CreateTextureImage() to this class
*/

VkImageView ImageHandler::CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    return imageView;
}

//uint32_t BufferHandler::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
//{
//    VkPhysicalDeviceMemoryProperties memProperties;
//    vkGetPhysicalDeviceMemoryProperties(deviceHandler->PhysicalDevice, &memProperties);
//
//    // Go over this section
//    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//            return i;
//        }
//    }
//
//    throw std::runtime_error("Failed to find suitable memory type");
//}

//void ImageHandler::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
//{
//    VkImageCreateInfo imageInfo{};
//    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//    imageInfo.imageType = VK_IMAGE_TYPE_2D;
//    imageInfo.extent.width = static_cast<uint32_t>(width);
//    imageInfo.extent.height = static_cast<uint32_t>(height);
//    imageInfo.extent.depth = 1;
//    imageInfo.mipLevels = 1;
//    imageInfo.arrayLayers = 1;
//
//    // Be sure to match the image format with pixel format in the buffer otherwise copy operation will fail
//    imageInfo.format = format;
//    imageInfo.tiling = tiling;
//    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    imageInfo.usage = usage;
//    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//    imageInfo.flags = 0; // Optional
//
//    if (vkCreateImage(*m_vulkanInstanceManager->GetRefLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create image!");
//    }
//
//    VkMemoryRequirements memRequirements;
//    vkGetImageMemoryRequirements(*m_vulkanInstanceManager->GetRefLogicalDevice(), image, &memRequirements);
//
//    VkMemoryAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    allocInfo.allocationSize = memRequirements.size;
//    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
//
//    if (vkAllocateMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
//        throw std::runtime_error("failed to allocate image memory!");
//    }
//
//    vkBindImageMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), image, imageMemory, 0);
//}