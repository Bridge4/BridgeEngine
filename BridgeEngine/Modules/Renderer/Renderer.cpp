//#pragma once
//#include "Renderer.h"
//// Textures through stb library
//#define STB_IMAGE_IMPLEMENTATION
//#include <stb_image.h>
//
//#define TINYOBJLOADER_IMPLEMENTATION
//#include <tiny_obj_loader.h>
//
//void initAPI() {
//
//    // Initialization
//    vulkanBridge.init.init(&window);
//    //init.assign(&vulkanBridge.m_surface, &vulkanBridge.m_physicalDevice, &vulkanBridge.m_logicalDevice, &vulkanBridge.m_graphicsQueue, &vulkanBridge.m_presentQueue);
//    vulkanBridge.init.assign(&vulkanBridge.m_surface, &vulkanBridge.m_physicalDevice, &vulkanBridge.m_logicalDevice, &vulkanBridge.m_graphicsQueue, &vulkanBridge.m_presentQueue);
//    // Swap Chain
//    //swapChain.Construct(&vulkanBridge);
//    vulkanBridge.swapChain.create(vulkanBridge.init, window);
//    vulkanBridge.swapChain.createImageViews(vulkanBridge.m_logicalDevice, vulkanBridge.imageView);
//    vulkanBridge.swapChain.assign(&vulkanBridge.m_swapChain, &vulkanBridge.m_swapChainImageFormat, &vulkanBridge.m_swapChainExtent, &vulkanBridge.m_swapChainImageViews);
//
//    createRenderPass();
//    createDescriptorSetLayout();
//    createGraphicsPipeline();
//    createCommandPool();
//
//    createDepthResources();
//    createFramebuffers();
//
//    createTextureImage();
//    createTextureImageView();
//    createTextureSampler();
//
//    loadModel();
//
//    createVertexBuffer();
//    createIndexBuffer();
//    createUniformBuffers();
//
//    createDescriptorPool();
//    createDescriptorSets();
//
//    createCommandBuffers();
//    createSyncObjects();
//}
//
//void renderLoop() {
//    while (!window.shouldClose()) {
//        window.poll();
//        drawFrame();
//    }
//    vkDeviceWaitIdle(vulkanBridge.m_logicalDevice);
//}
//
//void createRenderPass() {
//    VkAttachmentDescription colorAttachment{};
//    colorAttachment.format = vulkanBridge.m_swapChainImageFormat;
//    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    // Store rendered contents in memory to be read later
//    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
//
//    // Lookup stencil buffers later
//    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//
//    // The initialLayout specifies which layout the image will have before the render pass begins.
//    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//
//    /*
//        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
//        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
//        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Images to be used as destination for a memory copy operation
//
//        "The caveat of this special value is that the contents of the image are not guaranteed to be preserved,
//        but that doesn't matter since we're going to clear it anyway.
//        We want the image to be ready for presentation using the swap chain after rendering,
//        which is why we use VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as finalLayout."
//    */
//
//    // The finalLayout specifies the layout to automatically transition to when the render pass finishes. 
//    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
//
//    VkAttachmentReference colorAttachmentRef{};
//    colorAttachmentRef.attachment = 0;
//    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
//
//    VkAttachmentDescription depthAttachment{};
//    depthAttachment.format = findDepthFormat();
//    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
//    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
//    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
//    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
//    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//    VkAttachmentReference depthAttachmentRef{};
//    depthAttachmentRef.attachment = 1;
//    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
//
//    VkSubpassDescription subpass{};
//    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//    /*
//        pInputAttachments: Attachments that are read from a shader
//        pResolveAttachments: Attachments used for multisampling color attachments
//        pDepthStencilAttachment : Attachment for depthand stencil data
//        pPreserveAttachments : Attachments that are not used by this subpass, but for which the data must be preserved
//    */
//    // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive
//    subpass.colorAttachmentCount = 1;
//    subpass.pColorAttachments = &colorAttachmentRef;
//    subpass.pDepthStencilAttachment = &depthAttachmentRef;
//
//
//    VkSubpassDependency dependency{};
//    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
//    dependency.dstSubpass = 0;
//    dependency.srcAccessMask = 0;
//
//    // Creating render pass object and populating the struct with necessary properties
//    std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
//    VkRenderPassCreateInfo renderPassInfo{};
//    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//    renderPassInfo.pAttachments = attachments.data();
//    renderPassInfo.subpassCount = 1;
//    renderPassInfo.pSubpasses = &subpass;
//    renderPassInfo.dependencyCount = 1;
//    renderPassInfo.pDependencies = &dependency;
//
//    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//
//    if (vkCreateRenderPass(vulkanBridge.m_logicalDevice, &renderPassInfo, nullptr, &vulkanBridge.m_renderPass) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create render pass!");
//    }
//}
//
//void createDescriptorSetLayout() {
//
//    VkDescriptorSetLayoutBinding uboLayoutBinding{};
//    uboLayoutBinding.binding = 0;
//    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    uboLayoutBinding.descriptorCount = 1;
//    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
//    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional
//
//    VkDescriptorSetLayoutBinding samplerLayoutBinding{};
//    samplerLayoutBinding.binding = 1;
//    samplerLayoutBinding.descriptorCount = 1;
//    samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    samplerLayoutBinding.pImmutableSamplers = nullptr;
//    samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
//
//    std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };
//    VkDescriptorSetLayoutCreateInfo layoutInfo{};
//    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
//    layoutInfo.pBindings = bindings.data();
//
//    if (vkCreateDescriptorSetLayout(vulkanBridge.m_logicalDevice, &layoutInfo, nullptr, &vulkanBridge.m_descriptorSetLayout) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create descriptor set layout!");
//    }
//}
//
//void createGraphicsPipeline() {
//    std::vector<char> vertShaderCode = readFile("Modules/Renderer/Shaders/vert.spv");
//    std::vector<char> fragShaderCode = readFile("Modules/Renderer/Shaders/frag.spv");
//
//    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
//    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);
//    // CODE BELOW THIS
//    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
//    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
//
//    // IMPORTANT NOTE!!!
//    /*
//        The next two members specify the shader module containing the code, and the function to invoke, known as the entrypoint.
//        That means that it's possible to combine multiple fragment shaders into a single shader module and use different entry
//        points to differentiate between their behaviors. In this case we'll stick to the standard main, however.
//    */
//
//    // Shader module creation
//    vertShaderStageInfo.module = vertShaderModule;
//    vertShaderStageInfo.pName = "main";
//
//    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
//    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
//    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
//    fragShaderStageInfo.module = fragShaderModule;
//    fragShaderStageInfo.pName = "main";
//
//    VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
//
//
//    // Dynamic States of pipeline
//    std::vector<VkDynamicState> dynamicStates = {
//        VK_DYNAMIC_STATE_VIEWPORT,
//        VK_DYNAMIC_STATE_SCISSOR
//    };
//
//    VkPipelineDynamicStateCreateInfo dynamicState{};
//    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
//    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
//    dynamicState.pDynamicStates = dynamicStates.data();
//
//    // Passing vertex data into pipeline
//    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
//    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
//
//    auto bindingDescription = Vertex::getBindingDescription();
//    auto attributeDescriptions = Vertex::getAttributeDescriptions();
//
//
//    vertexInputInfo.vertexBindingDescriptionCount = 1;
//    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
//
//    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
//    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
//
//    // Geometry to be drawn from Vertices passed in
//    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
//    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
//    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
//    inputAssembly.primitiveRestartEnable = VK_FALSE;
//
//    // Viewports and Scissors
//
//    // The viewport takes the swapchain images and transforms them to framebuffers.
//    // Here we are setting the viewport to the swapchain images
//    VkViewport viewport{};
//    viewport.x = 0.0f;
//    viewport.y = 0.0f;
//    viewport.width = (float)vulkanBridge.m_swapChainExtent.width;
//    viewport.height = (float)vulkanBridge.m_swapChainExtent.height;
//    viewport.minDepth = 0.0f;
//    viewport.maxDepth = 1.0f;
//
//    // Determines which part of the framebuffers will be rasterized. 
//    // Here we set it to the swapchain images so we can see the whole framebuffer.
//    VkRect2D scissor{};
//    scissor.offset = { 0, 0 };
//    scissor.extent = vulkanBridge.m_swapChainExtent;
//
//    VkPipelineViewportStateCreateInfo viewportState{};
//    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
//    viewportState.viewportCount = 1;
//    viewportState.pViewports = &viewport;
//    viewportState.scissorCount = 1;
//    viewportState.pScissors = &scissor;
//
//
//    // Rasterizer
//    VkPipelineRasterizationStateCreateInfo rasterizer{};
//    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
//    // depthClampEnable : Whether or not things outside the near and far planes are discarded or clamped to them
//    rasterizer.depthClampEnable = VK_FALSE;
//    // rasterizerDiscardEnable : Allows geometry to pass through the rasterizer stage. 
//    // If disabled, disables output to framebuffer.
//    rasterizer.rasterizerDiscardEnable = VK_FALSE;
//    // VK_POLYGON_MODE_FILL = Fill in shape
//
//    // !! BELOW OPTIONS REQUIRE ENABLING GPU FEATURE !!
//    // VK_POLYGON_MODE_LINE = Wireframe
//    // VK_POLYGON_MODE_POINT = Vertices drawn as points
//    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
//    rasterizer.lineWidth = 1.0f;
//    // Backface culling enabled here
//    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
//    // Determines draw order of a "front" face
//    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
//    // !! Will learn about this in the future !!
//    rasterizer.depthBiasEnable = VK_FALSE;
//    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
//    rasterizer.depthBiasClamp = 0.0f; // Optional
//    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional
//
//
//    VkPipelineMultisampleStateCreateInfo multisampling{};
//    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
//    multisampling.sampleShadingEnable = VK_FALSE;
//    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
//    multisampling.minSampleShading = 1.0f; // Optional
//    multisampling.pSampleMask = nullptr; // Optional
//    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
//    multisampling.alphaToOneEnable = VK_FALSE; // Optional
//
//
//    VkPipelineDepthStencilStateCreateInfo depthStencil{};
//    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
//    depthStencil.depthTestEnable = VK_TRUE;
//    depthStencil.depthWriteEnable = VK_TRUE;
//    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
//    depthStencil.depthBoundsTestEnable = VK_FALSE;
//    depthStencil.minDepthBounds = 0.0f; // Optional
//    depthStencil.maxDepthBounds = 1.0f; // Optional
//    depthStencil.stencilTestEnable = VK_FALSE;
//    depthStencil.front = {}; // Optional
//    depthStencil.back = {}; // Optional
//
//    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
//    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
//    colorBlendAttachment.blendEnable = VK_TRUE;
//    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
//    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
//    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
//    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
//    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
//    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;
//
//    // REVIEW THIS. I DON'T KNOW WHAT THIS DOES BUT MY BRAIN IS FUZZY AND I DON'T CARE TO LEARN.
//    VkPipelineColorBlendStateCreateInfo colorBlending{};
//    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
//    colorBlending.logicOpEnable = VK_FALSE;
//    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
//    colorBlending.attachmentCount = 1;
//    colorBlending.pAttachments = &colorBlendAttachment;
//    colorBlending.blendConstants[0] = 0.0f; // Optional
//    colorBlending.blendConstants[1] = 0.0f; // Optional
//    colorBlending.blendConstants[2] = 0.0f; // Optional
//    colorBlending.blendConstants[3] = 0.0f; // Optional
//
//    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
//    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
//    pipelineLayoutInfo.setLayoutCount = 1;
//    pipelineLayoutInfo.pSetLayouts = &vulkanBridge.m_descriptorSetLayout;
//    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
//    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
//
//    if (vkCreatePipelineLayout(vulkanBridge.m_logicalDevice, &pipelineLayoutInfo, nullptr, &vulkanBridge.m_pipelineLayout) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create pipeline layout!");
//    };
//
//    // Finally creating graphics pipeline object and populating with necessary information
//    VkGraphicsPipelineCreateInfo pipelineInfo{};
//    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
//    pipelineInfo.stageCount = 2;
//    pipelineInfo.pStages = shaderStages;
//    pipelineInfo.pVertexInputState = &vertexInputInfo;
//    pipelineInfo.pInputAssemblyState = &inputAssembly;
//    pipelineInfo.pViewportState = &viewportState;
//    pipelineInfo.pRasterizationState = &rasterizer;
//    pipelineInfo.pMultisampleState = &multisampling;
//    pipelineInfo.pDepthStencilState = &depthStencil;
//    pipelineInfo.pColorBlendState = &colorBlending;
//    pipelineInfo.pDynamicState = &dynamicState;
//    pipelineInfo.layout = vulkanBridge.m_pipelineLayout;
//    pipelineInfo.renderPass = vulkanBridge.m_renderPass;
//    // Subpass INDEX
//    pipelineInfo.subpass = 0;
//    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
//    pipelineInfo.basePipelineIndex = -1; // Optional
//
//    if (vkCreateGraphicsPipelines(vulkanBridge.m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vulkanBridge.m_graphicsPipeline) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create graphics pipeline!");
//    }
//
//    // CODE ABOVE THIS
//    vkDestroyShaderModule(vulkanBridge.m_logicalDevice, fragShaderModule, nullptr);
//    vkDestroyShaderModule(vulkanBridge.m_logicalDevice, vertShaderModule, nullptr);
//}
//
//void createCommandPool() {
//    QueueFamilyIndices queueFamilyIndices = vulkanBridge.init.findQueueFamilies(vulkanBridge.m_physicalDevice);
//
//    VkCommandPoolCreateInfo poolInfo{};
//    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
//    // Selecting graphicsFamily in order to issue draw commands in this command pool
//    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
//
//    if (vkCreateCommandPool(vulkanBridge.m_logicalDevice, &poolInfo, nullptr, &vulkanBridge.m_commandPool) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create command pool!");
//    }
//}
//
//void createFramebuffers() {
//    vulkanBridge.m_swapChainFramebuffers.resize(vulkanBridge.m_swapChainImageViews.size());
//
//    // Loop through swap chain image views
//    for (size_t i = 0; i < vulkanBridge.m_swapChainImageViews.size(); i++) {
//        std::array<VkImageView, 2> attachments = {
//            vulkanBridge.m_swapChainImageViews[i],
//            vulkanBridge.m_depthImageView
//        };
//
//        VkFramebufferCreateInfo framebufferInfo{};
//        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
//        framebufferInfo.renderPass = vulkanBridge.m_renderPass;
//        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
//        framebufferInfo.pAttachments = attachments.data();
//        framebufferInfo.pAttachments = attachments.data();
//        framebufferInfo.width = vulkanBridge.m_swapChainExtent.width;
//        framebufferInfo.height = vulkanBridge.m_swapChainExtent.height;
//        framebufferInfo.layers = 1;
//
//        if (vkCreateFramebuffer(vulkanBridge.m_logicalDevice, &framebufferInfo, nullptr, &vulkanBridge.m_swapChainFramebuffers[i]) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create framebuffer!");
//        }
//    }
//}
//
//void createDepthResources() {
//    VkFormat depthFormat = findDepthFormat();
//    createImage(vulkanBridge.m_swapChainExtent.width, vulkanBridge.m_swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
//        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//        vulkanBridge.m_depthImage, vulkanBridge.m_depthImageMemory);
//    vulkanBridge.m_depthImageView = vulkanBridge.imageView.create(vulkanBridge.m_logicalDevice, vulkanBridge.m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
//
//    transitionImageLayout(vulkanBridge.m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
//}
//
//void createTextureImage() {
//    int texWidth, texHeight, texChannels;
//
//    stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
//
//    // The multiplication by 4 here is because the pixels are loaded row by row with 4 bytes per pixel
//    VkDeviceSize imageSize = texWidth * texHeight * 4;
//
//    if (!pixels) {
//        throw std::runtime_error("failed to load texture image!");
//    }
//
//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//
//    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//        stagingBuffer, stagingBufferMemory);
//
//    void* data;
//    vkMapMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
//    memcpy(data, pixels, static_cast<size_t>(imageSize));
//    vkUnmapMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory);
//
//    stbi_image_free(pixels);
//
//    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
//        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
//        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanBridge.m_textureImage, vulkanBridge.m_textureImageMemory);
//
//    transitionImageLayout(vulkanBridge.m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
//    copyBufferToImage(stagingBuffer, vulkanBridge.m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
//    transitionImageLayout(vulkanBridge.m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
//
//    vkDestroyBuffer(vulkanBridge.m_logicalDevice, stagingBuffer, nullptr);
//    vkFreeMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory, nullptr);
//}
//
//// TEXTURE IMAGE VIEW
//void createTextureImageView() {
//    vulkanBridge.m_textureImageView = vulkanBridge.imageView.create(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
//}
//
//// TEXTURE SAMPLER
//void createTextureSampler() {
//    VkSamplerCreateInfo samplerInfo{};
//    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
//    samplerInfo.magFilter = VK_FILTER_LINEAR;
//    samplerInfo.minFilter = VK_FILTER_LINEAR;
//    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
//
//    samplerInfo.anisotropyEnable = VK_TRUE;
//
//    VkPhysicalDeviceProperties properties{};
//    vkGetPhysicalDeviceProperties(vulkanBridge.m_physicalDevice, &properties);
//    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
//
//    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
//    samplerInfo.unnormalizedCoordinates = VK_FALSE;
//
//    samplerInfo.compareEnable = VK_FALSE;
//    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
//
//    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
//    samplerInfo.mipLodBias = 0.0f;
//    samplerInfo.minLod = 0.0f;
//    samplerInfo.maxLod = 0.0f;
//
//    if (vkCreateSampler(vulkanBridge.m_logicalDevice, &samplerInfo, nullptr, &vulkanBridge.m_textureSampler) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create texture sampler!");
//    }
//}
//
//void loadModel() {
//    tinyobj::attrib_t attrib;
//
//
//    // List: {0, 1, 2, 3}
//
//    // List(5): {0, 1, 2, 3, 4}
//
//    // Vector (5): {0, 1, 2, 3, 4} + {4564565}
//    // resize
//    // Vector (6) : {0, 1, 2, 3, 4, 4564565}
//
//    std::vector<tinyobj::shape_t> shapes;
//    std::vector<tinyobj::material_t> materials;
//    std::string warn, err;
//
//    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
//        throw std::runtime_error(warn + err);
//    }
//
//    std::unordered_map<Vertex, uint32_t> uniqueVertices{};
//
//    for (const auto& shape : shapes) {
//        for (const auto& index : shape.mesh.indices) {
//            Vertex vertex{};
//
//            vertex.pos = {
//                attrib.vertices[3 * index.vertex_index + 0],
//                attrib.vertices[3 * index.vertex_index + 1],
//                attrib.vertices[3 * index.vertex_index + 2]
//            };
//
//            vertex.texCoord = {
//                attrib.texcoords[2 * index.texcoord_index + 0],
//                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
//            };
//
//
//
//            if (uniqueVertices.count(vertex) == 0) {
//                uniqueVertices[vertex] = static_cast<uint32_t>(vulkanBridge.m_vertices.size());
//                vulkanBridge.m_vertices.push_back(vertex);
//            }
//            vulkanBridge.m_indices.push_back(uniqueVertices[vertex]);
//
//            vertex.color = { 1.0f, 1.0f, 1.0f };
//        }
//    }
//}
//
//// VERTEX BUFFER
//void createVertexBuffer() {
//
//    // TODO: Go over staging buffers and why the hell we need them
//
//    VkDeviceSize bufferSize = sizeof(vulkanBridge.m_vertices[0]) * vulkanBridge.m_vertices.size();
//
//    // Create staging buffer
//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//    createBuffer(bufferSize,
//        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
//        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//        stagingBuffer, stagingBufferMemory);
//
//    // Map it
//    void* data;
//    vkMapMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
//    memcpy(data, vulkanBridge.m_vertices.data(), (size_t)bufferSize);
//    vkUnmapMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory);
//
//    // Create vertex buffer
//    createBuffer(bufferSize,
//        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
//        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
//        vulkanBridge.m_vertexBuffer, vulkanBridge.m_vertexBufferMemory);
//
//    // Copy data from staging buffer to vertex buffer
//    copyBuffer(stagingBuffer, vulkanBridge.m_vertexBuffer, bufferSize);
//
//    // cleanup
//    vkDestroyBuffer(vulkanBridge.m_logicalDevice, stagingBuffer, nullptr);
//    vkFreeMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory, nullptr);
//}
//
//// INDEX BUFFER
//void createIndexBuffer() {
//    VkDeviceSize bufferSize = sizeof(vulkanBridge.m_indices[0]) * vulkanBridge.m_indices.size();
//
//    VkBuffer stagingBuffer;
//    VkDeviceMemory stagingBufferMemory;
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
//
//    void* data;
//    vkMapMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
//    memcpy(data, vulkanBridge.m_indices.data(), (size_t)bufferSize);
//    vkUnmapMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory);
//
//    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanBridge.m_indexBuffer, vulkanBridge.m_indexBufferMemory);
//
//    copyBuffer(stagingBuffer, vulkanBridge.m_indexBuffer, bufferSize);
//
//    vkDestroyBuffer(vulkanBridge.m_logicalDevice, stagingBuffer, nullptr);
//    vkFreeMemory(vulkanBridge.m_logicalDevice, stagingBufferMemory, nullptr);
//}
//
//// UNIFORM BUFFERS
//void createUniformBuffers() {
//    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
//
//    vulkanBridge.m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
//    vulkanBridge.m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
//    vulkanBridge.m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
//
//    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vulkanBridge.m_uniformBuffers[i], vulkanBridge.m_uniformBuffersMemory[i]);
//        vkMapMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_uniformBuffersMemory[i], 0, bufferSize, 0, &vulkanBridge.m_uniformBuffersMapped[i]);
//    }
//}
//
//// DESCRIPTOR POOL
//void createDescriptorPool() {
//
//    /*
//        Incorrectly sized descriptor pools could result in errors on some devices but not others depending on the GPU drivers
//        Some will let you get away with bad allocations, some won't. Be careful.
//    */
//
//    std::array<VkDescriptorPoolSize, 2> poolSizes{};
//    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//
//    VkDescriptorPoolCreateInfo poolInfo{};
//    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
//    poolInfo.poolSizeCount = static_cast<uint32_t> (poolSizes.size());
//    poolInfo.pPoolSizes = poolSizes.data();
//    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//
//    if (vkCreateDescriptorPool(vulkanBridge.m_logicalDevice, &poolInfo, nullptr, &vulkanBridge.m_descriptorPool) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create descriptor pool!");
//    }
//}
//
//// DESCRIPTOR SETS
//void createDescriptorSets() {
//    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, vulkanBridge.m_descriptorSetLayout);
//    VkDescriptorSetAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//    allocInfo.descriptorPool = vulkanBridge.m_descriptorPool;
//    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
//    allocInfo.pSetLayouts = layouts.data();
//
//    vulkanBridge.m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
//    if (vkAllocateDescriptorSets(vulkanBridge.m_logicalDevice, &allocInfo, vulkanBridge.m_descriptorSets.data()) != VK_SUCCESS) {
//        throw std::runtime_error("failed to allocate descriptor sets!");
//    }
//
//    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//        VkDescriptorBufferInfo bufferInfo{};
//        bufferInfo.buffer = vulkanBridge.m_uniformBuffers[i];
//        bufferInfo.offset = 0;
//        bufferInfo.range = sizeof(UniformBufferObject);
//
//        VkDescriptorImageInfo imageInfo{};
//        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
//        imageInfo.imageView = vulkanBridge.m_textureImageView;
//        imageInfo.sampler = vulkanBridge.m_textureSampler;
//
//        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
//        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//        descriptorWrites[0].dstSet = vulkanBridge.m_descriptorSets[i];
//        descriptorWrites[0].dstBinding = 0;
//        descriptorWrites[0].dstArrayElement = 0;
//        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//        descriptorWrites[0].descriptorCount = 1;
//        descriptorWrites[0].pBufferInfo = &bufferInfo;
//        descriptorWrites[0].pImageInfo = nullptr; // Optional
//        descriptorWrites[0].pTexelBufferView = nullptr; // Optional
//
//        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//        descriptorWrites[1].dstSet = vulkanBridge.m_descriptorSets[i];
//        descriptorWrites[1].dstBinding = 1;
//        descriptorWrites[1].dstArrayElement = 0;
//        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
//        descriptorWrites[1].descriptorCount = 1;
//        descriptorWrites[1].pImageInfo = &imageInfo;
//
//
//        vkUpdateDescriptorSets(vulkanBridge.m_logicalDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
//    }
//
//
//}
//
//void createCommandBuffers() {
//    vulkanBridge.m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
//
//    VkCommandBufferAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    allocInfo.commandPool = vulkanBridge.m_commandPool;
//    /*
//        VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
//        VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
//    */
//    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    allocInfo.commandBufferCount = (uint32_t)vulkanBridge.m_commandBuffers.size();
//
//    if (vkAllocateCommandBuffers(vulkanBridge.m_logicalDevice, &allocInfo, vulkanBridge.m_commandBuffers.data()) != VK_SUCCESS) {
//        throw std::runtime_error("failed to allocate command buffers!");
//    }
//}
//
//void createSyncObjects() {
//    vulkanBridge.m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
//    vulkanBridge.m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
//    vulkanBridge.m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
//
//
//    VkSemaphoreCreateInfo semaphoreInfo{};
//    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
//
//    VkFenceCreateInfo fenceInfo{};
//    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
//    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
//
//    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//        if (vkCreateSemaphore(vulkanBridge.m_logicalDevice, &semaphoreInfo, nullptr, &vulkanBridge.m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
//            vkCreateSemaphore(vulkanBridge.m_logicalDevice, &semaphoreInfo, nullptr, &vulkanBridge.m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
//            vkCreateFence(vulkanBridge.m_logicalDevice, &fenceInfo, nullptr, &vulkanBridge.m_inFlightFences[i]) != VK_SUCCESS) {
//            throw std::runtime_error("failed to create semaphores!");
//        }
//    }
//}
//
//void drawFrame() {
//    /*
//        - Wait for the previous frame to finish
//        - Acquire an image from the swap chain
//        - Record a command buffer which draws the scene onto that image
//        - Submit the recorded command buffer
//        - Present the swap chain image
//    */
//    // Wait for the previous frame to finish
//
//    updateUniformBuffer(vulkanBridge.m_currentFrame);
//    vkWaitForFences(vulkanBridge.m_logicalDevice, 1, &vulkanBridge.m_inFlightFences[vulkanBridge.m_currentFrame], VK_TRUE, UINT64_MAX);
//
//    // Acquire an image from the swap chain
//    uint32_t imageIndex;
//
//    // SWAP CHAIN RECREATION
//    VkResult result = vkAcquireNextImageKHR(vulkanBridge.m_logicalDevice, vulkanBridge.m_swapChain, UINT64_MAX, vulkanBridge.m_imageAvailableSemaphores[vulkanBridge.m_currentFrame], VK_NULL_HANDLE, &imageIndex);
//    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
//        recreateSwapChain();
//        return;
//    }
//    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
//        throw std::runtime_error("failed to acquire swap chain image!");
//    }
//
//    // Only reset fences if we are submitting work
//    vkResetFences(vulkanBridge.m_logicalDevice, 1, &vulkanBridge.m_inFlightFences[vulkanBridge.m_currentFrame]);
//
//    // Record a command buffer which draws the scene onto that image
//    vkResetCommandBuffer(vulkanBridge.m_commandBuffers[vulkanBridge.m_currentFrame], 0);
//    recordCommandBuffer(vulkanBridge.m_commandBuffers[vulkanBridge.m_currentFrame], imageIndex);
//
//    // Submit the recorded command buffer
//    VkSubmitInfo submitInfo{};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//
//    VkSemaphore waitSemaphores[] = { vulkanBridge.m_imageAvailableSemaphores[vulkanBridge.m_currentFrame] };
//    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
//    submitInfo.waitSemaphoreCount = 1;
//    submitInfo.pWaitSemaphores = waitSemaphores;
//    submitInfo.pWaitDstStageMask = waitStages;
//
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &vulkanBridge.m_commandBuffers[vulkanBridge.m_currentFrame];
//
//    VkSemaphore signalSemaphores[] = { vulkanBridge.m_renderFinishedSemaphores[vulkanBridge.m_currentFrame] };
//    submitInfo.signalSemaphoreCount = 1;
//    submitInfo.pSignalSemaphores = signalSemaphores;
//
//    if (vkQueueSubmit(vulkanBridge.m_graphicsQueue, 1, &submitInfo, vulkanBridge.m_inFlightFences[vulkanBridge.m_currentFrame]) != VK_SUCCESS) {
//        throw std::runtime_error("failed to submit draw command buffer!");
//    }
//
//    // Present swap chain image
//    VkPresentInfoKHR presentInfo{};
//    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
//
//    presentInfo.waitSemaphoreCount = 1;
//    presentInfo.pWaitSemaphores = signalSemaphores;
//
//    VkSwapchainKHR swapChains[] = { vulkanBridge.m_swapChain };
//    presentInfo.swapchainCount = 1;
//    presentInfo.pSwapchains = swapChains;
//
//    presentInfo.pImageIndices = &imageIndex;
//    //presentInfo.pResults = nullptr; // Optional
//
//    // SWAP CHAIN RECREATION
//    result = vkQueuePresentKHR(vulkanBridge.m_presentQueue, &presentInfo);
//
//    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.framebufferResized) {
//        window.framebufferResized = false;
//        recreateSwapChain();
//    }
//    else if (result != VK_SUCCESS) {
//        throw std::runtime_error("failed to present swap chain image!");
//    }
//
//    // Progress to next frame
//    vulkanBridge.m_currentFrame = (vulkanBridge.m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
//}
//
//void recreateSwapChain() {
//    // Handling minimization
//    window.handleMinimization();
//
//    vkDeviceWaitIdle(vulkanBridge.m_logicalDevice);
//
//    destroySwapChain();
//
//    vulkanBridge.swapChain.create(vulkanBridge.init, window);
//    vulkanBridge.swapChain.createImageViews(vulkanBridge.m_logicalDevice, vulkanBridge.imageView);
//    vulkanBridge.swapChain.assign(&vulkanBridge.m_swapChain, &vulkanBridge.m_swapChainImageFormat, &vulkanBridge.m_swapChainExtent, &vulkanBridge.m_swapChainImageViews);
//
//    createDepthResources();
//    createFramebuffers();
//}
//
//
//void destroySwapChain() {
//    vkDestroyImageView(vulkanBridge.m_logicalDevice, vulkanBridge.m_depthImageView, nullptr);
//    vkDestroyImage(vulkanBridge.m_logicalDevice, vulkanBridge.m_depthImage, nullptr);
//    vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_depthImageMemory, nullptr);
//
//    for (auto framebuffer : vulkanBridge.m_swapChainFramebuffers) {
//        vkDestroyFramebuffer(vulkanBridge.m_logicalDevice, framebuffer, nullptr);
//    }
//
//    for (auto imageView : vulkanBridge.m_swapChainImageViews) {
//        vkDestroyImageView(vulkanBridge.m_logicalDevice, imageView, nullptr);
//    }
//
//    vkDestroySwapchainKHR(vulkanBridge.m_logicalDevice, vulkanBridge.m_swapChain, nullptr);
//}
//
//void Destroy() {
//    destroySwapChain();
//
//    vkDestroySampler(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureSampler, nullptr);
//    vkDestroyImageView(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureImageView, nullptr);
//    vkDestroyImage(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureImage, nullptr);
//    vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_textureImageMemory, nullptr);
//
//    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//        vkDestroyBuffer(vulkanBridge.m_logicalDevice, vulkanBridge.m_uniformBuffers[i], nullptr);
//        vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_uniformBuffersMemory[i], nullptr);
//    }
//
//    vkDestroyDescriptorPool(vulkanBridge.m_logicalDevice, vulkanBridge.m_descriptorPool, nullptr);
//
//    vkDestroyDescriptorSetLayout(vulkanBridge.m_logicalDevice, vulkanBridge.m_descriptorSetLayout, nullptr);
//
//    // Buffer cleanup
//    vkDestroyBuffer(vulkanBridge.m_logicalDevice, vulkanBridge.m_indexBuffer, nullptr);
//    vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_indexBufferMemory, nullptr);
//    vkDestroyBuffer(vulkanBridge.m_logicalDevice, vulkanBridge.m_vertexBuffer, nullptr);
//    vkFreeMemory(vulkanBridge.m_logicalDevice, vulkanBridge.m_vertexBufferMemory, nullptr);
//
//    vkDestroyPipeline(vulkanBridge.m_logicalDevice, vulkanBridge.m_graphicsPipeline, nullptr);
//    vkDestroyPipelineLayout(vulkanBridge.m_logicalDevice, vulkanBridge.m_pipelineLayout, nullptr);
//
//    vkDestroyRenderPass(vulkanBridge.m_logicalDevice, vulkanBridge.m_renderPass, nullptr);
//
//    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
//        vkDestroySemaphore(vulkanBridge.m_logicalDevice, vulkanBridge.m_renderFinishedSemaphores[i], nullptr);
//        vkDestroySemaphore(vulkanBridge.m_logicalDevice, vulkanBridge.m_imageAvailableSemaphores[i], nullptr);
//        vkDestroyFence(vulkanBridge.m_logicalDevice, vulkanBridge.m_inFlightFences[i], nullptr);
//    }
//    vkDestroyCommandPool(vulkanBridge.m_logicalDevice, vulkanBridge.m_commandPool, nullptr);
//
//    // DEVICE DESTRUCTION
//    vulkanBridge.init.destroy();
//    // GLFW DESTRUCTION
//    window.destroy();
//}
//
//
//// Helpers
//
//void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
//    VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
//
//    // Creation
//    VkBufferCreateInfo bufferInfo{};
//    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
//    bufferInfo.size = size;
//    bufferInfo.usage = usage;
//    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//    if (vkCreateBuffer(vulkanBridge.m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
//        throw std::runtime_error("failed to create vertex buffer!");
//    }
//
//    // Allocation
//    VkMemoryRequirements memRequirements;
//    vkGetBufferMemoryRequirements(vulkanBridge.m_logicalDevice, buffer, &memRequirements);
//
//    VkMemoryAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
//    allocInfo.allocationSize = memRequirements.size;
//    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
//
//    if (vkAllocateMemory(vulkanBridge.m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
//        throw std::runtime_error("failed to allocate buffer memory!");
//    }
//
//    // Binding
//    vkBindBufferMemory(vulkanBridge.m_logicalDevice, buffer, bufferMemory, 0);
//}
//
//// used by findDepthFormat()
//VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
//    for (VkFormat format : candidates) {
//        VkFormatProperties props;
//        vkGetPhysicalDeviceFormatProperties(vulkanBridge.m_physicalDevice, format, &props);
//        if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
//            return format;
//        }
//        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
//            return format;
//        }
//    }
//    throw std::runtime_error("failed to find supported format!");
//}
//
//VkFormat findDepthFormat() {
//    return findSupportedFormat(
//        { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
//        VK_IMAGE_TILING_OPTIMAL,
//        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
//    );
//}
//
//bool hasStencilComponent(VkFormat format) {
//    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
//}
//
//void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
//    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//    VkBufferImageCopy region{};
//    region.bufferOffset = 0;
//    region.bufferRowLength = 0;
//    region.bufferImageHeight = 0;
//
//    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    region.imageSubresource.mipLevel = 0;
//    region.imageSubresource.baseArrayLayer = 0;
//    region.imageSubresource.layerCount = 1;
//
//    region.imageOffset = { 0, 0, 0 };
//    region.imageExtent = {
//        width,
//        height,
//        1
//    };
//    vkCmdCopyBufferToImage(
//        commandBuffer,
//        buffer,
//        image,
//        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
//        1,
//        &region
//    );
//    endSingleTimeCommands(commandBuffer);
//
//}
//
//// beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
//VkCommandBuffer beginSingleTimeCommands() {
//    VkCommandBufferAllocateInfo allocInfo{};
//    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//    allocInfo.commandPool = vulkanBridge.m_commandPool;
//    allocInfo.commandBufferCount = 1;
//
//    VkCommandBuffer commandBuffer;
//    vkAllocateCommandBuffers(vulkanBridge.m_logicalDevice, &allocInfo, &commandBuffer);
//
//    VkCommandBufferBeginInfo beginInfo{};
//    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//    vkBeginCommandBuffer(commandBuffer, &beginInfo);
//
//    return commandBuffer;
//}
//
//void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
//    vkEndCommandBuffer(commandBuffer);
//
//    VkSubmitInfo submitInfo{};
//    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
//    submitInfo.commandBufferCount = 1;
//    submitInfo.pCommandBuffers = &commandBuffer;
//
//    vkQueueSubmit(vulkanBridge.m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
//    vkQueueWaitIdle(vulkanBridge.m_graphicsQueue);
//
//    vkFreeCommandBuffers(vulkanBridge.m_logicalDevice, vulkanBridge.m_commandPool, 1, &commandBuffer);
//}
//
//void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
//    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//    VkImageMemoryBarrier barrier{};
//    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
//    barrier.oldLayout = oldLayout;
//    barrier.newLayout = newLayout;
//    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
//    barrier.image = image;
//    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
//        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
//
//        if (hasStencilComponent(format)) {
//            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
//        }
//    }
//    else {
//        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
//    }
//    barrier.subresourceRange.baseMipLevel = 0;
//    barrier.subresourceRange.levelCount = 1;
//    barrier.subresourceRange.baseArrayLayer = 0;
//    barrier.subresourceRange.layerCount = 1;
//    barrier.srcAccessMask = 0; // TODO
//    barrier.dstAccessMask = 0; // TODO
//
//    VkPipelineStageFlags sourceStage;
//    VkPipelineStageFlags destinationStage;
//
//    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
//        barrier.srcAccessMask = 0;
//        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//
//        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//    }
//    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
//        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
//        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
//
//        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
//        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
//    }
//    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
//        barrier.srcAccessMask = 0;
//        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
//
//        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
//        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
//    }
//    else {
//        throw std::invalid_argument("unsupported layout transition!");
//    }
//
//    // Syncronization 
//    vkCmdPipelineBarrier(
//        commandBuffer,
//        sourceStage, destinationStage,
//        0,
//        0, nullptr,
//        0, nullptr,
//        1, &barrier
//    );
//    endSingleTimeCommands(commandBuffer);
//}
//
//// COPY BUFFER
//void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
//    VkCommandBuffer commandBuffer = beginSingleTimeCommands();
//    VkBufferCopy copyRegion{};
//    copyRegion.size = size;
//    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
//    endSingleTimeCommands(commandBuffer);
//}
//
//uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
//    VkPhysicalDeviceMemoryProperties memProperties;
//    vkGetPhysicalDeviceMemoryProperties(vulkanBridge.m_physicalDevice, &memProperties);
//
//    // Go over this section
//    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
//        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
//            return i;
//        }
//    }
//
//    throw std::runtime_error("failed to find suitable memory type!");
//}