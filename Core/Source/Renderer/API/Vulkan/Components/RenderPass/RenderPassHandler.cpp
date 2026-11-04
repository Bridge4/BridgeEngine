#include "RenderPassHandler.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>

#include "../../VulkanContext.h"
#include "../VulkanGlobalState/VulkanGlobalState.h"
#include "Source/Renderer/API/Vulkan/Components/Images/ImageHandler.h"

namespace backend_vk::render_pass {

RenderPassData create_render_pass(VkDevice& logical_device,
                                  VkFormat color_attachment_image_format,
                                  VkFormat depth_attachment_format) {
    VkRenderPass render_pass;

    VkAttachmentDescription color_attachment{};
    color_attachment.format = color_attachment_image_format;
    color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    // Store rendered contents in memory to be read later
    color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // Lookup stencil buffers later
    color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    // The initialLayout specifies which layout the image will have before the
    // render pass begins.
    color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

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
    color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference color_attachment_ref{};
    color_attachment_ref.attachment = 0;
    color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription depth_attachment{};

    // depthAttachment.format =
    //     FindSupportedFormat({VK_FORMAT_D32_SFLOAT,
    //     VK_FORMAT_D32_SFLOAT_S8_UINT,
    //                          VK_FORMAT_D24_UNORM_S8_UINT},
    //                         VK_IMAGE_TILING_OPTIMAL,
    //                         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    depth_attachment.format = depth_attachment_format;
    depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_attachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_attachment_ref{};
    depth_attachment_ref.attachment = 1;
    depth_attachment_ref.layout =
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
    subpass.pColorAttachments = &color_attachment_ref;
    subpass.pDepthStencilAttachment = &depth_attachment_ref;

    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcAccessMask = 0;

    // Creating render pass object and populating the struct with necessary
    // properties
    std::array<VkAttachmentDescription, 2> attachments = {color_attachment,
                                                          depth_attachment};
    VkRenderPassCreateInfo render_pass_info{};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount =
        static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass;
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    if (vkCreateRenderPass(logical_device, &render_pass_info, nullptr,
                           &render_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    backend_vk::render_pass::RenderPassData render_pass_data = {};
    render_pass_data.render_pass = render_pass;
    render_pass_data.attachment_count = render_pass_info.attachmentCount;

    return render_pass_data;
}

RenderPassData create_shadow_pass(VkDevice& logical_device,
                                  VkFormat color_attachment_image_format,
                                  VkFormat depth_attachment_format) {
    VkRenderPass shadow_pass;
    VkAttachmentDescription depthAttachment{};

    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Two dependencies: begin and end
    std::array<VkSubpassDependency, 2> dependencies{};

    // External -> Subpass 0 (begin)
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Subpass 0 -> External (end)
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkRenderPassCreateInfo shadow_pass_info{};
    shadow_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    shadow_pass_info.attachmentCount = 1;
    shadow_pass_info.pAttachments = &depthAttachment;
    shadow_pass_info.subpassCount = 1;
    shadow_pass_info.pSubpasses = &subpass;
    shadow_pass_info.dependencyCount = 1;
    shadow_pass_info.pDependencies = dependencies.data();

    if (vkCreateRenderPass(logical_device, &shadow_pass_info, nullptr,
                           &shadow_pass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }

    backend_vk::render_pass::RenderPassData shadow_pass_data = {};
    shadow_pass_data.render_pass = shadow_pass;
    shadow_pass_data.attachment_count = shadow_pass_info.attachmentCount;

    return shadow_pass_data;
}

VkFormat find_supported_format(VkPhysicalDevice& physical_device,
                               const std::vector<VkFormat>& candidates,
                               VkImageTiling tiling,
                               VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);
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
}  // namespace backend_vk::render_pass

void RenderPassHandler::create_shadow_pass() {
    VkAttachmentDescription depthAttachment{};

    depthAttachment.format = VK_FORMAT_D32_SFLOAT;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Two dependencies: begin and end
    std::array<VkSubpassDependency, 2> dependencies{};

    // External -> Subpass 0 (begin)
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                                   VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].dstAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    // Subpass 0 -> External (end)
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].srcAccessMask =
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    VkRenderPassCreateInfo shadowPassInfo{};
    shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    shadowPassInfo.attachmentCount = 1;
    shadowPassInfo.pAttachments = &depthAttachment;
    shadowPassInfo.subpassCount = 1;
    shadowPassInfo.pSubpasses = &subpass;
    shadowPassInfo.dependencyCount = 1;
    shadowPassInfo.pDependencies = dependencies.data();

    if (vkCreateRenderPass(m_device_handler->m_logical_device, &shadowPassInfo,
                           nullptr,
                           &m_vulkanGlobalState->m_shadowPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

backend_vk::render_pass::DepthResources
RenderPassHandler::create_render_pass_depth_resources(VkFormat depth_format,
                                                      uint32_t width,
                                                      uint32_t height) {
    // VkFormat depthFormat =
    //     FindSupportedFormat({VK_FORMAT_D32_SFLOAT,
    //     VK_FORMAT_D32_SFLOAT_S8_UINT,
    //                          VK_FORMAT_D24_UNORM_S8_UINT},
    //                         VK_IMAGE_TILING_OPTIMAL,
    //                         VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    VkImage renderPassDepthImage = {};
    VkImageView renderPassDepthImageView = {};
    VkDeviceMemory renderPassDepthImageMemory = {};

    backend_vk::images::create_image(
        m_device_handler->m_physical_device, m_device_handler->m_logical_device,
        width, height, depth_format, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, renderPassDepthImage,
        renderPassDepthImageMemory);
    renderPassDepthImageView = backend_vk::images::create_image_view(
        m_device_handler->m_logical_device, renderPassDepthImage, depth_format,
        VK_IMAGE_ASPECT_DEPTH_BIT);

    transition_image_layout(renderPassDepthImage, depth_format,
                            VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    backend_vk::render_pass::DepthResources depthResources = {};
    depthResources.image = renderPassDepthImage;
    depthResources.imageView = renderPassDepthImageView;
    depthResources.imageMemory = renderPassDepthImageMemory;

    return depthResources;
}

// void RenderPassHandler::CreateRenderPassFrameBuffers() {
//     CreateRenderPassDepthResources();
//     m_vulkanGlobalState->m_renderPassFrameBuffers.resize(
//         m_vulkanGlobalState->m_swapChainImageViews.size());
//
//     // Loop through swap chain image views
//     for (size_t i = 0; i < m_vulkanGlobalState->m_swapChainImageViews.size();
//          i++) {
//         std::array<VkImageView, 2> attachments = {
//             m_vulkanGlobalState->m_swapChainImageViews[i],
//             m_vulkanGlobalState->m_renderPassDepthImageView};
//
//         VkFramebufferCreateInfo framebufferInfo{};
//         framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//         framebufferInfo.renderPass =
//             *m_vulkanGlobalState->m_renderPassData->renderPass;
//         framebufferInfo.attachmentCount =
//             static_cast<uint32_t>(attachments.size());
//         framebufferInfo.pAttachments = attachments.data();
//         framebufferInfo.pAttachments = attachments.data();
//         framebufferInfo.width =
//         m_vulkanGlobalState->GetSwapChainExtent().width;
//         framebufferInfo.height =
//             m_vulkanGlobalState->GetSwapChainExtent().height;
//         framebufferInfo.layers = 1;
//
//         if (vkCreateFramebuffer(
//                 m_deviceHandler->LogicalDevice, &framebufferInfo, nullptr,
//                 &m_vulkanGlobalState->m_renderPassFrameBuffers[i]) !=
//             VK_SUCCESS) {
//             throw std::runtime_error("failed to create framebuffer!");
//         }
//     }
// }

void RenderPassHandler::create_shadow_pass_depth_resources() {
    VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
    backend_vk::images::create_image(
        m_device_handler->m_physical_device, m_device_handler->m_logical_device,
        m_vulkanGlobalState->m_shadowMapWidth,
        m_vulkanGlobalState->m_shadowMapHeight, depthFormat,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_vulkanGlobalState->m_shadowPassDepthImage,
        m_vulkanGlobalState->m_shadowPassDepthImageMemory);

    m_vulkanGlobalState->m_shadowPassDepthImageView =
        backend_vk::images::create_image_view(
            m_device_handler->m_logical_device,
            m_vulkanGlobalState->m_shadowPassDepthImage, depthFormat,
            VK_IMAGE_ASPECT_DEPTH_BIT);

    transition_image_layout(m_vulkanGlobalState->m_shadowPassDepthImage,
                            depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
                            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

void RenderPassHandler::create_shadow_pass_framebuffers() {
    // WARN: It is very hard to track changes to the global state, when we
    // enter a function we can't tell what's going to change in the global
    // state, when we exit a function we don't know what *has* changed in the
    // global state
    create_shadow_pass_depth_resources();
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
                m_device_handler->m_logical_device, &framebufferInfo, nullptr,
                &m_vulkanGlobalState->m_shadowPassFrameBuffers[i]) !=
            VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

VkFormat RenderPassHandler::find_supported_format(
    const std::vector<VkFormat>& candidates, VkImageTiling tiling,
    VkFormatFeatureFlags features) {
    for (VkFormat format : candidates) {
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_device_handler->m_physical_device,
                                            format, &props);
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

void RenderPassHandler::transition_image_layout(VkImage image, VkFormat format,
                                                VkImageLayout oldLayout,
                                                VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = begin_single_time_commands();
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
    end_single_time_commands(commandBuffer);
}

VkCommandBuffer RenderPassHandler::begin_single_time_commands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_vulkanGlobalState->m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_device_handler->m_logical_device, &allocInfo,
                             &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void RenderPassHandler::end_single_time_commands(
    VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vulkanGlobalState->m_graphicsQueue, 1, &submitInfo,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkanGlobalState->m_graphicsQueue);

    vkFreeCommandBuffers(m_device_handler->m_logical_device,
                         m_vulkanGlobalState->m_commandPool, 1, &commandBuffer);
}
