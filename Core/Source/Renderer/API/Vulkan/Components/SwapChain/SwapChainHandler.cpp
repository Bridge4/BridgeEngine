#pragma once
#include "SwapChainHandler.h"
#include "../Images/ImageHandler.h"
#include "../Devices/DeviceHandler.h"
#include "../Window/WindowHandler.h"
#include "../RenderPass/RenderPassHandler.h"
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include "../../VulkanContext.h"

void SwapChainHandler::Initialize() {
    /*VkSwapchainKHR m_swapChain;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainImageFormat;
    VkExtent2D  m_swapChainExtent;*/

    VkSurfaceKHR surface = m_vulkanInstanceManager->GetSurface();

    SwapChainSupportDetails swapChainSupport = deviceHandler->querySwapChainSupport(m_vulkanInstanceManager->GetPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    // the reason we do +1 is because we will have to wait for the driver to complete internal operations before acquiring another image.
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
                                                
    // Populating swapchain struct with necessary info we just queried for 
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = deviceHandler->findQueueFamilies(m_vulkanInstanceManager->GetPhysicalDevice());
    uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(), indices.presentFamily.value() };

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0; // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(*m_vulkanInstanceManager->GetRefLogicalDevice(), &createInfo, nullptr, m_vulkanInstanceManager->GetRefSwapChain()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(*m_vulkanInstanceManager->GetRefLogicalDevice(), *m_vulkanInstanceManager->GetRefSwapChain(), &imageCount, nullptr);
    std::vector<VkImage> swapChainImages = *m_vulkanInstanceManager->GetRefSwapChainImages();
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(*m_vulkanInstanceManager->GetRefLogicalDevice(), *m_vulkanInstanceManager->GetRefSwapChain() ,&imageCount,swapChainImages.data());
    m_vulkanInstanceManager->SetSwapChainImages(swapChainImages);

    m_vulkanInstanceManager->SetSwapChainImageFormat(surfaceFormat.format);
    m_vulkanInstanceManager->SetSwapChainExtent(extent);
    std::vector<VkImageView> swapChainImageViews = *m_vulkanInstanceManager->GetRefSwapChainImageViews();
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        // Moved definition for createImageView() to a class and turned into a static function
        swapChainImageViews[i] = imageViewBuilder->CreateImageView(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_swapChainImages[i], m_vulkanInstanceManager->m_swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
        
    }
    m_vulkanInstanceManager->SetSwapChainImageViews(swapChainImageViews);

    /*SwapChain = SwapChain;
    SwapChainImageFormat = SwapChainImageFormat;
    SwapChainExtent = SwapChainExtent;
    SwapChainImageViews = SwapChainImageViews;*/
    //CreateFramebuffers();
}

void SwapChainHandler::Rebuild() {
    // Handling minimization
    windowHandler->handleMinimization();

    vkDeviceWaitIdle(*m_vulkanInstanceManager->GetRefLogicalDevice());

    Destroy();

    Initialize();

    CreateDepthResources();
    CreateFramebuffers();
}

void SwapChainHandler::Destroy() {
    vkDestroyImageView(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_depthImageView, nullptr);
    vkDestroyImage(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_depthImage, nullptr);
    vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_depthImageMemory, nullptr);

    for (auto framebuffer : m_vulkanInstanceManager->m_swapChainFramebuffers) {
        vkDestroyFramebuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), framebuffer, nullptr);
    }

    for (auto imageView : m_vulkanInstanceManager->m_swapChainImageViews) {
        vkDestroyImageView(*m_vulkanInstanceManager->GetRefLogicalDevice(), imageView, nullptr);
    }

    vkDestroySwapchainKHR(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_swapChain, nullptr);
}

void SwapChainHandler::CreateDepthResources() {
    VkFormat depthFormat = FindSupportedFormat(
        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
    CreateImage(m_vulkanInstanceManager->GetSwapChainExtent().width, m_vulkanInstanceManager->GetSwapChainExtent().height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_vulkanInstanceManager->m_depthImage, m_vulkanInstanceManager->m_depthImageMemory);
    m_vulkanInstanceManager->m_depthImageView = imageViewBuilder->CreateImageView(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    TransitionImageLayout(m_vulkanInstanceManager->m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void SwapChainHandler::CreateFramebuffers() {
    CreateDepthResources();
    m_vulkanInstanceManager->m_swapChainFramebuffers.resize(m_vulkanInstanceManager->m_swapChainImageViews.size());

    // Loop through swap chain image views
    for (size_t i = 0; i < m_vulkanInstanceManager->m_swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            m_vulkanInstanceManager->m_swapChainImageViews[i],
            m_vulkanInstanceManager->m_depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_vulkanInstanceManager->m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_vulkanInstanceManager->GetSwapChainExtent().width;
        framebufferInfo.height = m_vulkanInstanceManager->GetSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), &framebufferInfo, nullptr, &m_vulkanInstanceManager->m_swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}


// Helpers
void SwapChainHandler::CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(width);
    imageInfo.extent.height = static_cast<uint32_t>(height);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    // Be sure to match the image format with pixel format in the buffer otherwise copy operation will fail
    imageInfo.format = format;

    imageInfo.tiling = tiling;

    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfo.usage = usage;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.flags = 0; // Optional

    if (vkCreateImage(*m_vulkanInstanceManager->GetRefLogicalDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(*m_vulkanInstanceManager->GetRefLogicalDevice(), image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), image, imageMemory, 0);
}

VkFormat SwapChainHandler::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_vulkanInstanceManager->GetPhysicalDevice(), format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

uint32_t SwapChainHandler::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vulkanInstanceManager->GetPhysicalDevice(), &memProperties);

    // Go over this section
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}


void SwapChainHandler::TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = 0;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    // Syncronization 
    vkCmdPipelineBarrier(
        commandBuffer,
        sourceStage, destinationStage,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );
    EndSingleTimeCommands(commandBuffer);
}
//void SwapChainHandler::CreateFramebuffers() {
//    SwapChainFramebuffers.resize(SwapChainImageViews.size());
//
//    // Loop through swap chain image views
//    for (size_t i = 0; i < SwapChainImageViews.size(); i++) {
//        std::array<VkImageView, 2> attachments = {
//            SwapChainImageViews[i],
//            m_depthImageView
//        };
//
//        VkFramebufferCreateInfo framebufferInfo{};
//        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        framebufferInfo.renderPass = renderPassHandler->renderPass;
//        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//        framebufferInfo.pAttachments = attachments.data();
//        framebufferInfo.pAttachments = attachments.data();
//        framebufferInfo.width = SwapChainExtent.width;
//        framebufferInfo.height = SwapChainExtent.height;
//        framebufferInfo.layers = 1;
//
//        if (vkCreateFramebuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), &framebufferInfo, nullptr, &SwapChainFramebuffers[i]) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create framebuffer!");
//        }
//    }
//}

VkCommandBuffer SwapChainHandler::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_vulkanInstanceManager->m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}


void SwapChainHandler::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vulkanInstanceManager->m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkanInstanceManager->m_graphicsQueue);

    vkFreeCommandBuffers(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_commandPool, 1, &commandBuffer);
}

// HELPERS
VkSurfaceFormatKHR SwapChainHandler::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChainHandler::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChainHandler::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    //Fun fact: wrapping a function call with parentheses can prevent macro expansions
    if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        windowHandler->getFramebufferSize(&width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
