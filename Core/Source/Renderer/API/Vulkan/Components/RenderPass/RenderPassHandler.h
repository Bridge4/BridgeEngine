#ifndef RENDERPASSHANDLER_H
#define RENDERPASSHANDLER_H
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <memory>
#include <vector>

#include "../ComponentDeclarations.h"
// class VulkanContext;
// class SwapChainHandler;
// class DeviceHandler;

// enum VkFormat;

namespace backend_vk::render_pass {
struct DepthResources {
    VkImage image;
    VkImageView image_view;
    VkDeviceMemory image_memory;
};

struct RenderPassData {
    enum ERenderPassDataState {
        VALID,
        FRAMEBUFFERS_INVALID,
        ATTACHMENT_COUNT_INVALID,
        RENDER_PASS_INVALID
    };
    VkRenderPass render_pass = VK_NULL_HANDLE;
    int attachment_count;
    std::vector<VkFramebuffer> framebuffers;
    DepthResources depth_resources;

    ERenderPassDataState GetRenderPassDataState() {
        if (this->framebuffers.empty())
            return ERenderPassDataState::FRAMEBUFFERS_INVALID;
        if (attachment_count == 0)
            return ERenderPassDataState::ATTACHMENT_COUNT_INVALID;
        if (render_pass == VK_NULL_HANDLE)
            return ERenderPassDataState::RENDER_PASS_INVALID;

        return ERenderPassDataState::VALID;
    }
};

struct RenderPassFramebuffer {
    uint32_t width;
    uint32_t height;
    std::shared_ptr<VkFramebuffer> framebuffer;
};

RenderPassData create_render_pass(VkDevice& logical_device,
                                  VkFormat colorAttachmentImageFormat,
                                  VkFormat depthAttachmentFormat);

VkFormat find_supported_format(VkPhysicalDevice& physical_device,
                               const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features);

DepthResources create_render_pass_depth_resources(
    VkPhysicalDevice& physical_device, VkDevice& logical_device,
    VkCommandPool& command_pool, VkQueue& graphics_queue, VkFormat depth_format,
    uint32_t width, uint32_t height);
}  // namespace backend_vk::render_pass
#endif
