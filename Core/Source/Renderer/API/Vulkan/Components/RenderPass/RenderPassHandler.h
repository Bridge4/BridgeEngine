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
    VkImageView imageView;
    VkDeviceMemory imageMemory;
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

}  // namespace backend_vk::render_pass

struct BrVkRenderPass {
    VkViewport viewport;
    VkRect2D scissor;
    VkRenderPassBeginInfo renderPassInfo;
    VkPipeline pipeline;
};

class RenderPassHandler {
   public:
    RenderPassHandler(VulkanContext* vulkanContext,
                      SwapChainHandler* swapChainHandler,
                      DeviceHandler* deviceHandler,
                      VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkanContext = vulkanContext;
        this->m_swapChainHandler = swapChainHandler;
        this->m_device_handler = deviceHandler;
        this->m_vulkanGlobalState = VulkanGlobalState;
    }

    backend_vk::render_pass::DepthResources create_render_pass_depth_resources(
        VkFormat depthFormat, uint32_t width, uint32_t height);
    // void CreateRenderPassFrameBuffers();

    void create_shadow_pass();
    void create_shadow_pass_depth_resources();
    void create_shadow_pass_framebuffers();

    VkCommandBuffer begin_single_time_commands();

    void end_single_time_commands(VkCommandBuffer commandBuffer);
    VkFormat find_supported_format(const std::vector<VkFormat>& candidates,
                                   VkImageTiling tiling,
                                   VkFormatFeatureFlags features);

    void transition_image_layout(VkImage image, VkFormat format,
                                 VkImageLayout oldLayout,
                                 VkImageLayout newLayout);

    VkRenderPass m_renderPass = 0;
    VulkanContext* m_vulkanContext = 0;
    SwapChainHandler* m_swapChainHandler = 0;
    DeviceHandler* m_device_handler = 0;
    VulkanGlobalState* m_vulkanGlobalState = nullptr;
};
#endif
