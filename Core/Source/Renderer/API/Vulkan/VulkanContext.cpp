#pragma once
#include <cassert>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanContext.h"
// Textures through stb library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>


#include <chrono>
#include <iostream>
#include <array>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <map>
#include <optional>
#include <set>

#include "Components/Camera/CameraHandler.h"
#include "Components/Devices/DeviceHandler.h"
#include "Components/SwapChain/SwapChainHandler.h"
#include "Components/Window/WindowHandler.h"
#include "Components/Buffers/BufferHandler.h"
#include "Components/RenderPass/RenderPassHandler.h"
#include "Components/Mesh/Mesh3D.h"
//#include "../../../../Camera.h"

#include "Components/Images/ImageHandler.h"
#include "Components/VulkanInstanceManager/VulkanInstanceManager.h"

void VulkanContext::Construct() {

    m_vulkanInstanceManager = new VulkanInstanceManager(this);

    deviceHandler = new DeviceHandler(m_vulkanInstanceManager);
    imageViewBuilder = new ImageHandler(this);
    swapChainHandler = new SwapChainHandler(this, deviceHandler, windowHandler, imageViewBuilder, m_vulkanInstanceManager);
    bufferHandler = new BufferHandler(m_vulkanInstanceManager);
    renderPassHandler = new RenderPassHandler(this, swapChainHandler, deviceHandler, m_vulkanInstanceManager);
    //VulkanContext* vulkanContext, WindowHandler* windowHandler, SwapChainHandler* swapChainHandler, BufferHandler* bufferHandler
    cameraHandler = new CameraHandler(this, windowHandler, swapChainHandler, bufferHandler, m_vulkanInstanceManager);
    //camera = new Camera(this, windowHandler);

    //CreateVulkanContext();
    m_vulkanInstanceManager->CreateInstance();
    windowHandler->vulkanContext = this;
    if (windowHandler->CreateSurface() != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
    // DONE
    deviceHandler->Initialize();
    // DONE 
    swapChainHandler->Initialize();
    renderPassHandler->Initialize();

    swapChainHandler->AttachRenderPassHandler(renderPassHandler);
    CreateCommandPool();
    swapChainHandler->CreateFramebuffers();
    //swapChainHandler->CreateDepthResources();

    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();

    /*CreateDepthResources();
    CreateFramebuffers();*/


    CreateTextureImage(TEXTURE_PATH1);
    CreateTextureImageView();
    CreateTextureSampler();

    LoadModel(MODEL_PATH1);
    
    bufferHandler->BuildVertexBuffer(m_vertices);
    bufferHandler->BuildIndexBuffer(m_indices);
    //CreateIndexBuffer();
    //CreateUniformBuffers();
    bufferHandler->BuildUniformBuffers();

    CreateDescriptorPool();
    CreateDescriptorSets();

    //CreateCommandBuffers();
    bufferHandler->BuildCommandBuffers();
    CreateSyncObjects();   
}

void VulkanContext::RenderLoop() {
    while (!windowHandler->ShouldClose()) {
        
        windowHandler->Poll();
        DrawFrame();
    }
    vkDeviceWaitIdle(*m_vulkanInstanceManager->GetRefLogicalDevice());
    Destroy();
}


void VulkanContext::CreateDescriptorSetLayout() {

    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    // Example comment
    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
    samplerLayoutBinding.binding = 1;
    samplerLayoutBinding.descriptorCount = 1;
    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    samplerLayoutBinding.pImmutableSamplers = nullptr;
    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    // Another Example Comment
    if (vkCreateDescriptorSetLayout(*m_vulkanInstanceManager->GetRefLogicalDevice(), &layoutInfo, nullptr, &m_vulkanInstanceManager->m_descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void VulkanContext::CreateGraphicsPipeline() {
    std::vector<char> vertShaderCode = readFile("C:/Source/Engines/BridgeEngine/Core/Shaders/vert.spv");
    std::vector<char> fragShaderCode = readFile("C:/Source/Engines/BridgeEngine/Core/Shaders/frag.spv");

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
    // CODE BELOW THIS
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    // IMPORTANT NOTE!!!
    /*
        The next two members specify the shader module containing the code, and the function to invoke, known as the entrypoint.
        That means that it's possible to combine multiple fragment shaders into a single shader module and use different entry
        points to differentiate between their behaviors. In this case we'll stick to the standard main, however.
    */

    // Shader module creation
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


    // Dynamic States of pipeline
    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Passing vertex data into pipeline
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescriptions();


    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());

    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Geometry to be drawn from Vertices passed in
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports and Scissors

    // The viewport takes the swapchain images and transforms them to framebuffers.
    // Here we are setting the viewport to the swapchain images
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_vulkanInstanceManager->GetSwapChainExtent().width;
    viewport.height = (float)m_vulkanInstanceManager->GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Determines which part of the framebuffers will be rasterized. 
    // Here we set it to the swapchain images so we can see the whole framebuffer.
    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_vulkanInstanceManager->GetSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // depthClampEnable : Whether or not things outside the near and far planes are discarded or clamped to them
    rasterizer.depthClampEnable = VK_FALSE;
    // rasterizerDiscardEnable : Allows geometry to pass through the rasterizer stage. 
    // If disabled, disables output to framebuffer.
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // VK_POLYGON_MODE_FILL = Fill in shape

    // !! BELOW OPTIONS REQUIRE ENABLING GPU FEATURE !!
    // VK_POLYGON_MODE_LINE = Wireframe
    // VK_POLYGON_MODE_POINT = Vertices drawn as points
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    // Backface culling enabled here
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    // Determines draw order of a "front" face
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    // !! Will learn about this in the future !!
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional


    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional


    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // REVIEW THIS. I DON'T KNOW WHAT THIS DOES BUT MY BRAIN IS FUZZY AND I DON'T CARE TO LEARN.
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &m_vulkanInstanceManager->m_descriptorSetLayout;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(*m_vulkanInstanceManager->GetRefLogicalDevice(), &pipelineLayoutInfo, nullptr, &m_vulkanInstanceManager->m_pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    };

    // Finally creating graphics pipeline object and populating with necessary information
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicState;
    pipelineInfo.layout = m_vulkanInstanceManager->m_pipelineLayout;
    pipelineInfo.renderPass = *m_vulkanInstanceManager->GetRefRenderPass();
    // Subpass INDEX
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(*m_vulkanInstanceManager->GetRefLogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_vulkanInstanceManager->m_graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    // CODE ABOVE THIS
    vkDestroyShaderModule(*m_vulkanInstanceManager->GetRefLogicalDevice(), fragShaderModule, nullptr);
    vkDestroyShaderModule(*m_vulkanInstanceManager->GetRefLogicalDevice(), vertShaderModule, nullptr);
}

void VulkanContext::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = deviceHandler->findQueueFamilies(m_vulkanInstanceManager->GetPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // Selecting graphicsFamily in order to issue draw commands in this command pool
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(*m_vulkanInstanceManager->GetRefLogicalDevice(), &poolInfo, nullptr, &m_vulkanInstanceManager->m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

// Pass in the Model3D's texture path
void VulkanContext::CreateTextureImage(std::string texturePath) {
    int texWidth, texHeight, texChannels;

    stbi_uc* pixels = stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    // The multiplication by 4 here is because the pixels are loaded row by row with 4 bytes per pixel
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    bufferHandler->BuildBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory);

    stbi_image_free(pixels);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_vulkanInstanceManager->m_textureImage, m_vulkanInstanceManager->m_textureImageMemory);

    transitionImageLayout(m_vulkanInstanceManager->m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, m_vulkanInstanceManager->m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    transitionImageLayout(m_vulkanInstanceManager->m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory, nullptr);
}

// TEXTURE IMAGE VIEW
void VulkanContext::CreateTextureImageView() {
    m_vulkanInstanceManager->m_textureImageView = imageViewBuilder->CreateImageView(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

// TEXTURE SAMPLER
void VulkanContext::CreateTextureSampler() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_vulkanInstanceManager->GetPhysicalDevice(), &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(*m_vulkanInstanceManager->GetRefLogicalDevice(), &samplerInfo, nullptr, &m_vulkanInstanceManager->m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

// TODO: Move this to its own class, we should be passing in loaded objects to the renderer, renderer makes draw calls on those objects
void VulkanContext::LoadModel(std::string modelPath) {
    tinyobj::attrib_t attrib;

    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelPath.c_str())) {
        throw std::runtime_error(warn + err);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };



            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(m_vertices.size());
                m_vertices.push_back(vertex);
            }
            m_indices.push_back(uniqueVertices[vertex]);

            vertex.color = { 1.0f, 1.0f, 1.0f };
        }
    }
}

// DESCRIPTOR POOL
void VulkanContext::CreateDescriptorPool() {

    /*
        Incorrectly sized descriptor pools could result in errors on some devices but not others depending on the GPU drivers
        Some will let you get away with bad allocations, some won't. Be careful.
    */

    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t> (poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(*m_vulkanInstanceManager->GetRefLogicalDevice(), &poolInfo, nullptr, &m_vulkanInstanceManager->m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

// DESCRIPTOR SETS
void VulkanContext::CreateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_vulkanInstanceManager->m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkanInstanceManager->m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    m_vulkanInstanceManager->m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, m_vulkanInstanceManager->m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_vulkanInstanceManager->m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = m_vulkanInstanceManager->m_textureImageView;
        imageInfo.sampler = m_vulkanInstanceManager->m_textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = m_vulkanInstanceManager->m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        descriptorWrites[0].pImageInfo = nullptr; // Optional
        descriptorWrites[0].pTexelBufferView = nullptr; // Optional

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = m_vulkanInstanceManager->m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(*m_vulkanInstanceManager->GetRefLogicalDevice(), (uint32_t)descriptorWrites.size(), descriptorWrites.data(), descriptorCopyCount, nullptr);
    }
}


void VulkanContext::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    /*
        If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will implicitly reset it.
        It's not possible to append commands to a buffer at a later time.
    */
    VkCommandBufferBeginInfo beginInfo{};
    /*
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be rerecorded right after executing it once.
        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : This is a secondary command buffer that will be entirely within a single render pass.
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : The command buffer can be resubmitted while it is also already pending execution.
    */
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;


    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_vulkanInstanceManager->m_renderPass;
    // DEBUG
    //printf("imageIndex DEBUG: %d\n", imageIndex);
    renderPassInfo.framebuffer = m_vulkanInstanceManager->m_swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = m_vulkanInstanceManager->GetSwapChainExtent();

    // VK_ATTACHMENT_LOAD_OP_3 clear values for color and depth stencil
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = { {1.0f, 0.2331f, 0.1555f, 1.0f} };
    clearValues[1].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();


    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // Binding the graphics pipeline
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vulkanInstanceManager->m_graphicsPipeline);

    // We set the viewport and scissor state as dynamic in the pipeline 
    // We need to set those up in the command buffer now  
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_vulkanInstanceManager->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(m_vulkanInstanceManager->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = m_vulkanInstanceManager->GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = { bufferHandler->VertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindIndexBuffer(commandBuffer, bufferHandler->IndexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_vulkanInstanceManager->m_pipelineLayout, 0, 1,
        &m_vulkanInstanceManager->m_descriptorSets[m_vulkanInstanceManager->m_currentFrame], 0, nullptr);

    /*
        vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance)
        vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
        instanceCount: Used for instanced rendering, use 1 if you're not doing that.
        firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
        firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
    */
    //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

}

void VulkanContext::CreateSyncObjects() {
    m_vulkanInstanceManager->m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vulkanInstanceManager->m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    m_vulkanInstanceManager->m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);


    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(*m_vulkanInstanceManager->GetRefLogicalDevice(), &semaphoreInfo, nullptr, &m_vulkanInstanceManager->m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(*m_vulkanInstanceManager->GetRefLogicalDevice(), &semaphoreInfo, nullptr, &m_vulkanInstanceManager->m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(*m_vulkanInstanceManager->GetRefLogicalDevice(), &fenceInfo, nullptr, &m_vulkanInstanceManager->m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void VulkanContext::DrawFrame() {
    /*
        - Wait for the previous frame to finish
        - Acquire an image from the swap chain
        - Record a command buffer which draws the scene onto that image
        - Submit the recorded command buffer
        - Present the swap chain image
    */
    cameraHandler->HandleInput();
    cameraHandler->UpdateUniformBuffer(m_vulkanInstanceManager->m_currentFrame);
    vkWaitForFences(*m_vulkanInstanceManager->GetRefLogicalDevice(), 1, &m_vulkanInstanceManager->m_inFlightFences[m_vulkanInstanceManager->m_currentFrame], VK_TRUE, UINT64_MAX);

    // Acquire an image from the swap chain
    uint32_t imageIndex;

    // SWAP CHAIN RECREATION
    VkResult result = vkAcquireNextImageKHR(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_swapChain, UINT64_MAX, m_vulkanInstanceManager->m_imageAvailableSemaphores[m_vulkanInstanceManager->m_currentFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        swapChainHandler->Rebuild();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset fences if we are submitting work
    vkResetFences(*m_vulkanInstanceManager->GetRefLogicalDevice(), 1, &m_vulkanInstanceManager->m_inFlightFences[m_vulkanInstanceManager->m_currentFrame]);

    // Record a command buffer which draws the scene onto that image
    vkResetCommandBuffer(m_vulkanInstanceManager->m_commandBuffers[m_vulkanInstanceManager->m_currentFrame], 0);
    recordCommandBuffer(m_vulkanInstanceManager->m_commandBuffers[m_vulkanInstanceManager->m_currentFrame], imageIndex);

    // Submit the recorded command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_vulkanInstanceManager->m_imageAvailableSemaphores[m_vulkanInstanceManager->m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_vulkanInstanceManager->m_commandBuffers[m_vulkanInstanceManager->m_currentFrame];

    VkSemaphore signalSemaphores[] = { m_vulkanInstanceManager->m_renderFinishedSemaphores[m_vulkanInstanceManager->m_currentFrame] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(m_vulkanInstanceManager->m_graphicsQueue, 1, &submitInfo, m_vulkanInstanceManager->m_inFlightFences[m_vulkanInstanceManager->m_currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Present swap chain image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { *m_vulkanInstanceManager->GetRefSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
    //presentInfo.pResults = nullptr; // Optional

    // SWAP CHAIN RECREATION
    result = vkQueuePresentKHR(m_vulkanInstanceManager->m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || windowHandler->framebufferResized) {
        windowHandler->framebufferResized = false;
        swapChainHandler->Rebuild();
    }
    else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }

    // Progress to next frame
    m_vulkanInstanceManager->m_currentFrame = (m_vulkanInstanceManager->m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}


// BEGIN_REGION CreateVulkanContext()
// BEGIN_REGION CreateVulkanContext_helpers
static bool checkValidationLayerSupport(const std::vector<const char*>* validationLayers) {

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : *validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

static std::vector<const char*> getRequiredExtensions(const bool* enableValidationLayers) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (*enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
                                                    VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
                                                    void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}
// END_REGION CreateVulkanContext_helpers

void VulkanContext::CreateVulkanContext()
{
    // A lot of information is passed through structs rather than function parameters
    if (enableValidationLayers && !checkValidationLayerSupport(&validationLayers)) {
        throw std::runtime_error("validation layers requested but not available...");
    }

    // optional but helpful information for the driver
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Bridge Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Bridge Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_3;

    // MANDATORY for instance creation
    // Telling Vulkan what global (program) extensions and validations we want to use
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;


    // ADDING appInfo to createInfo
    createInfo.pApplicationInfo = &appInfo;


    // Vulkan is platform agnostic. Here we provide the GLFW extension to interface with the window system
    std::vector<const char*> extensions = getRequiredExtensions(&enableValidationLayers);
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        //populateDebugMessengerCreateInfo(debugCreateInfo);
        debugCreateInfo = {};
        debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debugCreateInfo.pfnUserCallback = debugCallback;
        debugCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}
// END_REGION CreateVulkanContext()


void VulkanContext::Destroy() {
    swapChainHandler->Destroy();
    //DestroySwapChain();

    vkDestroySampler(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_textureSampler, nullptr);
    vkDestroyImageView(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_textureImageView, nullptr);
    vkDestroyImage(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_textureImage, nullptr);
    vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_textureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), bufferHandler->UniformBuffers[i], nullptr);
        vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), bufferHandler->UniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_descriptorSetLayout, nullptr);

    // Buffer cleanup
    // Call BufferHandler::DestroyBuffers
    bufferHandler->DestroyBuffers();
    

    vkDestroyPipeline(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_pipelineLayout, nullptr);

    vkDestroyRenderPass(*m_vulkanInstanceManager->GetRefLogicalDevice(), renderPassHandler->renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_commandPool, nullptr);

    // DEVICE DESTRUCTION
    deviceHandler->Destroy();
    // GLFW DESTRUCTION
    windowHandler->Destroy();
}

VkShaderModule VulkanContext::createShaderModule(const std::vector<char>& code)
{
    
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(*m_vulkanInstanceManager->GetRefLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
    
}
// Helpers

bool hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanContext::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = { 0, 0, 0 };
    region.imageExtent = {
        width,
        height,
        1
    };
    vkCmdCopyBufferToImage(
        commandBuffer,
        buffer,
        image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );
    endSingleTimeCommands(commandBuffer);
}

// beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
VkCommandBuffer VulkanContext::beginSingleTimeCommands() {
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

void VulkanContext::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vulkanInstanceManager->m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkanInstanceManager->m_graphicsQueue);

    vkFreeCommandBuffers(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_commandPool, 1, &commandBuffer);
}

void VulkanContext::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
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
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), image, imageMemory, 0);
}

void VulkanContext::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format)) {
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
    endSingleTimeCommands(commandBuffer);
}

// COPY BUFFER
void VulkanContext::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    endSingleTimeCommands(commandBuffer);
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
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
