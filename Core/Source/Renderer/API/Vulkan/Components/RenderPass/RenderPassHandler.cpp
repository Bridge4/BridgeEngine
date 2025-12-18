#include "RenderPassHandler.h"

#include <vulkan/vulkan_core.h>

#include "../../VulkanContext.h"
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
#include "Source/Renderer/API/Vulkan/Components/Images/ImageHandler.h"

void RenderPassHandler::CreateRenderPass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = m_vulkanGlobalState->GetSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Store rendered contents in memory to be read later
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // Lookup stencil buffers later
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // The initialLayout specifies which layout the image will have before the
    // render pass begins.
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    /*
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color
       attachment VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the
       swap chain VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Images to be used as
       destination for a memory copy operation

        "The caveat of this special value is that the contents of the image are
       not guaranteed to be preserved, but that doesn't matter since we're going
       to clear it anyway. We want the image to be ready for presentation using
       the swap chain after rendering, which is why we use
       VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as finalLayout."
    */

    // The finalLayout specifies the layout to automatically transition to when
    // the render pass finishes.
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depthAttachment{};

    depthAttachment.format =
        FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT},
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    /*
        pInputAttachments: Attachments that are read from a shader
        pResolveAttachments: Attachments used for multisampling color
       attachments pDepthStencilAttachment : Attachment for depthand stencil
       data pPreserveAttachments : Attachments that are not used by this
       subpass, but for which the data must be preserved
    */
    // The index of the attachment in this array is directly referenced from the
    // fragment shader with the layout(location = 0) out vec4 outColor directive
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;

    // Creating render pass object and populating the struct with necessary
    // properties
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,
                                                          depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (vkCreateRenderPass(*m_vulkanGlobalState->GetRefLogicalDevice(),
                           &renderPassInfo, nullptr,
                           &m_vulkanGlobalState->m_renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPassHandler::CreateShadowPass() {
    VkAttachmentDescription depthAttachment{};

    depthAttachment.format =
        FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT},
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    // The index of the attachment in this array is directly referenced from the
    // fragment shader with the layout(location = 0) out vec4 outColor directive
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependency.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                              VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo shadowPassInfo{};
    shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    shadowPassInfo.attachmentCount = 1;
    shadowPassInfo.pAttachments = &depthAttachment;
    shadowPassInfo.subpassCount = 1;
    shadowPassInfo.pSubpasses = &subpass;
    shadowPassInfo.dependencyCount = 1;
    shadowPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(*m_vulkanGlobalState->GetRefLogicalDevice(),
                           &shadowPassInfo, nullptr,
                           &m_vulkanGlobalState->m_shadowPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void RenderPassHandler::CreateRenderPassDepthResources() {
    VkFormat depthFormat =
        FindSupportedFormat({VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                             VK_FORMAT_D24_UNORM_S8_UINT},
                            VK_IMAGE_TILING_OPTIMAL,
                            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    m_imageHandler->CreateImage(
        m_vulkanGlobalState->GetSwapChainExtent().width,
        m_vulkanGlobalState->GetSwapChainExtent().height, depthFormat,
        VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_vulkanGlobalState->m_renderPassDepthImage,
        m_vulkanGlobalState->m_renderPassDepthImageMemory);
    m_vulkanGlobalState->m_renderPassDepthImageView =
        m_imageHandler->CreateImageView(
            *m_vulkanGlobalState->GetRefLogicalDevice(),
            m_vulkanGlobalState->m_renderPassDepthImage, depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT);

    TransitionImageLayout(m_vulkanGlobalState->m_renderPassDepthImage,
                          depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void RenderPassHandler::CreateRenderPassFrameBuffers() {
    CreateRenderPassDepthResources();
    m_vulkanGlobalState->m_renderPassFrameBuffers.resize(
        m_vulkanGlobalState->m_swapChainImageViews.size());

    // Loop through swap chain image views
    for (size_t i = 0; i < m_vulkanGlobalState->m_swapChainImageViews.size();
         i++) {
        std::array<VkImageView, 2> attachments = {
            m_vulkanGlobalState->m_swapChainImageViews[i],
            m_vulkanGlobalState->m_renderPassDepthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_vulkanGlobalState->m_renderPass;
        framebufferInfo.attachmentCount =
            static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_vulkanGlobalState->GetSwapChainExtent().width;
        framebufferInfo.height =
            m_vulkanGlobalState->GetSwapChainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                *m_vulkanGlobalState->GetRefLogicalDevice(), &framebufferInfo,
                nullptr, &m_vulkanGlobalState->m_renderPassFrameBuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void RenderPassHandler::CreateShadowPassDepthResources() {
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    m_imageHandler->CreateImage(
        m_vulkanGlobalState->m_shadowMapWidth,
        m_vulkanGlobalState->m_shadowMapHeight, depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_vulkanGlobalState->m_shadowPassDepthImage,
        m_vulkanGlobalState->m_shadowPassDepthImageMemory);

    m_vulkanGlobalState->m_shadowPassDepthImageView =
        m_imageHandler->CreateImageView(
            *m_vulkanGlobalState->GetRefLogicalDevice(),
            m_vulkanGlobalState->m_shadowPassDepthImage, depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT);

    TransitionImageLayout(m_vulkanGlobalState->m_shadowPassDepthImage,
                          depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                          VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void RenderPassHandler::CreateShadowPassFrameBuffers() {
    CreateShadowPassDepthResources();
    m_vulkanGlobalState->m_shadowPassFrameBuffers.resize(
        m_vulkanGlobalState->m_lights.numLights.x);

    for (size_t i = 0; i < m_vulkanGlobalState->m_lights.numLights.x; i++) {
        std::array<VkImageView, 1> attachments = {
            m_vulkanGlobalState->m_shadowPassDepthImageView};

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_vulkanGlobalState->m_shadowPass;
        framebufferInfo.attachmentCount =
            static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_vulkanGlobalState->m_shadowMapWidth;
        framebufferInfo.height = m_vulkanGlobalState->m_shadowMapHeight;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(
                *m_vulkanGlobalState->GetRefLogicalDevice(), &framebufferInfo,
                nullptr, &m_vulkanGlobalState->m_shadowPassFrameBuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

VkFormat RenderPassHandler::FindSupportedFormat(
    const std::vector<VkFormat>& candidates, VkImageTiling tiling,
    VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(
            m_vulkanGlobalState->GetPhysicalDevice(), format, &props);
        if (tiling == VK_IMAGE_TILING_LINEAR &&
            (props.linearTilingFeatures & features) == features) {
            return format;
        } else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                   (props.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    throw std::runtime_error("failed to find supported format!");
}

void RenderPassHandler::TransitionImageLayout(VkImage image, VkFormat format,
                                              VkImageLayout oldLayout,
                                              VkImageLayout newLayout) {
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

        if (format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
            format == VK_FORMAT_D24_UNORM_S8_UINT) {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    } else {
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

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
        newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
               newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    } else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
               newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
                                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    } else {
        throw std::invalid_argument("unsupported layout transition!");
    }

    // Syncronization
    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0,
                         nullptr, 0, nullptr, 1, &barrier);
    EndSingleTimeCommands(commandBuffer);
}

VkCommandBuffer RenderPassHandler::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_vulkanGlobalState->m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(*m_vulkanGlobalState->GetRefLogicalDevice(),
                             &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void RenderPassHandler::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vulkanGlobalState->m_graphicsQueue, 1, &submitInfo,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkanGlobalState->m_graphicsQueue);

    vkFreeCommandBuffers(*m_vulkanGlobalState->GetRefLogicalDevice(),
                         m_vulkanGlobalState->m_commandPool, 1, &commandBuffer);
}
