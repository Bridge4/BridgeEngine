#include <vulkan/vulkan_core.h>

#include <cassert>

#include "Source/Renderer/API/Vulkan/Components/DescriptorSets/DescriptorSetHandler.h"
#include "config.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "VulkanContext.h"
// Textures through stb library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vector>

#include "Components/Buffers/BufferHandler.h"
#include "Components/Devices/DeviceHandler.h"
#include "Components/Images/ImageHandler.h"
#include "Components/RenderPass/RenderPassHandler.h"
#include "Components/SwapChain/SwapChainHandler.h"
#include "Components/VulkanGlobalState/VulkanGlobalState.h"
#include "Components/Window/WindowHandler.h"
#include "Source/Renderer/Camera/CameraController.h"
#include "Source/Renderer/Mesh/Mesh3D.h"
#include "VkTypes.h"

void VulkanContext::InitVulkan() {
    m_vulkanGlobalState = new VulkanGlobalState(this);
    m_descriptorSetHandler = new DescriptorSetHandler(m_vulkanGlobalState);

    m_deviceHandler = new DeviceHandler(m_vulkanGlobalState);
    m_imageHandler = new ImageHandler(this, m_vulkanGlobalState);
    m_swapChainHandler =
        new SwapChainHandler(this, m_deviceHandler, m_windowHandler,
                             m_imageHandler, m_vulkanGlobalState);
    m_bufferHandler = new BufferHandler(m_vulkanGlobalState);
    m_renderPassHandler =
        new RenderPassHandler(this, m_swapChainHandler, m_deviceHandler,
                              m_imageHandler, m_vulkanGlobalState);
    m_swapChainHandler->m_renderPassHandler = m_renderPassHandler;
    m_cameraController =
        new CameraController(this, m_windowHandler, m_swapChainHandler,
                             m_bufferHandler, m_vulkanGlobalState);

    m_vulkanGlobalState->CreateVulkanInstance();
    m_windowHandler->vulkanContext = this;
    if (m_windowHandler->CreateSurface() != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
    m_deviceHandler->Initialize();
    m_swapChainHandler->Initialize();

    m_renderPassHandler->CreateRenderPass();
    m_renderPassHandler->CreateShadowPass();

    CreateCommandPool();
    m_renderPassHandler->CreateRenderPassFrameBuffers();

    m_descriptorSetHandler->CreateDescriptorPool();
    // SceneDescriptorSets
    m_descriptorSetHandler->CreateSceneDescriptorSetLayout();
    m_bufferHandler->CreateCameraUBO();
    m_bufferHandler->CreateLightUBO();
    m_descriptorSetHandler->CreateSceneDescriptorSets();

    std::vector<char> vertShadow = ReadFile(SHADERS_DIR "shadow.spv");
    std::vector<char> fragShadow = ReadFile(SHADERS_DIR "shadowFrag.spv");
    CreateShadowPassPipeline(vertShadow, fragShadow,
                             &m_vulkanGlobalState->m_shadowPassPipeline);
    CreateTextureSamplerShadowPass();

    // Create Per-Mesh Descriptor Set Layout
    m_descriptorSetHandler->CreateTexturedPBRDescriptorSetLayout();
    std::vector<char> vertScene = ReadFile(SHADERS_DIR "scene.spv");
    std::vector<char> fragTextured = ReadFile(SHADERS_DIR "pbr.spv");
    CreatePBRPipeline(vertScene, fragTextured,
                      &m_vulkanGlobalState->m_pbrPipeline);

    m_bufferHandler->CreateCommandBuffers();
    CreateSyncObjects();
}

void VulkanContext::RunVulkanRenderer(
    std::vector<LoadedObject> objectsToRender) {
    std::chrono::high_resolution_clock::time_point lastFrameTime;
    lastFrameTime = std::chrono::high_resolution_clock::now();

    m_objectsToRender = objectsToRender;

    Light light0;
    light0.position = glm::vec4(10.0f, 20.0f, 10.0f, 1.0f);
    light0.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light0.intensity.x = 10000.0f;

    // Light light1;
    // light1.position = glm::vec4(0.0f, 10.0f, 0.0f, 0.0f);
    // light1.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    // light1.intensity.x = 1.5f;
    // Light light2;
    // light2.position = glm::vec4(-100.0f, 0.0f, 0.0f, 0.0f);
    // light2.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    // light2.intensity.x = 10000.0f;
    // Light light3;
    // light3.position = glm::vec4(0.0f, 0.0f, 10.0f, 0.0f);
    // light3.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    // light3.intensity.x = 10.0f;
    LightUBO lightUBO;
    lightUBO.lights[0] = light0;
    // lightUBO.lights[1] = light1;
    // lightUBO.lights[2] = light2;
    // lightUBO.lights[3] = light3;
    lightUBO.numLights.x = 1;
    m_vulkanGlobalState->m_lights = lightUBO;
    // need create a shadowpass framebuffer per shadow casting light
    m_renderPassHandler->CreateShadowPassFrameBuffers();
    LoadSceneObjects();
    m_vulkanGlobalState->m_shadowBias = glm::vec4(0.0005f, 0.0f, 0.0f, 0.0f);
    m_vulkanGlobalState->m_pbrPushConstants = {
        glm::mat4(1.0f), m_vulkanGlobalState->m_shadowBias};

    m_vulkanGlobalState->m_pbrPushConstants.hasEmissive =
        glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
    while (!m_windowHandler->ShouldClose()) {
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime =
            std::chrono::duration<float>(currentFrameTime - lastFrameTime)
                .count();
        lastFrameTime = currentFrameTime;
        m_windowHandler->Poll();
        m_cameraController->HandleInputOrbit(deltaTime);
        memcpy(m_vulkanGlobalState
                   ->m_lightUBOMapped[m_vulkanGlobalState->m_currentFrame],
               &m_vulkanGlobalState->m_lights,
               sizeof(m_vulkanGlobalState->m_lights));
        m_cameraController->UpdateCameraUBO(m_vulkanGlobalState->m_currentFrame,
                                            deltaTime);

        DrawFrame(deltaTime);
    }
    vkDeviceWaitIdle(*m_vulkanGlobalState->GetRefLogicalDevice());
    Destroy();
}

void VulkanContext::LoadSceneObjects() {
    int meshCount = 0;
    float xPos = 0.0f;
    for (auto& obj : m_objectsToRender) {
        LoadMesh(obj.objProperties, glm::vec3(xPos, 0.0f, 0.0f),
                 glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f, 5.0f, 5.0f));
        std::vector<MaterialEnums> materialTypes = {ALBEDO, METALLIC, ROUGHNESS,
                                                    AO,     NORMAL,   EMISSIVE};
        for (auto& matType : materialTypes) {
            CreateTextureImage(obj.textureProperties[matType],
                               &m_vulkanGlobalState->m_meshList[meshCount],
                               matType);
            CreateTextureImageView(&m_vulkanGlobalState->m_meshList[meshCount],
                                   matType);
            CreateTextureSampler(&m_vulkanGlobalState->m_meshList[meshCount],
                                 matType);
        }

        // Create Mesh Uniform Buffers
        m_bufferHandler->CreateModelUBO(
            &m_vulkanGlobalState->m_meshList[meshCount]);
        m_descriptorSetHandler->CreateTexturedPBRDescriptorSets(
            &m_vulkanGlobalState->m_meshList[meshCount]);
        meshCount++;
        xPos += 5.0f;
    }
    m_bufferHandler->CreateVertexBuffer(m_vertices);
    m_bufferHandler->CreateIndexBuffer(m_indices);
}

void VulkanContext::UnloadSceneObjects() {
    if (!m_vulkanGlobalState->m_meshList.empty()) {
        vkQueueWaitIdle(m_vulkanGlobalState->m_presentQueue);
        vkDestroyDescriptorPool(*m_vulkanGlobalState->GetRefLogicalDevice(),
                                m_vulkanGlobalState->m_descriptorPool, nullptr);
        for (auto& mesh : m_vulkanGlobalState->m_meshList) {
            vkDestroySampler(*m_vulkanGlobalState->GetRefLogicalDevice(),
                             mesh.m_textureSampler, nullptr);
            vkDestroyImageView(*m_vulkanGlobalState->GetRefLogicalDevice(),
                               mesh.m_textureImageView, nullptr);
            vkDestroyImage(*m_vulkanGlobalState->GetRefLogicalDevice(),
                           mesh.m_textureImage, nullptr);
            vkFreeMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
                         mesh.m_textureImageMemory, nullptr);
            int bufCount = 0;
            for (auto& buffer : mesh.m_uniformBuffers) {
                vkDestroyBuffer(*m_vulkanGlobalState->GetRefLogicalDevice(),
                                buffer, nullptr);
                vkFreeMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
                             mesh.m_uniformBuffersMemory[bufCount], nullptr);
                bufCount++;
            }
        }
        m_bufferHandler->DestroyBuffers();
        m_vulkanGlobalState->m_meshList = {};
    } else {
        std::cout << "No Scene Objects to Unload\n";
    }
}

void VulkanContext::CreatePBRPipeline(std::vector<char> vertShaderCode,
                                      std::vector<char> fragShaderCode,
                                      VkPipeline* pipeline) {
    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    // IMPORTANT NOTE!!!
    /*
        The next two members specify the shader module containing the code, and
       the function to invoke, known as the entrypoint. That means that it's
       possible to combine multiple fragment shaders into a single shader module
       and use different entry points to differentiate between their behaviors.
       In this case we'll stick to the standard main, however.
    */

    // Shader module creation
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    // Dynamic States of pipeline
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                 VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Passing vertex data into pipeline
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());

    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Geometry to be drawn from Vertices passed in
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports and Scissors

    // The viewport takes the swapchain images and transforms them to
    // framebuffers. Here we are setting the viewport to the swapchain images
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_vulkanGlobalState->GetSwapChainExtent().width;
    viewport.height = (float)m_vulkanGlobalState->GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Determines which part of the framebuffers will be rasterized.
    // Here we set it to the swapchain images so we can see the whole
    // framebuffer.
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_vulkanGlobalState->GetSwapChainExtent();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // depthClampEnable : Whether or not things outside the near and far planes
    // are discarded or clamped to them
    rasterizer.depthClampEnable = VK_FALSE;
    // rasterizerDiscardEnable : Allows geometry to pass through the rasterizer
    // stage. If disabled, disables output to framebuffer.
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
    rasterizer.depthBiasConstantFactor = 0.0f;  // Optional
    rasterizer.depthBiasClamp = 0.0f;           // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;     // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;           // Optional
    multisampling.pSampleMask = nullptr;             // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampling.alphaToOneEnable = VK_FALSE;       // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;  // Optional
    depthStencil.maxDepthBounds = 1.0f;  // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};  // Optional
    depthStencil.back = {};   // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor =
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;  // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f;  // Optional
    colorBlending.blendConstants[1] = 0.0f;  // Optional
    colorBlending.blendConstants[2] = 0.0f;  // Optional
    colorBlending.blendConstants[3] = 0.0f;  // Optional

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PBRPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    m_vulkanGlobalState->m_texturedMeshDescriptorSetLayouts.push_back(
        m_vulkanGlobalState->m_sceneDescriptorSetLayout);
    m_vulkanGlobalState->m_texturedMeshDescriptorSetLayouts.push_back(
        m_vulkanGlobalState->m_texturedPBRDescriptorSetLayout);
    pipelineLayoutInfo.setLayoutCount =
        m_vulkanGlobalState->m_texturedMeshDescriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts =
        m_vulkanGlobalState->m_texturedMeshDescriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &pipelineLayoutInfo,
            nullptr, &m_vulkanGlobalState->m_pbrPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    };

    // Finally creating graphics pipeline object and populating with necessary
    // information
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
    pipelineInfo.layout = m_vulkanGlobalState->m_pbrPipelineLayout;
    pipelineInfo.renderPass = m_vulkanGlobalState->m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(*m_vulkanGlobalState->GetRefLogicalDevice(),
                                  VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(*m_vulkanGlobalState->GetRefLogicalDevice(),
                          fragShaderModule, nullptr);
    vkDestroyShaderModule(*m_vulkanGlobalState->GetRefLogicalDevice(),
                          vertShaderModule, nullptr);
}

void VulkanContext::CreateShadowPassPipeline(std::vector<char> vertShaderCode,
                                             std::vector<char> fragShaderCode,
                                             VkPipeline* pipeline) {
    VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;

    // IMPORTANT NOTE!!!
    /*
        The next two members specify the shader module containing the code, and
       the function to invoke, known as the entrypoint. That means that it's
       possible to combine multiple fragment shaders into a single shader module
       and use different entry points to differentiate between their behaviors.
       In this case we'll stick to the standard main, however.
    */

    // Shader module creation
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo,
                                                      fragShaderStageInfo};

    // Dynamic States of pipeline
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,
                                                 VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount =
        static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Passing vertex data into pipeline
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    auto bindingDescription = Vertex::GetBindingDescription();
    auto attributeDescriptions = Vertex::GetAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(attributeDescriptions.size());

    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    // Geometry to be drawn from Vertices passed in
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType =
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewports and Scissors

    // The viewport takes the swapchain images and transforms them to
    // framebuffers. Here we are setting the viewport to the swapchain images
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_vulkanGlobalState->m_shadowMapWidth;
    viewport.height = (float)m_vulkanGlobalState->m_shadowMapHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Determines which part of the framebuffers will be rasterized.
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    VkExtent2D shadowMapExtent = {
        static_cast<uint32_t>(m_vulkanGlobalState->m_shadowMapWidth),
        static_cast<uint32_t>(m_vulkanGlobalState->m_shadowMapHeight)};
    scissor.extent = shadowMapExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType =
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    // depthClampEnable : Whether or not things outside the near and far planes
    // are discarded or clamped to them
    rasterizer.depthClampEnable = VK_FALSE;
    // rasterizerDiscardEnable : Allows geometry to pass through the rasterizer
    // stage. If disabled, disables output to framebuffer.
    // NOTE: Basically skips frag shader
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    // VK_POLYGON_MODE_FILL = Fill in shape

    // !! BELOW OPTIONS REQUIRE ENABLING GPU FEATURE !!
    // VK_POLYGON_MODE_LINE = Wireframe
    // VK_POLYGON_MODE_POINT = Vertices drawn as points
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    // Backface culling enabled here
    rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT;
    // Determines draw order of a "front" face
    rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    // !! Will learn about this in the future !!
    // NOTE: --- December 2025: Depth Bias is used to remove Shadow Acne
    rasterizer.depthBiasEnable = VK_TRUE;
    rasterizer.depthBiasConstantFactor = 0.0005f;
    rasterizer.depthBiasSlopeFactor = 1.0f;
    rasterizer.depthBiasClamp = 0.0f;  // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType =
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;           // Optional
    multisampling.pSampleMask = nullptr;             // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE;  // Optional
    multisampling.alphaToOneEnable = VK_FALSE;       // Optional

    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    depthStencil.sType =
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f;  // Optional
    depthStencil.maxDepthBounds = 1.0f;  // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};  // Optional
    depthStencil.back = {};   // Optional

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType =
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 0;
    colorBlending.pAttachments = nullptr;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(ShadowPassPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;

    // NOTE: ModelUBOs sent as push constants rather than descriptor sets
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &pipelineLayoutInfo,
            nullptr,
            &m_vulkanGlobalState->m_shadowPassPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    };

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
    pipelineInfo.layout = m_vulkanGlobalState->m_shadowPassPipelineLayout;
    pipelineInfo.renderPass = m_vulkanGlobalState->m_shadowPass;
    //  Subpass INDEX
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(*m_vulkanGlobalState->GetRefLogicalDevice(),
                                  VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow pass pipeline!");
    }

    vkDestroyShaderModule(*m_vulkanGlobalState->GetRefLogicalDevice(),
                          fragShaderModule, nullptr);

    vkDestroyShaderModule(*m_vulkanGlobalState->GetRefLogicalDevice(),
                          vertShaderModule, nullptr);
}
void VulkanContext::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = m_deviceHandler->FindQueueFamilies(
        m_vulkanGlobalState->GetPhysicalDevice());

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // Selecting graphicsFamily in order to issue draw commands in this command
    // pool
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &poolInfo, nullptr,
            &m_vulkanGlobalState->m_commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

// Pass in the Model3D's texture path
void VulkanContext::CreateTextureImage(TextureProperties props, Mesh3D* mesh,
                                       MaterialEnums materialType) {
    auto pixels = props.pixels;
    int texChannels = props.texChannels;
    int texHeight = props.texHeight;
    int texWidth = props.texWidth;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    if (!pixels) {
        throw std::runtime_error("failed to load texture image!");
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    m_bufferHandler->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                      VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
                stagingBufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
                  stagingBufferMemory);

    // WARN: Potential for memory leak here
    stbi_image_free(pixels);

    m_imageHandler->CreateImage(
        texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mesh->m_materials[materialType].m_textureImage,
        mesh->m_materials[materialType].m_textureImageMemory);

    m_imageHandler->TransitionImageLayout(
        mesh->m_materials[materialType].m_textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(
        stagingBuffer, mesh->m_materials[materialType].m_textureImage,
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    m_imageHandler->TransitionImageLayout(
        mesh->m_materials[materialType].m_textureImage, VK_FORMAT_R8G8B8A8_SRGB,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(*m_vulkanGlobalState->GetRefLogicalDevice(), stagingBuffer,
                    nullptr);
    vkFreeMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
                 stagingBufferMemory, nullptr);
}

// TEXTURE IMAGE VIEW
void VulkanContext::CreateTextureImageView(Mesh3D* mesh,
                                           MaterialEnums materialType) {
    mesh->m_materials[materialType].m_textureImageView =
        m_imageHandler->CreateImageView(
            *m_vulkanGlobalState->GetRefLogicalDevice(),
            mesh->m_materials[materialType].m_textureImage,
            VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

// TEXTURE SAMPLER
void VulkanContext::CreateTextureSampler(Mesh3D* mesh,
                                         MaterialEnums materialType) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_vulkanGlobalState->GetPhysicalDevice(),
                                  &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &samplerInfo, nullptr,
            &mesh->m_materials[materialType].m_textureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

void VulkanContext::CreateTextureSamplerShadowPass() {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;

    samplerInfo.anisotropyEnable = VK_FALSE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(m_vulkanGlobalState->GetPhysicalDevice(),
                                  &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    samplerInfo.compareEnable = VK_TRUE;
    samplerInfo.compareOp = VK_COMPARE_OP_LESS;

    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &samplerInfo, nullptr,
            &m_vulkanGlobalState->m_shadowPassTextureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
void VulkanContext::LoadMesh(ObjProperties props, glm::vec3 scenePosition,
                             glm::vec3 objectRotation, glm::vec3 objectScale) {
    std::unordered_map<Vertex, uint32_t> uniqueVertices{};

    int indexBufferStartIndex = 0;
    int vertexBufferStartIndex = 0;
    int vertexBufferEndIndex = 0;
    int indexBufferEndIndex = 0;
    if (m_indices.size() > 0) {
        indexBufferStartIndex = m_indices.size();
    }
    if (m_vertices.size() > 0) {
        vertexBufferStartIndex = m_vertices.size();
    }

    auto shapes = props.shapes;
    auto attrib = props.attrib;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            vertex.pos = {attrib.vertices[3 * index.vertex_index + 0],
                          attrib.vertices[3 * index.vertex_index + 1],
                          attrib.vertices[3 * index.vertex_index + 2]};

            vertex.normal = {attrib.normals[3 * index.normal_index + 0],
                             attrib.normals[3 * index.normal_index + 1],
                             attrib.normals[3 * index.normal_index + 2]};

            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] =
                    static_cast<uint32_t>(m_vertices.size());
                m_vertices.push_back(vertex);
            }
            m_indices.push_back(uniqueVertices[vertex]);

            vertex.color = {1.0f, 1.0f, 1.0f};
            vertexBufferEndIndex = m_vertices.size() - 1;
            indexBufferEndIndex = m_indices.size() - 1;
        }
    }
    Mesh3D mesh = Mesh3D();
    mesh.MeshType = TEXTURED;
    mesh.m_indexBufferStartIndex = indexBufferStartIndex;
    mesh.m_vertexBufferStartIndex = vertexBufferStartIndex;
    mesh.m_indexBufferEndIndex = indexBufferEndIndex;
    mesh.m_vertexBufferEndIndex = vertexBufferEndIndex;
    mesh.m_indexCount = indexBufferEndIndex - indexBufferStartIndex;
    mesh.m_position = scenePosition;
    mesh.m_rotation = objectRotation;
    mesh.m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    m_vulkanGlobalState->m_meshList.push_back(mesh);
}

void VulkanContext::RecordCommandBuffer(VkCommandBuffer commandBuffer,
                                        uint32_t imageIndex) {
    /*
        If the command buffer was already recorded once, then a call to
       vkBeginCommandBuffer will implicitly reset it. It's not possible to
       append commands to a buffer at a later time.
    */
    VkCommandBufferBeginInfo beginInfo{};
    /*
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be
       rerecorded right after executing it once.
        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : This is a secondary
       command buffer that will be entirely within a single render pass.
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : The command buffer can be
       resubmitted while it is also already pending execution.
    */
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    // We set the viewport and scissor state as dynamic in the pipeline
    // We need to set those up in the command buffer now
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_vulkanGlobalState->m_shadowMapWidth);
    viewport.height =
        static_cast<float>(m_vulkanGlobalState->m_shadowMapHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkExtent2D shadowMapExtent = {
        static_cast<uint32_t>(m_vulkanGlobalState->m_shadowMapWidth),
        static_cast<uint32_t>(m_vulkanGlobalState->m_shadowMapHeight)};
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = shadowMapExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo shadowPassInfo{};
    shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    shadowPassInfo.renderPass = m_vulkanGlobalState->m_shadowPass;
    shadowPassInfo.renderArea.offset = {0, 0};
    shadowPassInfo.renderArea.extent = shadowMapExtent;

    // VK_ATTACHMENT_LOAD_OP_3 clear values for color and depth stencil
    std::array<VkClearValue, 1> shadowPassClearValues{};
    shadowPassClearValues[0].depthStencil = {1.0f, 0};
    shadowPassInfo.clearValueCount = 1;
    shadowPassInfo.pClearValues = shadowPassClearValues.data();
    // for (auto& light : m_vulkanGlobalState->m_lights.lights) {
    // WARN: HARDCODING SHADOWPASS FRAMEBBUFFERS.
    // THIS SHOULD BE CHANGED IF MORE THAN 1 SHADOW CASTER
    shadowPassInfo.framebuffer =
        m_vulkanGlobalState->m_shadowPassFrameBuffers[0];
    vkCmdBeginRenderPass(commandBuffer, &shadowPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_vulkanGlobalState->m_shadowPassPipeline);

    if (!m_vulkanGlobalState->m_meshList.empty()) {
        VkBuffer vertexBuffers[] = {m_bufferHandler->VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_bufferHandler->IndexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        for (auto& mesh : m_vulkanGlobalState->m_meshList) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, -glm::radians(90.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));

            m_vulkanGlobalState->m_shadowPassPushConstants.model = model;

            vkCmdPushConstants(
                commandBuffer, m_vulkanGlobalState->m_shadowPassPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPassPushConstants),
                &m_vulkanGlobalState->m_shadowPassPushConstants);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(mesh.m_indexCount), 1,
                             mesh.m_indexBufferStartIndex, 0, 0);
        }
    }
    vkCmdEndRenderPass(commandBuffer);
    //}

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width =
        static_cast<float>(m_vulkanGlobalState->GetSwapChainExtent().width);
    viewport.height =
        static_cast<float>(m_vulkanGlobalState->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = m_vulkanGlobalState->GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_vulkanGlobalState->m_renderPass;
    // DEBUG
    // printf("imageIndex DEBUG: %d\n", imageIndex);
    renderPassInfo.framebuffer =
        m_vulkanGlobalState->m_renderPassFrameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent =
        m_vulkanGlobalState->GetSwapChainExtent();

    // VK_ATTACHMENT_LOAD_OP_3 clear values for color and depth stencil
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_vulkanGlobalState->m_pbrPipelineLayout, 0, 1,
        &m_vulkanGlobalState
             ->m_sceneDescriptorSets[m_vulkanGlobalState->m_currentFrame],
        0, nullptr);
    std::vector<VkDescriptorSet> descriptorSetsToBind;
    if (!m_vulkanGlobalState->m_meshList.empty()) {
        VkBuffer vertexBuffers[] = {m_bufferHandler->VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_bufferHandler->IndexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        for (auto& mesh : m_vulkanGlobalState->m_meshList) {
            // Binding the graphics pipeline
            if (mesh.MeshType == TEXTURED) {
                vkCmdBindPipeline(commandBuffer,
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  m_vulkanGlobalState->m_pbrPipeline);
                vkCmdBindDescriptorSets(
                    commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_vulkanGlobalState->m_pbrPipelineLayout, 1, 1,
                    &mesh.m_descriptorSets[m_vulkanGlobalState->m_currentFrame],
                    0, nullptr);
            }
            /*
                vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount,
               firstVertex, firstInstance) vertexCount: Even though we don't
               have a vertex buffer, we technically still have 3 vertices to
               draw. instanceCount: Used for instanced rendering, use 1 if
               you're not doing that. firstVertex: Used as an offset into the
               vertex buffer, defines the lowest value of gl_VertexIndex.
                firstInstance: Used as an offset for instanced rendering,
               defines the lowest value of gl_InstanceIndex.
            */
            // vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()),
            // 1, 0, 0);
            // m_vulkanGlobalState->m_pbrPushConstants = {
            //    m_vulkanGlobalState->m_shadowPassPushConstants.lightViewProj,
            //    m_vulkanGlobalState->m_shadowBias};

            m_vulkanGlobalState->m_pbrPushConstants.lightViewProj =
                m_vulkanGlobalState->m_shadowPassPushConstants.lightViewProj;

            m_vulkanGlobalState->m_pbrPushConstants.bias =
                m_vulkanGlobalState->m_shadowBias;

            vkCmdPushConstants(
                commandBuffer, m_vulkanGlobalState->m_pbrPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                sizeof(PBRPushConstants),
                &m_vulkanGlobalState->m_pbrPushConstants);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(mesh.m_indexCount), 1,
                             mesh.m_indexBufferStartIndex, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void VulkanContext::RecordCommandBufferGeneric(
    VkCommandBuffer commandBuffer, uint32_t imageIndex,
    std::vector<BrVkRenderPass> renderPasses) {
    /*
        If the command buffer was already recorded once, then a call to
       vkBeginCommandBuffer will implicitly reset it. It's not possible to
       append commands to a buffer at a later time.
    */
    VkCommandBufferBeginInfo beginInfo{};
    /*
        VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT: The command buffer will be
       rerecorded right after executing it once.
        VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : This is a secondary
       command buffer that will be entirely within a single render pass.
        VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : The command buffer can be
       resubmitted while it is also already pending execution.
    */
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    // We set the viewport and scissor state as dynamic in the pipeline
    // We need to set those up in the command buffer now
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_vulkanGlobalState->m_shadowMapWidth);
    viewport.height =
        static_cast<float>(m_vulkanGlobalState->m_shadowMapHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkExtent2D shadowMapExtent = {
        static_cast<uint32_t>(m_vulkanGlobalState->m_shadowMapWidth),
        static_cast<uint32_t>(m_vulkanGlobalState->m_shadowMapHeight)};
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = shadowMapExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo shadowPassInfo{};
    shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    shadowPassInfo.renderPass = m_vulkanGlobalState->m_shadowPass;
    shadowPassInfo.renderArea.offset = {0, 0};
    shadowPassInfo.renderArea.extent = shadowMapExtent;

    // VK_ATTACHMENT_LOAD_OP_3 clear values for color and depth stencil
    std::array<VkClearValue, 1> shadowPassClearValues{};
    shadowPassClearValues[0].depthStencil = {1.0f, 0};
    shadowPassInfo.clearValueCount = 1;
    shadowPassInfo.pClearValues = shadowPassClearValues.data();
    // for (auto& light : m_vulkanGlobalState->m_lights.lights) {
    // WARN: HARDCODING SHADOWPASS FRAMEBBUFFERS.
    // THIS SHOULD BE CHANGED IF MORE THAN 1 SHADOW CASTER
    shadowPassInfo.framebuffer =
        m_vulkanGlobalState->m_shadowPassFrameBuffers[0];
    vkCmdBeginRenderPass(commandBuffer, &shadowPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    // DRAW CALLS
    // SCENE
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_vulkanGlobalState->m_shadowPassPipeline);

    if (!m_vulkanGlobalState->m_meshList.empty()) {
        VkBuffer vertexBuffers[] = {m_bufferHandler->VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_bufferHandler->IndexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        for (auto& mesh : m_vulkanGlobalState->m_meshList) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, -glm::radians(90.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));

            m_vulkanGlobalState->m_shadowPassPushConstants.model = model;

            vkCmdPushConstants(
                commandBuffer, m_vulkanGlobalState->m_shadowPassPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPassPushConstants),
                &m_vulkanGlobalState->m_shadowPassPushConstants);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(mesh.m_indexCount), 1,
                             mesh.m_indexBufferStartIndex, 0, 0);
        }
    }
    vkCmdEndRenderPass(commandBuffer);
    //}

    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width =
        static_cast<float>(m_vulkanGlobalState->GetSwapChainExtent().width);
    viewport.height =
        static_cast<float>(m_vulkanGlobalState->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = m_vulkanGlobalState->GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_vulkanGlobalState->m_renderPass;
    // DEBUG
    // printf("imageIndex DEBUG: %d\n", imageIndex);
    renderPassInfo.framebuffer =
        m_vulkanGlobalState->m_renderPassFrameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent =
        m_vulkanGlobalState->GetSwapChainExtent();

    // VK_ATTACHMENT_LOAD_OP_3 clear values for color and depth stencil
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindDescriptorSets(
        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_vulkanGlobalState->m_pbrPipelineLayout, 0, 1,
        &m_vulkanGlobalState
             ->m_sceneDescriptorSets[m_vulkanGlobalState->m_currentFrame],
        0, nullptr);
    std::vector<VkDescriptorSet> descriptorSetsToBind;
    if (!m_vulkanGlobalState->m_meshList.empty()) {
        VkBuffer vertexBuffers[] = {m_bufferHandler->VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_bufferHandler->IndexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        for (auto& mesh : m_vulkanGlobalState->m_meshList) {
            // Binding the graphics pipeline
            if (mesh.MeshType == TEXTURED) {
                vkCmdBindPipeline(commandBuffer,
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  m_vulkanGlobalState->m_pbrPipeline);
                vkCmdBindDescriptorSets(
                    commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_vulkanGlobalState->m_pbrPipelineLayout, 1, 1,
                    &mesh.m_descriptorSets[m_vulkanGlobalState->m_currentFrame],
                    0, nullptr);
            }
            /*
                vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount,
               firstVertex, firstInstance) vertexCount: Even though we don't
               have a vertex buffer, we technically still have 3 vertices to
               draw. instanceCount: Used for instanced rendering, use 1 if
               you're not doing that. firstVertex: Used as an offset into the
               vertex buffer, defines the lowest value of gl_VertexIndex.
                firstInstance: Used as an offset for instanced rendering,
               defines the lowest value of gl_InstanceIndex.
            */
            // vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()),
            // 1, 0, 0);
            m_vulkanGlobalState->m_pbrPushConstants = {
                m_vulkanGlobalState->m_shadowPassPushConstants.lightViewProj,
                m_vulkanGlobalState->m_shadowBias};

            vkCmdPushConstants(
                commandBuffer, m_vulkanGlobalState->m_pbrPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                sizeof(PBRPushConstants),
                &m_vulkanGlobalState->m_shadowPassPushConstants.lightViewProj);
            vkCmdDrawIndexed(commandBuffer,
                             static_cast<uint32_t>(mesh.m_indexCount), 1,
                             mesh.m_indexBufferStartIndex, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}
void VulkanContext::CreateSyncObjects() {
    m_vulkanGlobalState->m_imageAvailableSemaphores.resize(m_maxFramesInFlight);
    m_vulkanGlobalState->m_renderFinishedSemaphores.resize(m_maxFramesInFlight);
    m_vulkanGlobalState->m_inFlightFences.resize(m_maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < m_maxFramesInFlight; i++) {
        if (vkCreateSemaphore(
                *m_vulkanGlobalState->GetRefLogicalDevice(), &semaphoreInfo,
                nullptr, &m_vulkanGlobalState->m_imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(
                *m_vulkanGlobalState->GetRefLogicalDevice(), &semaphoreInfo,
                nullptr, &m_vulkanGlobalState->m_renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(*m_vulkanGlobalState->GetRefLogicalDevice(),
                          &fenceInfo, nullptr,
                          &m_vulkanGlobalState->m_inFlightFences[i]) !=
                VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }
}

void VulkanContext::DrawFrame(float deltaTime) {
    /*
        - Wait for the previous frame to finish
        - Acquire an image from the swap chain
        - Record a command buffer which draws the scene onto that image
        - Submit the recorded command buffer
        - Present the swap chain image
    */
    vkWaitForFences(
        *m_vulkanGlobalState->GetRefLogicalDevice(), 1,
        &m_vulkanGlobalState
             ->m_inFlightFences[m_vulkanGlobalState->m_currentFrame],
        VK_TRUE, UINT64_MAX);

    // Acquire an image from the swap chain
    uint32_t imageIndex;

    // SWAP CHAIN RECREATION
    VkResult result = vkAcquireNextImageKHR(
        *m_vulkanGlobalState->GetRefLogicalDevice(),
        m_vulkanGlobalState->m_swapChain, UINT64_MAX,
        m_vulkanGlobalState
            ->m_imageAvailableSemaphores[m_vulkanGlobalState->m_currentFrame],
        VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        m_swapChainHandler->Rebuild();
        // TODO: MUST NOT FORGET THIS
        m_renderPassHandler->CreateRenderPass();
        m_renderPassHandler->CreateRenderPassFrameBuffers();
        m_renderPassHandler->CreateShadowPass();
        m_renderPassHandler->CreateShadowPassFrameBuffers();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset fences if we are submitting work
    vkResetFences(*m_vulkanGlobalState->GetRefLogicalDevice(), 1,
                  &m_vulkanGlobalState
                       ->m_inFlightFences[m_vulkanGlobalState->m_currentFrame]);

    // Record a command buffer which draws the scene onto that image
    vkResetCommandBuffer(
        m_vulkanGlobalState
            ->m_commandBuffers[m_vulkanGlobalState->m_currentFrame],
        0);

    RecordCommandBuffer(
        m_vulkanGlobalState
            ->m_commandBuffers[m_vulkanGlobalState->m_currentFrame],
        imageIndex);

    // Submit the recorded command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        m_vulkanGlobalState
            ->m_imageAvailableSemaphores[m_vulkanGlobalState->m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &m_vulkanGlobalState
             ->m_commandBuffers[m_vulkanGlobalState->m_currentFrame];

    VkSemaphore signalSemaphores[] = {
        m_vulkanGlobalState
            ->m_renderFinishedSemaphores[m_vulkanGlobalState->m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(
            m_vulkanGlobalState->m_graphicsQueue, 1, &submitInfo,
            m_vulkanGlobalState
                ->m_inFlightFences[m_vulkanGlobalState->m_currentFrame]) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Present swap chain image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {*m_vulkanGlobalState->GetRefSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
    // presentInfo.pResults = nullptr; // Optional

    // SWAP CHAIN RECREATION
    result =
        vkQueuePresentKHR(m_vulkanGlobalState->m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_windowHandler->framebufferResized) {
        m_windowHandler->framebufferResized = false;
        m_swapChainHandler->Rebuild();
        // TODO: MUST NOT FORGET THIS
        m_renderPassHandler->CreateRenderPass();
        m_renderPassHandler->CreateRenderPassFrameBuffers();
        m_renderPassHandler->CreateShadowPass();
        m_renderPassHandler->CreateShadowPassFrameBuffers();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }

    // Progress to next frame
    m_vulkanGlobalState->m_currentFrame =
        (m_vulkanGlobalState->m_currentFrame + 1) % m_maxFramesInFlight;
}

// Helpers
static bool CheckValidationLayerSupport(
    const std::vector<const char*>* validationLayers) {
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

static std::vector<const char*> GetRequiredExtensions(
    const bool* enableValidationLayers) {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions,
                                        glfwExtensions + glfwExtensionCount);

    if (*enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT messageType,
              const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
              void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void VulkanContext::Destroy() {
    m_swapChainHandler->Destroy();
    // DestroySwapChain();

    for (auto& mesh : m_vulkanGlobalState->m_meshList) {
        vkDestroySampler(*m_vulkanGlobalState->GetRefLogicalDevice(),
                         mesh.m_textureSampler, nullptr);
        vkDestroyImageView(*m_vulkanGlobalState->GetRefLogicalDevice(),
                           mesh.m_textureImageView, nullptr);
        vkDestroyImage(*m_vulkanGlobalState->GetRefLogicalDevice(),
                       mesh.m_textureImage, nullptr);
        vkFreeMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
                     mesh.m_textureImageMemory, nullptr);
    }

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        vkDestroyBuffer(*m_vulkanGlobalState->GetRefLogicalDevice(),
                        m_bufferHandler->UniformBuffers[i], nullptr);
        vkFreeMemory(*m_vulkanGlobalState->GetRefLogicalDevice(),
                     m_bufferHandler->UniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(*m_vulkanGlobalState->GetRefLogicalDevice(),
                            m_vulkanGlobalState->m_descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(*m_vulkanGlobalState->GetRefLogicalDevice(),
                                 m_vulkanGlobalState->m_meshDescriptorSetLayout,
                                 nullptr);

    // Buffer cleanup
    // Call BufferHandler::DestroyBuffers
    m_bufferHandler->DestroyBuffers();

    vkDestroyPipeline(*m_vulkanGlobalState->GetRefLogicalDevice(),
                      m_vulkanGlobalState->m_pbrPipeline, nullptr);
    vkDestroyPipelineLayout(*m_vulkanGlobalState->GetRefLogicalDevice(),
                            m_vulkanGlobalState->m_pbrPipelineLayout, nullptr);

    vkDestroyRenderPass(*m_vulkanGlobalState->GetRefLogicalDevice(),
                        m_renderPassHandler->m_renderPass, nullptr);

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        vkDestroySemaphore(*m_vulkanGlobalState->GetRefLogicalDevice(),
                           m_vulkanGlobalState->m_renderFinishedSemaphores[i],
                           nullptr);
        vkDestroySemaphore(*m_vulkanGlobalState->GetRefLogicalDevice(),
                           m_vulkanGlobalState->m_imageAvailableSemaphores[i],
                           nullptr);
        vkDestroyFence(*m_vulkanGlobalState->GetRefLogicalDevice(),
                       m_vulkanGlobalState->m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(*m_vulkanGlobalState->GetRefLogicalDevice(),
                         m_vulkanGlobalState->m_commandPool, nullptr);

    // DEVICE DESTRUCTION
    m_deviceHandler->Destroy();
    // GLFW DESTRUCTION
    m_windowHandler->Destroy();
}

VkShaderModule VulkanContext::CreateShaderModule(
    const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(*m_vulkanGlobalState->GetRefLogicalDevice(),
                             &createInfo, nullptr,
                             &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}
// Helpers

bool HasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
           format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void VulkanContext::CopyBufferToImage(VkBuffer buffer, VkImage image,
                                      uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};
    vkCmdCopyBufferToImage(commandBuffer, buffer, image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
    EndSingleTimeCommands(commandBuffer);
}

// beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
VkCommandBuffer VulkanContext::BeginSingleTimeCommands() {
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

void VulkanContext::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
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

// void VulkanContext::CreateImage(uint32_t width, uint32_t height,
//                                 VkFormat format, VkImageTiling tiling,
//                                 VkImageUsageFlags usage,
//                                 VkMemoryPropertyFlags properties,
//                                 VkImage& image, VkDeviceMemory& imageMemory)
//                                 {
//     VkImageCreateInfo imageInfo{};
//     imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
//     imageInfo.imageType = VK_IMAGE_TYPE_2D;
//     imageInfo.extent.width = static_cast<uint32_t>(width);
//     imageInfo.extent.height = static_cast<uint32_t>(height);
//     imageInfo.extent.depth = 1;
//     imageInfo.mipLevels = 1;
//     imageInfo.arrayLayers = 1;
//
//     // Be sure to match the image format with pixel format in the buffer
//     // otherwise copy operation will fail
//     imageInfo.format = format;
//
//     imageInfo.tiling = tiling;
//
//     imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
//
//     imageInfo.usage = usage;
//     imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//
//     imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
//     imageInfo.flags = 0;  // Optional
//
//     if (vkCreateImage(*m_vulkanGlobalState->GetRefLogicalDevice(),
//     &imageInfo,
//                       nullptr, &image) != VK_SUCCESS) {
//         throw std::runtime_error("failed to create image!");
//     }
//
//     VkMemoryRequirements memRequirements;
//     vk
