#ifndef RENDERPASSHANDLER_H
#define RENDERPASSHANDLER_H
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <vector>

#include "../ComponentDeclarations.h"
// class VulkanContext;
// class SwapChainHandler;
// class DeviceHandler;

// enum VkFormat;

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
                      DeviceHandler* deviceHandler, ImageHandler* imageHandler,
                      VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkanContext = vulkanContext;
        this->m_swapChainHandler = swapChainHandler;
        this->m_deviceHandler = deviceHandler;
        this->m_imageHandler = imageHandler;
        this->m_vulkanGlobalState = VulkanGlobalState;
    }

    void CreateRenderPass();
    void CreateRenderPassDepthResources();
    void CreateRenderPassFrameBuffers();

    void CreateShadowPass();
    void CreateShadowPassDepthResources();
    void CreateShadowPassFrameBuffers();

    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features);

    void TransitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);

    VkRenderPass m_renderPass = 0;
    VulkanContext* m_vulkanContext = 0;
    SwapChainHandler* m_swapChainHandler = 0;
    DeviceHandler* m_deviceHandler = 0;
    VulkanGlobalState* m_vulkanGlobalState = nullptr;
    ImageHandler* m_imageHandler = 0;
};
#endif
