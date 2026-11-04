#pragma once
#include "SwapChainHandler.h"

#include <vulkan/vulkan_core.h>

#include <algorithm>
#include <cstdlib>
#include <stdexcept>
#include <vector>

#include "../../VulkanContext.h"
#include "../Devices/DeviceHandler.h"
#include "../Images/ImageHandler.h"
#include "../RenderPass/RenderPassHandler.h"
#include "../VulkanGlobalState/VulkanGlobalState.h"
#include "../WindowVK/WindowHandlerVK.h"

void SwapChainHandler::init(int frameBufferWidth, int frameBufferHeight) {
    VkSurfaceKHR surface = m_vulkan_global_state->GetSurface();

    SwapChainSupportDetails swapChainSupport =
        backend_vk::devices::query_swap_chain_support(
            m_device_handler->m_physical_device,
            m_vulkan_global_state->m_surface);

    VkSurfaceFormatKHR surfaceFormat =
        ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode =
        ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(frameBufferWidth, frameBufferHeight,
                                         swapChainSupport.capabilities);

    // the reason we do +1 is because we will have to wait for the driver to
    // complete internal operations before acquiring another image.
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
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

    QueueFamilyIndices indices = backend_vk::devices::find_queue_families(
        m_device_handler->m_physical_device, m_vulkan_global_state->m_surface);

    uint32_t queueFamilyIndices[] = {indices.graphics_family.value(),
                                     indices.present_family.value()};

    if (indices.graphics_family != indices.present_family) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;      // Optional
        createInfo.pQueueFamilyIndices = nullptr;  // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;

    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(
            m_device_handler->m_logical_device, &createInfo, nullptr,
            m_vulkan_global_state->GetRefSwapChain()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(m_device_handler->m_logical_device,
                            *m_vulkan_global_state->GetRefSwapChain(),
                            &imageCount, nullptr);
    std::vector<VkImage> swapChainImages =
        *m_vulkan_global_state->GetRefSwapChainImages();
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_device_handler->m_logical_device,
                            *m_vulkan_global_state->GetRefSwapChain(),
                            &imageCount, swapChainImages.data());
    m_vulkan_global_state->SetSwapChainImages(swapChainImages);

    m_vulkan_global_state->SetSwapChainImageFormat(surfaceFormat.format);
    m_vulkan_global_state->SetSwapChainExtent(extent);
    std::vector<VkImageView> swapChainImageViews =
        *m_vulkan_global_state->GetRefSwapChainImageViews();
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        // Moved definition for createImageView() to a class and turned into a
        // static function
        swapChainImageViews[i] = backend_vk::images::create_image_view(
            m_device_handler->m_logical_device,
            m_vulkan_global_state->m_swapChainImages[i],
            m_vulkan_global_state->m_swapChainImageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }
    m_vulkan_global_state->SetSwapChainImageViews(swapChainImageViews);
}

void SwapChainHandler::Rebuild(int frameBufferWidth, int frameBufferHeight) {
    // Handling minimization
    // m_windowHandler->PollWindowNotMinimized();
    // vkDeviceWaitIdle(m_deviceHandler->LogicalDevice);
    Destroy();
    init(frameBufferWidth, frameBufferHeight);
}

void SwapChainHandler::Destroy() {
    vkDestroyImageView(
        m_device_handler->m_logical_device,
        m_vulkan_global_state->m_render_pass_data->depth_resources.image_view,
        nullptr);
    vkDestroyImage(
        m_device_handler->m_logical_device,
        m_vulkan_global_state->m_render_pass_data->depth_resources.image,
        nullptr);
    vkFreeMemory(
        m_device_handler->m_logical_device,
        m_vulkan_global_state->m_render_pass_data->depth_resources.image_memory,
        nullptr);

    for (auto framebuffer :
         m_vulkan_global_state->m_render_pass_data->framebuffers) {
        vkDestroyFramebuffer(m_device_handler->m_logical_device, framebuffer,
                             nullptr);
    }

    for (auto imageView : m_vulkan_global_state->m_swapChainImageViews) {
        vkDestroyImageView(m_device_handler->m_logical_device, imageView,
                           nullptr);
    }

    vkDestroySwapchainKHR(m_device_handler->m_logical_device,
                          m_vulkan_global_state->m_swapChain, nullptr);
}

// HELPERS
VkSurfaceFormatKHR SwapChainHandler::ChooseSwapSurfaceFormat(
    const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR SwapChainHandler::ChooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChainHandler::ChooseSwapExtent(
    int frameBufferWidth, int frameBufferHeight,
    const VkSurfaceCapabilitiesKHR& capabilities) {
    // Fun fact: wrapping a function call with parentheses can prevent macro
    // expansions
    if (capabilities.currentExtent.width !=
        (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    }

    VkExtent2D actualExtent = {static_cast<uint32_t>(frameBufferWidth),
                               static_cast<uint32_t>(frameBufferHeight)};

    actualExtent.width =
        std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                   capabilities.maxImageExtent.width);
    actualExtent.height =
        std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                   capabilities.maxImageExtent.height);

    return actualExtent;
}
