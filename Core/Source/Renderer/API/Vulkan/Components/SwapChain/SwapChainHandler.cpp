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
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
#include "../Window/WindowHandler.h"

void SwapChainHandler::Initialize() {
    VkSurfaceKHR surface = m_vulkanGlobalState->GetSurface();

    SwapChainSupportDetails swapChainSupport =
        m_deviceHandler->QuerySwapChainSupport(
            m_vulkanGlobalState->GetPhysicalDevice());

    VkSurfaceFormatKHR surfaceFormat =
        ChooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode =
        ChooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities);

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

    QueueFamilyIndices indices = m_deviceHandler->FindQueueFamilies(
        m_vulkanGlobalState->GetPhysicalDevice());
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(),
                                     indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily) {
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
            *m_vulkanGlobalState->GetRefLogicalDevice(), &createInfo, nullptr,
            m_vulkanGlobalState->GetRefSwapChain()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }

    vkGetSwapchainImagesKHR(*m_vulkanGlobalState->GetRefLogicalDevice(),
                            *m_vulkanGlobalState->GetRefSwapChain(),
                            &imageCount, nullptr);
    std::vector<VkImage> swapChainImages =
        *m_vulkanGlobalState->GetRefSwapChainImages();
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(*m_vulkanGlobalState->GetRefLogicalDevice(),
                            *m_vulkanGlobalState->GetRefSwapChain(),
                            &imageCount, swapChainImages.data());
    m_vulkanGlobalState->SetSwapChainImages(swapChainImages);

    m_vulkanGlobalState->SetSwapChainImageFormat(surfaceFormat.format);
    m_vulkanGlobalState->SetSwapChainExtent(extent);
    std::vector<VkImageView> swapChainImageViews =
        *m_vulkanGlobalState->GetRefSwapChainImageViews();
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        // Moved definition for createImageView() to a class and turned into a
        // static function
        swapChainImageViews[i] = m_imageViewHandler->CreateImageView(
            *m_vulkanGlobalState->GetRefLogicalDevice(),
            m_vulkanGlobalState->m_swapChainImages[i],
            m_vulkanGlobalState->m_swapChainImageFormat,
            VK_IMAGE_ASPECT_COLOR_BIT);
    }
    m_vulkanGlobalState->SetSwapChainImageViews(swapChainImageViews);
}

void SwapChainHandler::Rebuild() {
    // Handling minimization
    m_windowHandler->HandleMinimization();
    vkDeviceWaitIdle(*m_vulkanGlobalState->GetRefLogicalDevice());
    Destroy();
    Initialize();

    // TODO: MUST NOT FORGET THIS
    // m_renderPassHandler->RebuildPasses();
}

void SwapChainHandler::Destroy() {
    // vkDestroyImageView(*m_vulkanGlobalState->GetRefLogicalDevice(),
    //                    m_vulkanGlobalState->m_renderPassDepthImageView,
    //                    nullptr);
    // vkDestroyImage(*m_vulkanGlobalState->GetRefLogicalDevice(),
    //                m_vulkanGlobalState->m_renderPassDepthImage, nullptr);
    // vkFreeMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
    //              m_vulkanGlobalState->m_renderPassDepthImageMemory, nullptr);

    // for (auto framebuffer : m_vulkanGlobalState->m_renderPassFrameBuffers) {
    //     vkDestroyFramebuffer(*m_vulkanGlobalState->GetRefLogicalDevice(),
    //                          framebuffer, nullptr);
    // }

    // for (auto imageView : m_vulkanGlobalState->m_swapChainImageViews) {
    //     vkDestroyImageView(*m_vulkanGlobalState->GetRefLogicalDevice(),
    //                        imageView, nullptr);
    // }

    vkDestroySwapchainKHR(*m_vulkanGlobalState->GetRefLogicalDevice(),
                          m_vulkanGlobalState->m_swapChain, nullptr);
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
    const VkSurfaceCapabilitiesKHR& capabilities) {
    // Fun fact: wrapping a function call with parentheses can prevent macro
    // expansions
    if (capabilities.currentExtent.width !=
        (std::numeric_limits<uint32_t>::max)()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        m_windowHandler->GetFramebufferSize(&width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width),
                                   static_cast<uint32_t>(height)};

        actualExtent.width =
            std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                       capabilities.maxImageExtent.width);
        actualExtent.height =
            std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                       capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
