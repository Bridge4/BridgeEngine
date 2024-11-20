#pragma once

#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#define GLM_FORCE_RADIANS
// To meet Vulkan data alignment specs
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>

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
#include <cstdint> // Necessary for uint32_t
#include <limits> // Necessary for std::numeric_limits
#include <algorithm> // Necessary for std::clamp
#include <fstream>
#include <glm/glm.hpp>
#include <unordered_map>

#include "Includes/Window.h"
#include "Includes/Initializer.h"
#include "Includes/ImageView.h"
#include "Includes/SwapChain.h"
#include "VulkanInstance.h"

extern const uint32_t WIDTH = 800;
extern const uint32_t HEIGHT = 600;
extern const std::string MODEL_PATH = "Models/VikingRoom/VikingRoom.obj";
extern const std::string TEXTURE_PATH = "Models/VikingRoom/Textures/VikingRoom.png";

extern const int MAX_FRAMES_IN_FLIGHT = 2;

//struct Vertex {
//    glm::vec3 pos;
//    glm::vec3 color;
//    glm::vec2 texCoord;
//
//    static VkVertexInputBindingDescription getBindingDescription() {
//        VkVertexInputBindingDescription bindingDescription{};
//        bindingDescription.binding = 0;
//        bindingDescription.stride = sizeof(Vertex);
//        /*
//            VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
//            VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
//        */
//        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
//
//        return bindingDescription;
//    }
//
//    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
//        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
//
//        // position description
//        attributeDescriptions[0].location = 0;
//        attributeDescriptions[0].binding = 0;
//        /*
//            float: VK_FORMAT_R32_SFLOAT
//            vec2:  VK_FORMAT_R32G32_SFLOAT
//            vec3:  VK_FORMAT_R32G32B32_SFLOAT
//            vec4:  VK_FORMAT_R32G32B32A32_SFLOAT
//        */
//        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[0].offset = offsetof(Vertex, pos);
//        // color description
//        attributeDescriptions[1].binding = 0;
//        attributeDescriptions[1].location = 1;
//        /*
//            Color type (SFLOAT, UINT, SINT) and bit width should match to type of shader input
//            ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed integers
//            uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of 32-bit unsigned integers
//            double: VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float
//        */
//        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
//        attributeDescriptions[1].offset = offsetof(Vertex, color);
//
//        attributeDescriptions[2].binding = 0;
//        attributeDescriptions[2].location = 2;
//        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
//        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
//
//        return attributeDescriptions;
//    }
//
//    bool operator==(const Vertex& other) const {
//        return pos == other.pos && color == other.color && texCoord == other.texCoord;
//    }
//};
//
//namespace std {
//    template<> struct hash<Vertex> {
//        size_t operator()(Vertex const& vertex) const {
//            return ((hash<glm::vec3>()(vertex.pos) ^
//                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
//                (hash<glm::vec2>()(vertex.texCoord) << 1);
//        }
//    };
//}
//
//struct UniformBufferObject {
//    alignas(16) glm::mat4 model;
//    alignas(16) glm::mat4 view;
//    alignas(16) glm::mat4 proj;
//};


/*
*   For now, this Renderer is actually the Vulkan Instance class that will be placed in Modules/Renderer/API/Vulkan
*/
class Renderer {
public:
    
    void run() {
        //window.createWindow();
        initVulkan();
        renderLoop();
        cleanup();
    }

    Window window;
    bool customModelLoader = false;

    /*
    * 
    * 
    * 
    * The renderer draws things to the screen, different APIs have different ways of drawing things to the screen, therefore, 
    * the renderer draw call should be a virtual function implemented by the API instance we have created.
    * 
    * GRAPHICS PIPELINE
    * 
    * The graphics pipeline needs a reference to several things:
    *   - Swap Chain Extent
    *   - Descriptor Set Layout
    *   - Logical Device
    *   - Pipeline Layout
    *   - Render Pass
    * 
    *   We will get this info by accessing the Vulkan instance
    *   
    *   The Graphics Pipeline needs a pointer to a Vulkan Instance.
    *   When the Pipeline is initialized, the Instance Pointer is pointed towards the current Vulkan Instance.
    *   From there we can then access all the information we need to setup the pipeline.
    * 
    *   This also means that our Vulkan Instance needs to have a pointer to its constituent parts
    * 
    *   We can have many Graphics Pipelines, each time a Pipeline is created Vulkan will need to add it to a data structure 
    *   that will keep track of all the active Pipelines.
    * 
    *   As a base case we will only be creating one pipeline, keeping in mind that in the future we will need to store pipelines
    *   in a data structure.
    *   
    * 
    *   Start app -> Initialize Renderer -> Vulkan API selected -> Spin up an instance of Vulkan
    * 
    *   Vulkan Init -> RenderPassCreator.Create() 
    *               -> Vulkan Instance RenderPass set to value returned by RenderPassCreator.Create()
    *               -> RenderPassCreator.Clean();
    * 
    *   Repeat process with each member
    *   
    *   TODO: Proof of concept using existing abstractions. 
    * 
    *   Attempting first with Swap Chains.
    * 
    *   init.init(&window);
    *   
    *   
    */

    VkSwapchainKHR* getSwapChain() { return &vulkanInstance.m_swapChain; }
    VkFormat* getSwapChainImageFormat() { return &vulkanInstance.m_swapChainImageFormat; }
    VkExtent2D* getSwapChainExtent() { return &vulkanInstance.m_swapChainExtent; }
    std::vector<VkImageView>* getSwapChainImageViews() { return &vulkanInstance.m_swapChainImageViews; }

    VulkanInstance vulkanInstance;


private:
    
    // INSTANCES
    Initializer init;
    //Window window;
    SwapChain swapChain;
    ImageView imageView;


    //VkSurfaceKHR vulkanInstance.m_surface;
    //VkPhysicalDevice vulkanInstance.m_physicalDevice;
    //VkDevice vulkanInstance.m_logicalDevice;
    //VkQueue vulkanInstance.m_graphicsQueue;
    //VkQueue vulkanInstance.m_presentQueue;

    //VkSwapchainKHR vulkanInstance.m_swapChain;
    //VkFormat vulkanInstance.m_swapChainImageFormat;
    //VkExtent2D vulkanInstance.m_swapChainExtent;
    //std::vector<VkImageView> vulkanInstance.m_swapChainImageViews;

    //VkRenderPass vulkanInstance.m_renderPass;

    //VkDescriptorSetLayout vulkanInstance.m_descriptorSetLayout;
    //VkDescriptorPool vulkanInstance.m_descriptorPool;
    //std::vector<VkDescriptorSet> vulkanInstance.m_descriptorSets;

    //VkPipelineLayout vulkanInstance.m_pipelineLayout;

    //VkPipeline vulkanInstance.m_graphicsPipeline;
    //std::vector<VkFramebuffer> vulkanInstance.m_swapChainFramebuffers;
    //VkCommandPool vulkanInstance.m_commandPool;

    //std::vector<Vertex> vulkanInstance.m_vertices;
    //std::vector<uint32_t> vulkanInstance.m_indices;

    //VkBuffer vulkanInstance.m_vertexBuffer;
    //VkDeviceMemory vulkanInstance.m_vertexBufferMemory;

    //VkBuffer vulkanInstance.m_indexBuffer;
    //VkDeviceMemory vulkanInstance.m_indexBufferMemory;

    //VkImage vulkanInstance.m_textureImage;
    //VkImageView vulkanInstance.m_textureImageView;
    //VkSampler vulkanInstance.m_textureSampler;
    //VkDeviceMemory vulkanInstance.m_textureImageMemory;

    //std::vector<VkBuffer> vulkanInstance.m_uniformBuffers;
    //std::vector<VkDeviceMemory> vulkanInstance.m_uniformBuffersMemory;
    //std::vector<void*> vulkanInstance.m_uniformBuffersMapped;

    //VkImage vulkanInstance.m_depthImage;
    //VkDeviceMemory vulkanInstance.m_depthImageMemory;
    //VkImageView vulkanInstance.m_depthImageView;

    //// Frames in flight require their own command buffers, semaphores and fences
    //std::vector<VkCommandBuffer> vulkanInstance.m_commandBuffers;
    //std::vector<VkSemaphore> vulkanInstance.m_imageAvailableSemaphores;
    //std::vector<VkSemaphore> vulkanInstance.m_renderFinishedSemaphores;
    //std::vector<VkFence> vulkanInstance.m_inFlightFences;
    //uint32_t vulkanInstance.m_currentFrame = 0;



    //VulkanInstance vulkanInstance;
    

    // INITIALIZING VULKAN INSTANCE
    void initVulkan() {

        // Initialization
        init.init(&window);
        //init.assign(&vulkanInstance.m_surface, &vulkanInstance.m_physicalDevice, &vulkanInstance.m_logicalDevice, &vulkanInstance.m_graphicsQueue, &vulkanInstance.m_presentQueue);
        init.assign(&vulkanInstance.m_surface, &vulkanInstance.m_physicalDevice, &vulkanInstance.m_logicalDevice, &vulkanInstance.m_graphicsQueue, &vulkanInstance.m_presentQueue);
        // Swap Chain
        //swapChain.Construct(&vulkanInstance);
        swapChain.create(init, window);
        swapChain.createImageViews(vulkanInstance.m_logicalDevice, imageView);
        swapChain.assign(&vulkanInstance.m_swapChain, &vulkanInstance.m_swapChainImageFormat, &vulkanInstance.m_swapChainExtent, &vulkanInstance.m_swapChainImageViews);
        //swapChain.assign();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createCommandPool();

        createDepthResources();
        createFramebuffers();

        createTextureImage();
        createTextureImageView();
        createTextureSampler();
        
        loadModel();
        
        createVertexBuffer();
        createIndexBuffer();
        createUniformBuffers();

        createDescriptorPool();
        createDescriptorSets();

        createCommandBuffers();
        createSyncObjects();
    }
    
    /*
    * Window is polled and frames are drawn until window is closed
    */
    void renderLoop() {
        while (!window.shouldClose()) {
            window.poll();
            drawFrame();
        }
        vkDeviceWaitIdle(vulkanInstance.m_logicalDevice);
    }


    void recreateSwapChain() {
        // Handling minimization
        window.handleMinimization();

        vkDeviceWaitIdle(vulkanInstance.m_logicalDevice);

        cleanupSwapChain();

        swapChain.create(init, window);
        swapChain.createImageViews(vulkanInstance.m_logicalDevice, imageView);
        swapChain.assign(&vulkanInstance.m_swapChain, &vulkanInstance.m_swapChainImageFormat, &vulkanInstance.m_swapChainExtent, &vulkanInstance.m_swapChainImageViews);

        createDepthResources();
        createFramebuffers();
    }


    // RENDER PASSES
    void createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = vulkanInstance.m_swapChainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        // Store rendered contents in memory to be read later
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        // Lookup stencil buffers later
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

        // The initialLayout specifies which layout the image will have before the render pass begins.
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        /*
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL: Images used as color attachment
            VK_IMAGE_LAYOUT_PRESENT_SRC_KHR: Images to be presented in the swap chain
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : Images to be used as destination for a memory copy operation

            "The caveat of this special value is that the contents of the image are not guaranteed to be preserved,
            but that doesn't matter since we're going to clear it anyway.
            We want the image to be ready for presentation using the swap chain after rendering,
            which is why we use VK_IMAGE_LAYOUT_PRESENT_SRC_KHR as finalLayout."
        */

        // The finalLayout specifies the layout to automatically transition to when the render pass finishes. 
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = findDepthFormat();
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        /*
            pInputAttachments: Attachments that are read from a shader
            pResolveAttachments: Attachments used for multisampling color attachments
            pDepthStencilAttachment : Attachment for depthand stencil data
            pPreserveAttachments : Attachments that are not used by this subpass, but for which the data must be preserved
        */
        // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;


        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcAccessMask = 0;

        // Creating render pass object and populating the struct with necessary properties
        std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        if (vkCreateRenderPass(vulkanInstance.m_logicalDevice, &renderPassInfo, nullptr, &vulkanInstance.m_renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void createDescriptorSetLayout() {

        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

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

        if (vkCreateDescriptorSetLayout(vulkanInstance.m_logicalDevice, &layoutInfo, nullptr, &vulkanInstance.m_descriptorSetLayout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }
    }

    // GRAPHICS PIPELINE
    /*
    *   Create a graphics pipeline to render
    * 
    */
    void createGraphicsPipeline() {
        auto vertShaderCode = readFile("Modules/Renderer/Shaders/vert.spv");
        auto fragShaderCode = readFile("Modules/Renderer/Shaders/frag.spv");
        
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
        viewport.width = (float)vulkanInstance.m_swapChainExtent.width;
        viewport.height = (float)vulkanInstance.m_swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;

        // Determines which part of the framebuffers will be rasterized. 
        // Here we set it to the swapchain images so we can see the whole framebuffer.
        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = vulkanInstance.m_swapChainExtent;

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
        pipelineLayoutInfo.pSetLayouts = &vulkanInstance.m_descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        if (vkCreatePipelineLayout(vulkanInstance.m_logicalDevice, &pipelineLayoutInfo, nullptr, &vulkanInstance.m_pipelineLayout) != VK_SUCCESS) {
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
        pipelineInfo.layout = vulkanInstance.m_pipelineLayout;
        pipelineInfo.renderPass = vulkanInstance.m_renderPass;
        // Subpass INDEX
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineInfo.basePipelineIndex = -1; // Optional

        if (vkCreateGraphicsPipelines(vulkanInstance.m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &vulkanInstance.m_graphicsPipeline) != VK_SUCCESS) {
            throw std::runtime_error("failed to create graphics pipeline!");
        }

        // CODE ABOVE THIS
        vkDestroyShaderModule(vulkanInstance.m_logicalDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(vulkanInstance.m_logicalDevice, vertShaderModule, nullptr);
    }

    // SWAPCHAIN FRAME BUFFERS
    void createFramebuffers() {
        vulkanInstance.m_swapChainFramebuffers.resize(vulkanInstance.m_swapChainImageViews.size());

        // Loop through swap chain image views
        for (size_t i = 0; i < vulkanInstance.m_swapChainImageViews.size(); i++) {
            std::array<VkImageView, 2> attachments = {
                vulkanInstance.m_swapChainImageViews[i],
                vulkanInstance.m_depthImageView
            };

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = vulkanInstance.m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = vulkanInstance.m_swapChainExtent.width;
            framebufferInfo.height = vulkanInstance.m_swapChainExtent.height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(vulkanInstance.m_logicalDevice, &framebufferInfo, nullptr, &vulkanInstance.m_swapChainFramebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    // COMMAND POOL
    /*
        Command pools manage the memory used to store command buffers
        Allows for multithreaded command recording since all commands are available together in the buffers
    */
    void createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = init.findQueueFamilies(vulkanInstance.m_physicalDevice);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // Selecting graphicsFamily in order to issue draw commands in this command pool
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        if (vkCreateCommandPool(vulkanInstance.m_logicalDevice, &poolInfo, nullptr, &vulkanInstance.m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void createDepthResources() {
        VkFormat depthFormat = findDepthFormat();
        createImage(vulkanInstance.m_swapChainExtent.width, vulkanInstance.m_swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vulkanInstance.m_depthImage, vulkanInstance.m_depthImageMemory);
        vulkanInstance.m_depthImageView = imageView.create(vulkanInstance.m_logicalDevice, vulkanInstance.m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

        transitionImageLayout(vulkanInstance.m_depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
    }

    VkFormat findDepthFormat() {
        return findSupportedFormat(
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    // used by findDepthFormat()
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(vulkanInstance.m_physicalDevice, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        throw std::runtime_error("failed to find supported format!");
    }

    bool hasStencilComponent(VkFormat format) {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    // TEXTURE IMAGE
    void createTextureImage() {
        int texWidth, texHeight, texChannels;

        stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

        // The multiplication by 4 here is because the pixels are loaded row by row with 4 bytes per pixel
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        if (!pixels) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory, 0, imageSize, 0, &data);
        memcpy(data, pixels, static_cast<size_t>(imageSize));
        vkUnmapMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory);

        stbi_image_free(pixels);

        createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
            VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanInstance.m_textureImage, vulkanInstance.m_textureImageMemory);

        transitionImageLayout(vulkanInstance.m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyBufferToImage(stagingBuffer, vulkanInstance.m_textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
        transitionImageLayout(vulkanInstance.m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

        vkDestroyBuffer(vulkanInstance.m_logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory, nullptr);
    }

    // TEXTURE IMAGE VIEW
    void createTextureImageView() {
        vulkanInstance.m_textureImageView = imageView.create(vulkanInstance.m_logicalDevice, vulkanInstance.m_textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    // TEXTURE SAMPLER
    void createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(vulkanInstance.m_physicalDevice, &properties);
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(vulkanInstance.m_logicalDevice, &samplerInfo, nullptr, &vulkanInstance.m_textureSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }


    // helper function used in createTextureImage();
    void createImage(uint32_t width, uint32_t height, VkFormat format,
        VkImageTiling tiling, VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image, VkDeviceMemory& imageMemory) {

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

        if (vkCreateImage(vulkanInstance.m_logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(vulkanInstance.m_logicalDevice, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(vulkanInstance.m_logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(vulkanInstance.m_logicalDevice, image, imageMemory, 0);
    }

    // TODO: Figure out what the fuck this does
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) {
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
        barrier.srcAccessMask = 0; // TODO
        barrier.dstAccessMask = 0; // TODO

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

    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
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


    void loadModel() {
        if (!customModelLoader) {
            tinyobj::attrib_t attrib;


            // List: {0, 1, 2, 3}

            // List(5): {0, 1, 2, 3, 4}

            // Vector (5): {0, 1, 2, 3, 4} + {4564565}
            // resize
            // Vector (6) : {0, 1, 2, 3, 4, 4564565}

            std::vector<tinyobj::shape_t> shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;

            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
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
                        uniqueVertices[vertex] = static_cast<uint32_t>(vulkanInstance.m_vertices.size());
                        vulkanInstance.m_vertices.push_back(vertex);
                    }
                    vulkanInstance.m_indices.push_back(uniqueVertices[vertex]);

                    vertex.color = { 1.0f, 1.0f, 1.0f };
                }
            }
        }
        else {

        }
    }

    // VERTEX BUFFER
    void createVertexBuffer() {

        // TODO: Go over staging buffers and why the hell we need them

        VkDeviceSize bufferSize = sizeof(vulkanInstance.m_vertices[0]) * vulkanInstance.m_vertices.size();

        // Create staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // Map it
        void* data;
        vkMapMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vulkanInstance.m_vertices.data(), (size_t)bufferSize);
        vkUnmapMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory);

        // Create vertex buffer
        createBuffer(bufferSize,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            vulkanInstance.m_vertexBuffer, vulkanInstance.m_vertexBufferMemory);

        // Copy data from staging buffer to vertex buffer
        copyBuffer(stagingBuffer, vulkanInstance.m_vertexBuffer, bufferSize);

        // cleanup
        vkDestroyBuffer(vulkanInstance.m_logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory, nullptr);
    }

    // INDEX BUFFER
    void createIndexBuffer() {
        VkDeviceSize bufferSize = sizeof(vulkanInstance.m_indices[0]) * vulkanInstance.m_indices.size();

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

        void* data;
        vkMapMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
        memcpy(data, vulkanInstance.m_indices.data(), (size_t)bufferSize);
        vkUnmapMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vulkanInstance.m_indexBuffer, vulkanInstance.m_indexBufferMemory);

        copyBuffer(stagingBuffer, vulkanInstance.m_indexBuffer, bufferSize);

        vkDestroyBuffer(vulkanInstance.m_logicalDevice, stagingBuffer, nullptr);
        vkFreeMemory(vulkanInstance.m_logicalDevice, stagingBufferMemory, nullptr);
    }

    // UNIFORM BUFFERS
    void createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);

        vulkanInstance.m_uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanInstance.m_uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanInstance.m_uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,vulkanInstance.m_uniformBuffers[i], vulkanInstance.m_uniformBuffersMemory[i]);
            vkMapMemory(vulkanInstance.m_logicalDevice, vulkanInstance.m_uniformBuffersMemory[i], 0, bufferSize, 0, &vulkanInstance.m_uniformBuffersMapped[i]);
        }
    }

    // DESCRIPTOR POOL
    void createDescriptorPool() {

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

        if (vkCreateDescriptorPool(vulkanInstance.m_logicalDevice, &poolInfo, nullptr, &vulkanInstance.m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    // DESCRIPTOR SETS
    void createDescriptorSets() {
        std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, vulkanInstance.m_descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = vulkanInstance.m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        vulkanInstance.m_descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(vulkanInstance.m_logicalDevice, &allocInfo, vulkanInstance.m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = vulkanInstance.m_uniformBuffers[i];
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(UniformBufferObject);

            VkDescriptorImageInfo imageInfo{};
            imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfo.imageView = vulkanInstance.m_textureImageView;
            imageInfo.sampler = vulkanInstance.m_textureSampler;

            std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
            descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[0].dstSet = vulkanInstance.m_descriptorSets[i];
            descriptorWrites[0].dstBinding = 0;
            descriptorWrites[0].dstArrayElement = 0;
            descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[0].descriptorCount = 1;
            descriptorWrites[0].pBufferInfo = &bufferInfo;
            descriptorWrites[0].pImageInfo = nullptr; // Optional
            descriptorWrites[0].pTexelBufferView = nullptr; // Optional

            descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[1].dstSet = vulkanInstance.m_descriptorSets[i];
            descriptorWrites[1].dstBinding = 1;
            descriptorWrites[1].dstArrayElement = 0;
            descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[1].descriptorCount = 1;
            descriptorWrites[1].pImageInfo = &imageInfo;


            vkUpdateDescriptorSets(vulkanInstance.m_logicalDevice, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        }


    }

    // Buffer creation helper
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties,
        VkBuffer& buffer, VkDeviceMemory& bufferMemory) {

        // Creation
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(vulkanInstance.m_logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create vertex buffer!");
        }

        // Allocation
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(vulkanInstance.m_logicalDevice, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(vulkanInstance.m_logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        // Binding
        vkBindBufferMemory(vulkanInstance.m_logicalDevice, buffer, bufferMemory, 0);
    }

    // beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
    VkCommandBuffer beginSingleTimeCommands() {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = vulkanInstance.m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(vulkanInstance.m_logicalDevice, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(vulkanInstance.m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(vulkanInstance.m_graphicsQueue);

        vkFreeCommandBuffers(vulkanInstance.m_logicalDevice, vulkanInstance.m_commandPool, 1, &commandBuffer);
    }

    // COPY BUFFER
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        endSingleTimeCommands(commandBuffer);
    }

    // TODO: Move this to the Texture class as well as the Image class if I end up creating that (the Image class)
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(vulkanInstance.m_physicalDevice, &memProperties);

        // Go over this section
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    // COMMAND BUFFER
    void createCommandBuffers() {
        vulkanInstance.m_commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = vulkanInstance.m_commandPool;
        /*
            VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
            VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
        */
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)vulkanInstance.m_commandBuffers.size();

        if (vkAllocateCommandBuffers(vulkanInstance.m_logicalDevice, &allocInfo, vulkanInstance.m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
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
        renderPassInfo.renderPass = vulkanInstance.m_renderPass;
        // DEBUG
        //printf("imageIndex DEBUG: %d\n", imageIndex);
        renderPassInfo.framebuffer = vulkanInstance.m_swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = vulkanInstance.m_swapChainExtent;

        // VK_ATTACHMENT_LOAD_OP_CLEAR clear values for color and depth stencil
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();


        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        // Binding the graphics pipeline
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanInstance.m_graphicsPipeline);

        // We set the viewport and scissor state as dynamic in the pipeline 
        // We need to set those up in the command buffer now  
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(vulkanInstance.m_swapChainExtent.width);
        viewport.height = static_cast<float>(vulkanInstance.m_swapChainExtent.height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = vulkanInstance.m_swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkBuffer vertexBuffers[] = { vulkanInstance.m_vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, vulkanInstance.m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkanInstance.m_pipelineLayout, 0, 1,
            &vulkanInstance.m_descriptorSets[vulkanInstance.m_currentFrame], 0, nullptr);

        /*
            vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount, firstVertex, firstInstance)
            vertexCount: Even though we don't have a vertex buffer, we technically still have 3 vertices to draw.
            instanceCount: Used for instanced rendering, use 1 if you're not doing that.
            firstVertex: Used as an offset into the vertex buffer, defines the lowest value of gl_VertexIndex.
            firstInstance: Used as an offset for instanced rendering, defines the lowest value of gl_InstanceIndex.
        */
        //vkCmdDraw(commandBuffer, static_cast<uint32_t>(vertices.size()), 1, 0, 0);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vulkanInstance.m_indices.size()), 1, 0, 0, 0);
        vkCmdEndRenderPass(commandBuffer);

        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }

    }

    // SEMAPHORES AND FENCES
    // Creates semaphores and fences for each frame in flight
    void createSyncObjects() {
        vulkanInstance.m_imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanInstance.m_renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
        vulkanInstance.m_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);


        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(vulkanInstance.m_logicalDevice, &semaphoreInfo, nullptr, &vulkanInstance.m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(vulkanInstance.m_logicalDevice, &semaphoreInfo, nullptr, &vulkanInstance.m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(vulkanInstance.m_logicalDevice, &fenceInfo, nullptr, &vulkanInstance.m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }

    // DRAW FRAME
    void drawFrame() {
        /*
            - Wait for the previous frame to finish
            - Acquire an image from the swap chain
            - Record a command buffer which draws the scene onto that image
            - Submit the recorded command buffer
            - Present the swap chain image
        */
        // Wait for the previous frame to finish

        updateUniformBuffer(vulkanInstance.m_currentFrame);
        vkWaitForFences(vulkanInstance.m_logicalDevice, 1, &vulkanInstance.m_inFlightFences[vulkanInstance.m_currentFrame], VK_TRUE, UINT64_MAX);

        // Acquire an image from the swap chain
        uint32_t imageIndex;

        // SWAP CHAIN RECREATION
        VkResult result = vkAcquireNextImageKHR(vulkanInstance.m_logicalDevice, vulkanInstance.m_swapChain, UINT64_MAX, vulkanInstance.m_imageAvailableSemaphores[vulkanInstance.m_currentFrame],VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        // Only reset fences if we are submitting work
        vkResetFences(vulkanInstance.m_logicalDevice, 1, &vulkanInstance.m_inFlightFences[vulkanInstance.m_currentFrame]);

        // Record a command buffer which draws the scene onto that image
        vkResetCommandBuffer(vulkanInstance.m_commandBuffers[vulkanInstance.m_currentFrame], 0);
        recordCommandBuffer(vulkanInstance.m_commandBuffers[vulkanInstance.m_currentFrame], imageIndex);

        // Submit the recorded command buffer
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { vulkanInstance.m_imageAvailableSemaphores[vulkanInstance.m_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &vulkanInstance.m_commandBuffers[vulkanInstance.m_currentFrame];

        VkSemaphore signalSemaphores[] = { vulkanInstance.m_renderFinishedSemaphores[vulkanInstance.m_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        if (vkQueueSubmit(vulkanInstance.m_graphicsQueue, 1, &submitInfo, vulkanInstance.m_inFlightFences[vulkanInstance.m_currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit draw command buffer!");
        }

        // Present swap chain image
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { vulkanInstance.m_swapChain };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &imageIndex;
        //presentInfo.pResults = nullptr; // Optional

        // SWAP CHAIN RECREATION
        result = vkQueuePresentKHR(vulkanInstance.m_presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.framebufferResized) {
            window.framebufferResized = false;
            recreateSwapChain();
        }
        else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        // Progress to next frame
        vulkanInstance.m_currentFrame = (vulkanInstance.m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), vulkanInstance.m_swapChainExtent.width / (float)vulkanInstance.m_swapChainExtent.height, 0.1f, 10.0f);
        // IMPORTANT: VULKAN HAS INVERTED Y AXIS TO OPENGL AND GLM WAS DESIGNED FOR OPENGL. THIS CONVERTS TO VULKAN.
        ubo.proj[1][1] *= -1;

        memcpy(vulkanInstance.m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
    }

    // SHADER MODULES
    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(vulkanInstance.m_logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
        }

        return shaderModule;
    }

    // Reading in SPIRV shaders
    static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        //std::cout << buffer.size() << std::endl;

        return buffer;
    }

    void cleanupSwapChain() {
        vkDestroyImageView(vulkanInstance.m_logicalDevice, vulkanInstance.m_depthImageView, nullptr);
        vkDestroyImage(vulkanInstance.m_logicalDevice, vulkanInstance.m_depthImage, nullptr);
        vkFreeMemory(vulkanInstance.m_logicalDevice, vulkanInstance.m_depthImageMemory, nullptr);

        for (auto framebuffer : vulkanInstance.m_swapChainFramebuffers) {
            vkDestroyFramebuffer(vulkanInstance.m_logicalDevice, framebuffer, nullptr);
        }

        for (auto imageView : vulkanInstance.m_swapChainImageViews) {
            vkDestroyImageView(vulkanInstance.m_logicalDevice, imageView, nullptr);
        }

        vkDestroySwapchainKHR(vulkanInstance.m_logicalDevice, vulkanInstance.m_swapChain, nullptr);
    }

    // CLEANUP
    void cleanup() {
        cleanupSwapChain();

        vkDestroySampler(vulkanInstance.m_logicalDevice, vulkanInstance.m_textureSampler, nullptr);
        vkDestroyImageView(vulkanInstance.m_logicalDevice, vulkanInstance.m_textureImageView, nullptr);
        vkDestroyImage(vulkanInstance.m_logicalDevice, vulkanInstance.m_textureImage, nullptr);
        vkFreeMemory(vulkanInstance.m_logicalDevice, vulkanInstance.m_textureImageMemory, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroyBuffer(vulkanInstance.m_logicalDevice, vulkanInstance.m_uniformBuffers[i], nullptr);
            vkFreeMemory(vulkanInstance.m_logicalDevice, vulkanInstance.m_uniformBuffersMemory[i], nullptr);
        }

        vkDestroyDescriptorPool(vulkanInstance.m_logicalDevice, vulkanInstance.m_descriptorPool, nullptr);

        vkDestroyDescriptorSetLayout(vulkanInstance.m_logicalDevice, vulkanInstance.m_descriptorSetLayout, nullptr);

        // Buffer cleanup
        vkDestroyBuffer(vulkanInstance.m_logicalDevice, vulkanInstance.m_indexBuffer, nullptr);
        vkFreeMemory(vulkanInstance.m_logicalDevice, vulkanInstance.m_indexBufferMemory, nullptr);
        vkDestroyBuffer(vulkanInstance.m_logicalDevice, vulkanInstance.m_vertexBuffer, nullptr);
        vkFreeMemory(vulkanInstance.m_logicalDevice, vulkanInstance.m_vertexBufferMemory, nullptr);

        vkDestroyPipeline(vulkanInstance.m_logicalDevice, vulkanInstance.m_graphicsPipeline, nullptr);
        vkDestroyPipelineLayout(vulkanInstance.m_logicalDevice, vulkanInstance.m_pipelineLayout, nullptr);

        vkDestroyRenderPass(vulkanInstance.m_logicalDevice, vulkanInstance.m_renderPass, nullptr);

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(vulkanInstance.m_logicalDevice, vulkanInstance.m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(vulkanInstance.m_logicalDevice, vulkanInstance.m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(vulkanInstance.m_logicalDevice, vulkanInstance.m_inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(vulkanInstance.m_logicalDevice, vulkanInstance.m_commandPool, nullptr);

        // DEVICE DESTRUCTION
        init.destroy();
        // GLFW DESTRUCTION
        window.destroy();
    }
};
