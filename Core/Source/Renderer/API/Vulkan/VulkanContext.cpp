#include <vulkan/vulkan_core.h>

#include <cassert>
#include <random>
#include <thread>

#include "Source/Engine.h"
#include "Source/Renderer/API/Vulkan/Components/DescriptorSets/DescriptorSetHandler.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/Materials/Materials.h"
#include "Source/Renderer/RendererTypes.h"
#include "config.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <imgui.h>

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
#include "Source/FileSystem/FileLoader.h"
#include "Source/Renderer/Camera/CameraController.h"
#include "Source/Renderer/Mesh/Mesh3D.h"
#include "Source/Window/WindowingBackend.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

// This is our little safe playground to move things away from the vulkan global
// state
namespace context_vk {
void RenderUI() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // bool show_demo_window = true;
    // ImGui::ShowDemoWindow(&show_demo_window);
    ImGui::Begin("ImGui Sanity Check");

    // Test static text
    ImGui::Text("Hello, Vulkan + ImGui!");
    ImGui::Separator();

    // Test interactivity with a counter
    static int counter = 0;
    if (ImGui::Button("Click Me!")) {
        counter++;
    }
    ImGui::SameLine();
    ImGui::Text("Button clicks = %d", counter);

    // Test value tracking and color manipulation
    static float color[3] = {0.45f, 0.55f, 0.60f};
    ImGui::ColorEdit3("Clear Color Picker", color);

    // Test frame rates (verifies NewFrame delta timing is working)
    ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)",
                1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    ImGui::End();

    ImGui::Render();
}

VkShaderModule CreateShaderModule(const std::vector<char>& code,
                                  VkDevice& logical_device) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(logical_device, &createInfo, nullptr,
                             &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module!");
    }

    return shaderModule;
}

VkPipeline CreateComputePipeline(
    std::vector<char> shader, VkDevice& logical_device,
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
    VkPipelineLayout& pipelineLayout) {
    VkPipeline pipeline;
    VkShaderModule computeShader = CreateShaderModule(shader, logical_device);

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.module = computeShader;
    shaderStageCreateInfo.pName = "main";

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

    if (vkCreatePipelineLayout(logical_device, &pipelineLayoutInfo, nullptr,
                               &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout!");
    };

    VkComputePipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.stage = shaderStageCreateInfo;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    VkResult result = vkCreateComputePipelines(
        logical_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline);

    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute pipeline!");
    }

    vkDestroyShaderModule(logical_device, computeShader, nullptr);

    return pipeline;
}

// Courtesy of my code from college :)
void SetIndex(std::vector<unsigned int>& indices, unsigned int r3,
              unsigned int r2, unsigned int r1, unsigned int l3,
              unsigned int l2, unsigned int l1) {
    indices.push_back(r3);
    indices.push_back(r2);
    indices.push_back(r1);
    indices.push_back(l3);
    indices.push_back(l2);
    indices.push_back(l1);
}

void SetNormal(std::vector<Vertex>& vertices, unsigned int v1, unsigned int v2,
               unsigned int v3) {
    /*
        From khronos website
        Set Vector U to (Triangle.p2 minus Triangle.p1)
        Set Vector V to (Triangle.p3 minus Triangle.p1)
        Set Normal.x to (multiply U.y by V.z) minus (multiply U.z by V.y)
        Set Normal.y to (multiply U.z by V.x) minus (multiply U.x by V.z)
        Set Normal.z to (multiply U.x by V.y) minus (multiply U.y by V.x)
    */
    glm::vec3 A = vertices[v1].pos;
    glm::vec3 B = vertices[v2].pos;
    glm::vec3 C = vertices[v3].pos;

    // From my lighting homework

    //  VECTOR FROM A TO B
    float dx0 = A.x - B.x;
    float dy0 = A.y - B.y;
    float dz0 = A.z - B.z;
    //  VECTOR FROM C TO A
    float dx1 = C.x - A.x;
    float dy1 = C.y - A.y;
    float dz1 = C.z - A.z;
    //  Normal
    float Nx = dy0 * dz1 - dy1 * dz0;
    float Ny = dz0 * dx1 - dz1 * dx0;
    float Nz = dx0 * dy1 - dx1 * dy0;

    glm::vec3 normal = glm::vec3(Nx, Ny, Nz);
    vertices[v1].normal = normal;
    vertices[v2].normal = normal;
    vertices[v3].normal = normal;
}

void SetIndices(std::vector<uint32_t>& indices, Vertex* vertices,
                int worldHeight, int worldWidth) {
    // DRAW ORDER OF TRIANGLES
    // right facing triangle draw order
    unsigned int r1, r2, r3;
    r1 = worldWidth;
    r2 = 0;
    r3 = 1;
    // left facing triangle draw order
    unsigned int l1, l2, l3;
    l1 = worldWidth + 1;
    l2 = worldWidth;
    l3 = 1;
    // total drawn in current row
    int inRowRight = 0;
    int inRowLeft = 0;
    // total drawn overall
    int drawnRight = 0;
    int drawnLeft = 0;
    // limit of triangles to draw
    int limit = (worldWidth - 1) * (worldHeight - 1);

    // WARN: worldHeight * worldWidth
    for (int i = 0; i < worldHeight * worldWidth; i += 1) {
        if (drawnRight == limit && drawnLeft == limit) break;
        // Draw order for right-facing and left-facing triangles
        SetIndex(indices, r3, r2, r1, l3, l2, l1);
        // SetNormal(vertices, r3, r2, r1);
        // SetNormal(vertices, l3, l2, l1);
        inRowLeft++;
        drawnLeft++;
        inRowRight++;
        drawnRight++;
        if (inRowLeft == worldWidth - 1) {
            l1 += 2;
            l2 += 2;
            l3 += 2;
            inRowLeft = 0;
        } else {
            l1++;
            l2++;
            l3++;
        }
        if (inRowRight == worldWidth - 1) {
            r1 += 2;
            r2 += 2;
            r3 += 2;
            inRowRight = 0;
        } else {
            r1++;
            r2++;
            r3++;
        }
    }
}
std::vector<VkFramebuffer> CreateSwapChainFramebuffers(
    int numberOfFramebuffers, std::vector<VkImageView> perFrameImageViews,
    std::vector<VkImageView> swapChainImageViews, uint32_t framebufferWidth,
    uint32_t framebufferHeight, VkDevice& logical_device,
    render_pass::RenderPassData renderPassData) {
    // TODO: Check if our targeted render pass is valid
    // TODO: Check if count > 0
    // TODO: Check if attachmentImageViews is not empty
    // TODO: Check if each attachmentImageView is valid
    // TODO: Check if framebufferHeight is not 0
    // TODO: Check if attachmentImageViews.size() == renderPass attachment count

    VkRenderPass renderPass = renderPassData.render_pass;
    if (renderPass == VK_NULL_HANDLE || renderPass == nullptr) {
        std::cout << __LINE__
                  << " FAILED TO CREATE FRAMEBUFFER: INVALID RENDER PASS\n";
        return {};
    }

    std::vector<VkFramebuffer> framebuffers = {};

    // CreateShadowPassDepthResources();
    framebuffers.resize(numberOfFramebuffers);

    for (size_t i = 0; i < numberOfFramebuffers; i++) {
        // NOTE: Each renderpass framebuffer has a swapChainImageView and a
        // renderPassDepthImageView
        std::vector<VkImageView> attachments = {};
        attachments.push_back(swapChainImageViews[i]);
        for (size_t j = 0; j < perFrameImageViews.size(); j++) {
            attachments.push_back(perFrameImageViews[j]);
        }
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPassData.render_pass;
        framebufferInfo.attachmentCount = renderPassData.attachment_count;
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = framebufferWidth;
        framebufferInfo.height = framebufferHeight;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logical_device, &framebufferInfo, nullptr,
                                &framebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    return framebuffers;
}

bool CheckRenderPassDataValidity(
    render_pass::RenderPassData::ERenderPassDataState renderPassDataState) {
    if (renderPassDataState !=
        render_pass::RenderPassData::ERenderPassDataState::VALID) {
        switch (renderPassDataState) {
            case render_pass::RenderPassData::ERenderPassDataState::
                RENDER_PASS_INVALID:
                std::cout << __LINE__ << " ERROR: RENDER PASS INVALID\n";
                break;
            case render_pass::RenderPassData::ERenderPassDataState::
                ATTACHMENT_COUNT_INVALID:
                std::cout << __LINE__
                          << " ERROR: RENDER PASS ATTACHMENT COUNT INVALID\n";
                break;
            case render_pass::RenderPassData::ERenderPassDataState::
                FRAMEBUFFERS_INVALID:
                std::cout << __LINE__
                          << " ERROR: RENDER PASS FRAMEBUFFERS INVALID\n";
                break;
            default:
                break;
        }
        return false;
    }
    std::cout << "SUCCESS: RENDER PASS DATA VALID\n";
    return true;
}

}  // namespace context_vk
namespace cvk_render_pass = context_vk::render_pass;

void VulkanContext::init_vulkan(ERenderingMode rendering_mode) {
    m_vulkan_global_state = new VulkanGlobalState(this, rendering_mode);

    m_device_handler = new DeviceHandler(m_vulkan_global_state);
    m_descriptor_set_handler =
        new DescriptorSetHandler(m_vulkan_global_state, m_device_handler);
    m_image_handler =
        new ImageHandler(this, m_vulkan_global_state, m_device_handler);
    m_swap_chain_handler = new SwapChainHandler(
        this, m_device_handler, m_image_handler, m_vulkan_global_state);
    m_buffer_handler =
        new BufferHandler(m_vulkan_global_state, m_device_handler);
    m_render_pass_handler =
        new RenderPassHandler(this, m_swap_chain_handler, m_device_handler,
                              m_image_handler, m_vulkan_global_state);
    m_swap_chain_handler->m_renderPassHandler = m_render_pass_handler;
    m_cameraController =
        new CameraController(this, m_windowing_backend, m_swap_chain_handler,
                             m_buffer_handler, m_vulkan_global_state);

    m_vulkan_global_state->create_vulkan_instance();
    // NOTE: Windowing code
    // m_windowHandlerVK->vulkanContext = this;

    if (m_windowing_backend->create_surface_vk_glfw(
            m_vulkan_global_state->m_instance,
            &m_vulkan_global_state->m_surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");

    m_device_handler->init();
    int framebuffer_width, framebuffer_height;
    m_windowing_backend->get_framebuffer_size_glfw(&framebuffer_width,
                                                   &framebuffer_height);
    m_swap_chain_handler->init(framebuffer_width, framebuffer_height);
    create_command_pool();

    VkFormat render_pass_depth_attachment_format =
        m_render_pass_handler->find_supported_format(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
             VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    if (!render_pass_depth_attachment_format) {
        std::cout << "renderPassDepthAttachmentFormat invalid\n";
    }

    cvk_render_pass::DepthResources render_pass_depth_resources =
        m_render_pass_handler->create_render_pass_depth_resources(
            render_pass_depth_attachment_format,
            m_vulkan_global_state->m_swapChainExtent.width,
            m_vulkan_global_state->m_swapChainExtent.height);

    cvk_render_pass::RenderPassData render_pass_data =
        m_render_pass_handler->create_render_pass(
            m_device_handler->m_logical_device,
            m_vulkan_global_state->GetSwapChainImageFormat(),
            render_pass_depth_attachment_format);

    render_pass_data.depthResources = render_pass_depth_resources;

    m_vulkan_global_state->m_render_pass_data =
        std::make_unique<cvk_render_pass::RenderPassData>(render_pass_data);

    if (m_vulkan_global_state->m_render_pass_data == nullptr) {
        std::cout << "Vulkan Init Error: m_vulkanGlobalState->m_renderPassData "
                     "is INVALID!\n";
        return;
    }

    // m_renderPassHandler->CreateShadowPass();

    QueueFamilyIndices queue_family_indices =
        m_device_handler->find_queue_families(m_device_handler->PhysicalDevice);
    m_vulkan_global_state->m_commandPoolCompute = create_command_pool_compute(
        m_device_handler->m_logical_device, queue_family_indices);

    // m_renderPassHandler->CreateRenderPassFrameBuffers();

    std::vector<VkImageView> renderPassPerFrameImageViews = {};

    renderPassPerFrameImageViews.push_back(
        m_vulkan_global_state->m_render_pass_data->depthResources.imageView);

    m_vulkan_global_state->m_render_pass_data->framebuffers =
        context_vk::CreateSwapChainFramebuffers(
            m_vulkan_global_state->m_swapChainImageViews.size(),
            renderPassPerFrameImageViews,
            m_vulkan_global_state->m_swapChainImageViews,
            m_vulkan_global_state->GetSwapChainExtent().width,
            m_vulkan_global_state->GetSwapChainExtent().height,
            m_device_handler->m_logical_device,
            *m_vulkan_global_state->m_render_pass_data);
    if (m_vulkan_global_state->m_renderingMode == ERenderingMode::Bindless) {
        m_descriptor_set_handler->CreateDescriptorPoolBindless();
        m_descriptor_set_handler->CreateDescriptorPoolBindlessScene();
        m_descriptor_set_handler->CreateDescriptorPoolPerMeshBindless();
        m_descriptor_set_handler->CreateDescriptorSetLayoutBindless();
        m_descriptor_set_handler->CreateDescriptorSetBindless();
    } else {
        m_descriptor_set_handler->CreateDescriptorPool();
    }
    // SceneDescriptorSets
    m_descriptor_set_handler->CreateSceneDescriptorSetLayout();
    m_buffer_handler->CreateCameraUBO();
    m_buffer_handler->CreateLightUBO();
    m_descriptor_set_handler->CreateSceneDescriptorSets();

    // NOTE: WE HAVE FINISHED SETTING UP THE BINDLESS DESCRIPTOR POOLS AND
    // WE NOW NEED TO SETUP THE MATERIAL INDEX STRUCT, BINDLESS PBR SHADERS,
    // BINDLESS PBR PIPELINE AND BINDLESS PBR DRAW LOGIC

    std::vector<char> vertShadow = read_file(SHADERS_DIR "shadow.spv");
    std::vector<char> fragShadow = read_file(SHADERS_DIR "shadowFrag.spv");
    // CreateShadowPassPipeline(vertShadow, fragShadow,
    //                          &m_vulkanGlobalState->m_shadowPassPipeline);
    // CreateTextureSamplerShadowPass();

    if (m_vulkan_global_state->m_renderingMode == ERenderingMode::Bindless) {
        std::vector<char> vertScene = read_file(SHADERS_DIR "scene.spv");
        std::vector<char> fragTextured =
            read_file(SHADERS_DIR "pbr_bindless.spv");
        m_descriptor_set_handler
            ->CreateTexturedPBRDescriptorSetLayoutBindless();
        CreatePBRPipelineBindless(vertScene, fragTextured,
                                  &m_vulkan_global_state->m_pbrPipeline);
    } else {
        // Create Per-Mesh Descriptor Set Layout
        m_descriptor_set_handler->CreateTexturedPBRDescriptorSetLayout();
        std::vector<char> vertScene = read_file(SHADERS_DIR "scene.spv");
        std::vector<char> fragTextured = read_file(SHADERS_DIR "pbr.spv");
        CreatePBRPipeline(vertScene, fragTextured,
                          &m_vulkan_global_state->m_pbrPipeline);
    }

    m_buffer_handler->CreateComputeSSBOs();
    m_buffer_handler->CreateCommandBuffers();
    m_buffer_handler->CreateCommandBuffersCompute(
        m_device_handler->m_logical_device,
        m_vulkan_global_state->m_commandPoolCompute);

    CreateSyncObjects();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |=
        ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    // ImGui::StyleColorsLight();
    // float main_scale =
    //    ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
    //// Setup scaling
    // ImGuiStyle& style = ImGui::GetStyle();
    // style.ScaleAllSizes(main_scale);
    // style.FontScaleDpi = main_scale;
    //                   // io.ConfigDpiScaleFonts=true automatically overrides
    //                   this
    //                   // for every window depending on the current monitor)

    // Setup Platform/Renderer backends
    ImGui_ImplVulkan_InitInfo init_info = {};

    ImGui_ImplGlfw_InitForVulkan(m_windowing_backend->m_window, true);
    init_info.ApiVersion = VK_API_VERSION_1_3;  // Pass in your
    // value of VkApplicationInfo::apiVersion, otherwise will default to
    // header / version.
    init_info.Instance = m_vulkan_global_state->m_instance;
    init_info.PhysicalDevice = m_device_handler->PhysicalDevice;
    init_info.Device = m_device_handler->m_logical_device;

    init_info.QueueFamily = queue_family_indices.graphicsFamily.value();
    init_info.Queue = m_vulkan_global_state->m_graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    // init_info.DescriptorPool = VK_NULL_HANDLE;
    if (m_vulkan_global_state->m_renderingMode == ERenderingMode::Bindless) {
        init_info.DescriptorPool =
            m_vulkan_global_state->m_descriptorPoolBindlessScene;

    } else {
        init_info.DescriptorPool = m_vulkan_global_state->m_descriptorPool;
    }
    init_info.MinImageCount = 2;
    init_info.ImageCount = m_maxFramesInFlight;
    // init_info.Allocator = g_Allocator;
    init_info.PipelineInfoMain.RenderPass =
        m_vulkan_global_state->m_render_pass_data->render_pass;

    assert(init_info.PipelineInfoMain.RenderPass != VK_NULL_HANDLE);
    init_info.PipelineInfoMain.Subpass = 0;
    init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    // init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_LoadFunctions(
        VK_API_VERSION_1_3,
        [](const char* function_name, void* user_data) -> PFN_vkVoidFunction {
            VkInstance instance = *static_cast<VkInstance*>(user_data);
            return vkGetInstanceProcAddr(instance, function_name);
        },
        &m_vulkan_global_state->m_instance);
    ImGui_ImplVulkan_Init(&init_info);

    // m_vulkanGlobalState->m_computeInputBuffer = {2.0f};
    m_buffer_handler->CreateCommandBuffersCompute(
        m_device_handler->m_logical_device,
        m_vulkan_global_state->m_commandPoolCompute);
}

void VulkanContext::describe_scene(std::vector<ObjResource> objectsToRender,
                                   std::vector<Light> lightsToRender) {
    m_objects_to_render = objectsToRender;

    LightUBO lightUBO;
    int i = 0;
    for (auto light : lightsToRender) {
        light.position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        lightUBO.lights[i] = light;
        i++;
        lightUBO.numLights.x = i;
    }

    m_vulkan_global_state->m_lights = lightUBO;
    load_scene_objects();
    // need create a shadowpass framebuffer per shadow casting light
    // WARN: when we eventually reimplement shadowmaps for bindless pbr we
    // will need to flip this dude on
    // m_renderPassHandler->CreateShadowPassFrameBuffers();
    // m_vulkanGlobalState->m_shadowBias = glm::vec4(0.0005f, 0.0f, 0.0f, 0.0f);
    // m_vulkanGlobalState->m_pbrPushConstants = {
    //     glm::mat4(1.0f), m_vulkanGlobalState->m_shadowBias};
}
// Returns the index of the registered texture so it can be accessed in the
// bindless shader
uint32_t VulkanContext::register_texture(VkImageView* textureImageView,
                                         VkSampler* textureSampler) {
    uint32_t idx;
    // Find free slot
    for (int i = 0; i < 22000; i++) {
        // lmao
        if (m_textureHeap[i] == 0) {
            idx = i;
            m_textureHeap[i] = 1;
            break;
        }
    }

    std::cout << "IDX " << idx << "\n";
    // Get free index in our global textures array
    VkDescriptorImageInfo imageInfo{*textureSampler, *textureImageView,
                                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
    VkWriteDescriptorSet w{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    // CREATE THE DESCRIPTOR SET YOU DOLT
    w.dstSet = m_vulkan_global_state->m_globalBindlessDescriptorSet;
    w.dstBinding = 0;
    w.dstArrayElement = idx;
    w.descriptorCount = 1;
    w.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    w.pImageInfo = &imageInfo;
    vkUpdateDescriptorSets(m_device_handler->m_logical_device, 1, &w, 0,
                           nullptr);
    return idx;  // store in the material
}

void VulkanContext::tick(std::vector<ObjResource> objectsToRender,
                         float deltaTime) {
    m_timeAccumulator += deltaTime;

    if (m_timeAccumulator > 1.0f) {
        // NewLandscape();
        //  std::thread t([this]() { NewLandscape(); });
        //  t.detach();
        m_timeAccumulator = 0.0f;
    }
    context_vk::RenderUI();

    m_cameraController->HandleInputOrbit(deltaTime);

    memcpy(m_vulkan_global_state
               ->m_lightUBOMapped[m_vulkan_global_state->m_currentFrame],
           &m_vulkan_global_state->m_lights,
           sizeof(m_vulkan_global_state->m_lights));

    m_cameraController->UpdateCameraPosition(
        m_vulkan_global_state->m_currentFrame, deltaTime);

    if (!m_vulkan_global_state->m_meshList.empty()) {
        int count = 0;
        for (auto& mesh : m_vulkan_global_state->m_meshList) {
            if (count > 0) break;
            ModelUBO modelUBO{};
            modelUBO.model = glm::mat4(1.0f);
            modelUBO.model =
                glm::translate(modelUBO.model, glm::vec3(0.0f, 0.0f, 0.0f));
            modelUBO.model = glm::rotate(modelUBO.model, -glm::radians(90.0f),
                                         glm::vec3(0.0f, 1.0f, 0.0f));
            modelUBO.model =
                glm::scale(modelUBO.model, glm::vec3(0.5f, 0.5f, 0.5f));
            // glm::scale(modelUBO.model, glm::vec3(00.1f, 00.1f, 00.1f));
            if (glfwGetMouseButton(m_windowing_backend->m_window,
                                   GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
            }

            memcpy(mesh.m_uniformBuffersMapped[m_vulkan_global_state
                                                   ->m_currentFrame],
                   &modelUBO, sizeof(modelUBO));
            count++;
        }
    }

    DrawFrame(deltaTime);
}

void VulkanContext::load_scene_objects() {
    int meshCount = 0;
    float xPos = 0.0f;
    for (auto& obj : m_objects_to_render) {
        Mesh3D mesh =
            LoadMesh(obj.objProperties, glm::vec3(xPos, 0.0f, 0.0f),
                     glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(5.0f, 5.0f, 5.0f));
        std::vector<MaterialEnums> materialTypes = {
            ALBEDO, METALLIC, ROUGHNESS, AO, NORMAL, EMISSIVE, ORM};
        if (m_vulkan_global_state->m_renderingMode ==
            ERenderingMode::Bindless) {
            for (auto& matType : materialTypes) {
                if (!obj.textureResourceMap.contains(matType)) {
                    std::cout << "DOES NOT CONTAIN " << matType << "\n";
                    mesh.m_materialIndices[matType] = -1;
                    continue;
                }
                CreateTextureImage(obj.textureResourceMap[matType], &mesh,
                                   matType);
                CreateTextureImageView(&mesh, matType);
                CreateTextureSampler(&mesh, matType);

                std::cout << "REGISTERING " << matType << "\n";
                mesh.m_materialIndices[matType] = register_texture(
                    &mesh.m_materialDataMap[matType].m_textureImageView,
                    &mesh.m_materialDataMap[matType].m_textureSampler);
            }

            // Create Mesh Uniform Buffers
            m_buffer_handler->CreateModelUBO(&mesh);
            meshCount++;
            xPos += 5.0f;
            m_descriptor_set_handler->CreateTexturedPBRDescriptorSetsBindless(
                &mesh);
        } else {
            for (auto& matType : materialTypes) {
                CreateTextureImage(obj.textureResourceMap[matType], &mesh,
                                   matType);
                CreateTextureImageView(&mesh, matType);
                CreateTextureSampler(&mesh, matType);
                m_descriptor_set_handler->CreateTexturedPBRDescriptorSets(
                    &mesh);
            }

            // Create Mesh Uniform Buffers
            m_buffer_handler->CreateModelUBO(&mesh);

            meshCount++;
            xPos += 5.0f;
        }
        m_vulkan_global_state->m_meshList.push_back(mesh);
    }

    Mesh3D landscape = Mesh3D();
    landscape.MeshType = TEXTURED;
    landscape.m_indexBufferStartIndex = m_indices.size();
    landscape.m_vertexBufferStartIndex = m_vertices.size();
    run_compute_shader();

    Vertex* landscapeVertices = static_cast<Vertex*>(
        m_vulkan_global_state->m_computeOutputBufferMemoryMapped);
    context_vk::SetIndices(m_indices, landscapeVertices, 1024, 1024);

    for (int i : {0, 1, 1023, 1024, 500000})
        printf("v%d pos %f %f %f  n %f %f %f\n", i, landscapeVertices[i].pos.x,
               landscapeVertices[i].pos.y, landscapeVertices[i].pos.z,
               landscapeVertices[i].normal.x, landscapeVertices[i].normal.y,
               landscapeVertices[i].normal.z);

    m_vertices.insert(m_vertices.end(), landscapeVertices,
                      landscapeVertices + (1024 * 1024));

    // TODO: Textures in the engine textures folder should be loaded and
    // accessible through handles that can be passed into the create texture
    // image functions so we dont have to do filesystem shit in the future
    TextureResource tex =
        FileLoader::LoadTextureFile(MODELS_DIR "EngineDefault/DefaultTile.png");

    CreateTextureImage(tex, &landscape, ALBEDO);
    CreateTextureImageView(&landscape, ALBEDO);
    CreateTextureSampler(&landscape, ALBEDO);

    // std::vector<MaterialEnums> materialTypes = {
    //     ALBEDO, METALLIC, ROUGHNESS, AO, NORMAL, EMISSIVE, ORM};
    landscape.m_materialIndices[ALBEDO] = -1;
    landscape.m_materialIndices[METALLIC] = -1;
    landscape.m_materialIndices[ROUGHNESS] = -1;
    landscape.m_materialIndices[AO] = -1;
    landscape.m_materialIndices[EMISSIVE] = -1;
    landscape.m_materialIndices[NORMAL] = -1;
    landscape.m_materialIndices[ORM] = -1;
    // RegisterTexture(&landscape.m_materialDataMap[ALBEDO].m_textureImageView,
    //                 &landscape.m_materialDataMap[ALBEDO].m_textureSampler);

    m_buffer_handler->CreateModelUBO(&landscape);

    m_descriptor_set_handler->CreateTexturedPBRDescriptorSetsBindless(
        &landscape);

    int vertexBufferEndIndex = m_vertices.size() - 1;
    int indexBufferEndIndex = m_indices.size() - 1;
    landscape.m_indexBufferEndIndex = indexBufferEndIndex;
    landscape.m_vertexBufferEndIndex = vertexBufferEndIndex;
    landscape.m_indexCount =
        m_indices.size() - landscape.m_indexBufferStartIndex;

    // Loop through all the vertices spat out by our compute shader and
    // create indices buffer

    m_vulkan_global_state->m_meshList.push_back(landscape);
    m_buffer_handler->CreateVertexBuffer(m_vertices);
    m_buffer_handler->CreateIndexBuffer(m_indices);
}

void VulkanContext::new_landscape() {
    run_compute_shader();
    Vertex* landscapeVertices = static_cast<Vertex*>(
        m_vulkan_global_state->m_computeOutputBufferMemoryMapped);
    std::copy(
        landscapeVertices, landscapeVertices + (1024 * 1024),
        m_vertices.begin() +
            m_vulkan_global_state->m_meshList[0].m_vertexBufferStartIndex);
    vkDeviceWaitIdle(
        m_device_handler
            ->m_logical_device);  // GPU must not be reading the old buffer
    // destroy old vertex buffer + memory here (whatever your cleanup does)
    m_buffer_handler->CreateVertexBuffer(m_vertices);
    // m_vertices.assign(&landscapeVertices[0], &landscapeVertices[1024 *
    // 1024]);
}

void VulkanContext::unload_scene_objects() {
    if (!m_vulkan_global_state->m_meshList.empty()) {
        vkQueueWaitIdle(m_vulkan_global_state->m_presentQueue);
        vkDestroyDescriptorPool(m_device_handler->m_logical_device,
                                m_vulkan_global_state->m_descriptorPool,
                                nullptr);
        for (auto& mesh : m_vulkan_global_state->m_meshList) {
            vkDestroySampler(m_device_handler->m_logical_device,
                             mesh.m_textureSampler, nullptr);
            vkDestroyImageView(m_device_handler->m_logical_device,
                               mesh.m_textureImageView, nullptr);
            vkDestroyImage(m_device_handler->m_logical_device,
                           mesh.m_textureImage, nullptr);
            vkFreeMemory(m_device_handler->m_logical_device,
                         mesh.m_textureImageMemory, nullptr);
            int bufCount = 0;
            for (auto& buffer : mesh.m_uniformBuffers) {
                vkDestroyBuffer(m_device_handler->m_logical_device, buffer,
                                nullptr);
                vkFreeMemory(m_device_handler->m_logical_device,
                             mesh.m_uniformBuffersMemory[bufCount], nullptr);
                bufCount++;
            }
        }
        m_buffer_handler->DestroyBuffers();
        m_vulkan_global_state->m_meshList = {};
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
    viewport.width = (float)m_vulkan_global_state->GetSwapChainExtent().width;
    viewport.height = (float)m_vulkan_global_state->GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Determines which part of the framebuffers will be rasterized.
    // Here we set it to the swapchain images so we can see the whole
    // framebuffer.
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_vulkan_global_state->GetSwapChainExtent();

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
    m_vulkan_global_state->m_texturedMeshDescriptorSetLayouts.push_back(
        m_vulkan_global_state->m_sceneDescriptorSetLayout);
    m_vulkan_global_state->m_texturedMeshDescriptorSetLayouts.push_back(
        m_vulkan_global_state->m_texturedPBRDescriptorSetLayout);
    pipelineLayoutInfo.setLayoutCount =
        m_vulkan_global_state->m_texturedMeshDescriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts =
        m_vulkan_global_state->m_texturedMeshDescriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(
            m_device_handler->m_logical_device, &pipelineLayoutInfo, nullptr,
            &m_vulkan_global_state->m_pbrPipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.layout = m_vulkan_global_state->m_pbrPipelineLayout;
    pipelineInfo.renderPass =
        m_vulkan_global_state->m_render_pass_data->render_pass;

    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(m_device_handler->m_logical_device,
                                  VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device_handler->m_logical_device, fragShaderModule,
                          nullptr);
    vkDestroyShaderModule(m_device_handler->m_logical_device, vertShaderModule,
                          nullptr);
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
    viewport.width = (float)m_vulkan_global_state->m_shadowMapWidth;
    viewport.height = (float)m_vulkan_global_state->m_shadowMapHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Determines which part of the framebuffers will be rasterized.
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    VkExtent2D shadowMapExtent = {
        static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapWidth),
        static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapHeight)};
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
            m_device_handler->m_logical_device, &pipelineLayoutInfo, nullptr,
            &m_vulkan_global_state->m_shadowPassPipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.layout = m_vulkan_global_state->m_shadowPassPipelineLayout;
    pipelineInfo.renderPass = m_vulkan_global_state->m_shadowPass;
    //  Subpass INDEX
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(m_device_handler->m_logical_device,
                                  VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow pass pipeline!");
    }

    vkDestroyShaderModule(m_device_handler->m_logical_device, fragShaderModule,
                          nullptr);

    vkDestroyShaderModule(m_device_handler->m_logical_device, vertShaderModule,
                          nullptr);
}

void VulkanContext::CreatePBRPipelineBindless(std::vector<char> vertShaderCode,
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
    viewport.width = (float)m_vulkan_global_state->GetSwapChainExtent().width;
    viewport.height = (float)m_vulkan_global_state->GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Determines which part of the framebuffers will be rasterized.
    // Here we set it to the swapchain images so we can see the whole
    // framebuffer.
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_vulkan_global_state->GetSwapChainExtent();

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

    VkPushConstantRange vertPushConstantRange{};
    vertPushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    vertPushConstantRange.offset = 0;
    vertPushConstantRange.size = sizeof(VertexShaderPushConstants);

    VkPushConstantRange fragPushConstantRange{};
    fragPushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragPushConstantRange.offset = 64;
    fragPushConstantRange.size = sizeof(FragShaderPushConstantsBindless);

    std::array<VkPushConstantRange, 2> pushConstantRanges;
    pushConstantRanges[0] = vertPushConstantRange;
    pushConstantRanges[1] = fragPushConstantRange;

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    std::vector<VkDescriptorSetLayout> bindlessDescriptorSetLayouts;
    bindlessDescriptorSetLayouts.push_back(
        m_vulkan_global_state->m_sceneDescriptorSetLayout);
    bindlessDescriptorSetLayouts.push_back(
        m_vulkan_global_state->m_texturedPBRDescriptorSetLayoutBindless);
    bindlessDescriptorSetLayouts.push_back(
        m_vulkan_global_state->m_bindlessDescriptorSetLayout);
    pipelineLayoutInfo.setLayoutCount = bindlessDescriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = bindlessDescriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 2;
    pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data();

    if (vkCreatePipelineLayout(
            m_device_handler->m_logical_device, &pipelineLayoutInfo, nullptr,
            &m_vulkan_global_state->m_pbrPipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.layout = m_vulkan_global_state->m_pbrPipelineLayout;
    pipelineInfo.renderPass =
        m_vulkan_global_state->m_render_pass_data->render_pass;

    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(m_device_handler->m_logical_device,
                                  VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline!");
    }

    vkDestroyShaderModule(m_device_handler->m_logical_device, fragShaderModule,
                          nullptr);
    vkDestroyShaderModule(m_device_handler->m_logical_device, vertShaderModule,
                          nullptr);
}

void VulkanContext::CreateShadowPassPipelineBindless(
    std::vector<char> vertShaderCode, std::vector<char> fragShaderCode,
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
    viewport.width = (float)m_vulkan_global_state->m_shadowMapWidth;
    viewport.height = (float)m_vulkan_global_state->m_shadowMapHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Determines which part of the framebuffers will be rasterized.
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    VkExtent2D shadowMapExtent = {
        static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapWidth),
        static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapHeight)};
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
            m_device_handler->m_logical_device, &pipelineLayoutInfo, nullptr,
            &m_vulkan_global_state->m_shadowPassPipelineLayout) != VK_SUCCESS) {
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
    pipelineInfo.layout = m_vulkan_global_state->m_shadowPassPipelineLayout;
    pipelineInfo.renderPass = m_vulkan_global_state->m_shadowPass;
    //  Subpass INDEX
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;  // Optional
    pipelineInfo.basePipelineIndex = -1;               // Optional

    if (vkCreateGraphicsPipelines(m_device_handler->m_logical_device,
                                  VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                  pipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shadow pass pipeline!");
    }

    vkDestroyShaderModule(m_device_handler->m_logical_device, fragShaderModule,
                          nullptr);

    vkDestroyShaderModule(m_device_handler->m_logical_device, vertShaderModule,
                          nullptr);
}
void VulkanContext::create_command_pool() {
    QueueFamilyIndices queueFamilyIndices =
        m_device_handler->find_queue_families(m_device_handler->PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // Selecting graphicsFamily in order to issue draw commands in this command
    // pool
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device_handler->m_logical_device, &poolInfo,
                            nullptr, &m_vulkan_global_state->m_commandPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

VkCommandPool VulkanContext::create_command_pool_compute(
    VkDevice& logical_device, QueueFamilyIndices queueFamilyIndices) {
    VkCommandPool commandPool;
    // QueueFamilyIndices queueFamilyIndices =
    //     m_deviceHandler->FindQueueFamilies(m_deviceHandler->PhysicalDevice);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    // Selecting graphicsFamily in order to issue draw commands in this command
    // pool
    poolInfo.queueFamilyIndex = queueFamilyIndices.computeFamily.value();

    if (vkCreateCommandPool(logical_device, &poolInfo, nullptr, &commandPool) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
    return commandPool;
}

// Pass in the Model3D's texture path
void VulkanContext::CreateTextureImage(TextureResource props, Mesh3D* mesh,
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

    m_buffer_handler->CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                   stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_device_handler->m_logical_device, stagingBufferMemory, 0,
                imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_device_handler->m_logical_device, stagingBufferMemory);

    // WARN: Potential for memory leak here
    stbi_image_free(pixels);

    m_image_handler->CreateImage(
        texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        mesh->m_materialDataMap[materialType].m_textureImage,
        mesh->m_materialDataMap[materialType].m_textureImageMemory);

    m_image_handler->TransitionImageLayout(
        mesh->m_materialDataMap[materialType].m_textureImage,
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    CopyBufferToImage(
        stagingBuffer, mesh->m_materialDataMap[materialType].m_textureImage,
        static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
    m_image_handler->TransitionImageLayout(
        mesh->m_materialDataMap[materialType].m_textureImage,
        VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(m_device_handler->m_logical_device, stagingBuffer, nullptr);
    vkFreeMemory(m_device_handler->m_logical_device, stagingBufferMemory,
                 nullptr);
}

// TEXTURE IMAGE VIEW
void VulkanContext::CreateTextureImageView(Mesh3D* mesh,
                                           MaterialEnums materialType) {
    mesh->m_materialDataMap[materialType].m_textureImageView =
        m_image_handler->CreateImageView(
            m_device_handler->m_logical_device,
            mesh->m_materialDataMap[materialType].m_textureImage,
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
    vkGetPhysicalDeviceProperties(m_device_handler->PhysicalDevice,
                                  &properties);
    std::cout << "MAX COMPUTE WORK GROUP SIZE "
              << properties.limits.maxComputeWorkGroupSize[0] << "\n"
              << "MAX COMPUTE WORK GROUP COUNT "
              << properties.limits.maxComputeWorkGroupCount[0] << "\n"
              << "MAX COMPUTE WORK GROUP INVOCATIONS "
              << properties.limits.maxComputeWorkGroupInvocations << "\n"
              << "DEVICE NAME " << properties.deviceName << "\n"
              << "DEVICE TYPE " << properties.deviceType << "\n";
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
            m_device_handler->m_logical_device, &samplerInfo, nullptr,
            &mesh->m_materialDataMap[materialType].m_textureSampler) !=
        VK_SUCCESS) {
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
    vkGetPhysicalDeviceProperties(m_device_handler->PhysicalDevice,
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
            m_device_handler->m_logical_device, &samplerInfo, nullptr,
            &m_vulkan_global_state->m_shadowPassTextureSampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}
Mesh3D VulkanContext::LoadMesh(ObjProperties props, glm::vec3 scenePosition,
                               glm::vec3 objectRotation,
                               glm::vec3 objectScale) {
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
    return mesh;
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
    if (m_vulkan_global_state->m_renderingMode == ERenderingMode::Bindless) {
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(
            m_vulkan_global_state->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(
            m_vulkan_global_state->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = m_vulkan_global_state->GetSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass =
            m_vulkan_global_state->m_render_pass_data->render_pass;
        // DEBUG
        // printf("imageIndex DEBUG: %d\n", imageIndex);
        renderPassInfo.framebuffer =
            m_vulkan_global_state->m_render_pass_data->framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent =
            m_vulkan_global_state->GetSwapChainExtent();

        // VK_ATTACHMENT_LOAD_OP_3 clear values for color and depth stencil
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount =
            static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        // BIND SCENE DESCRIPTOR SET
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_vulkan_global_state->m_pbrPipelineLayout, 0, 1,
            &m_vulkan_global_state
                 ->m_sceneDescriptorSets[m_vulkan_global_state->m_currentFrame],
            0, nullptr);
        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_vulkan_global_state->m_pbrPipelineLayout, 2, 1,
            &m_vulkan_global_state->m_globalBindlessDescriptorSet, 0, nullptr);
        std::vector<VkDescriptorSet> descriptorSetsToBind;
        if (!m_vulkan_global_state->m_meshList.empty()) {
            VkBuffer vertexBuffers[] = {m_buffer_handler->VertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_buffer_handler->IndexBuffer,
                                 0, VK_INDEX_TYPE_UINT32);
            for (auto& mesh : m_vulkan_global_state->m_meshList) {
                mesh.m_scale = glm::vec3(100.0f, 100.0f, 100.0f);
                // Binding the graphics pipeline
                if (mesh.MeshType == TEXTURED) {
                    vkCmdBindPipeline(commandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      m_vulkan_global_state->m_pbrPipeline);
                    // BIND PER MESH DESCRIPTOR SET
                    vkCmdBindDescriptorSets(
                        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_vulkan_global_state->m_pbrPipelineLayout, 1, 1,
                        &mesh.m_descriptorSets[m_vulkan_global_state
                                                   ->m_currentFrame],
                        0, nullptr);
                }

                m_vulkan_global_state->m_fragShaderPushConstantsBindless
                    .albedoIdx = mesh.m_materialIndices[MaterialEnums::ALBEDO];
                // std::cout << m_vulkanGlobalState
                //                  ->m_fragShaderPushConstantsBindless.albedoIdx
                //           << "\n";
                m_vulkan_global_state->m_fragShaderPushConstantsBindless
                    .metallicIdx =
                    mesh.m_materialIndices[MaterialEnums::METALLIC];
                m_vulkan_global_state->m_fragShaderPushConstantsBindless
                    .roughnessIdx =
                    mesh.m_materialIndices[MaterialEnums::ROUGHNESS];
                m_vulkan_global_state->m_fragShaderPushConstantsBindless.aoIdx =
                    mesh.m_materialIndices[MaterialEnums::AO];
                m_vulkan_global_state->m_fragShaderPushConstantsBindless
                    .normalIdx = mesh.m_materialIndices[MaterialEnums::NORMAL];
                m_vulkan_global_state->m_fragShaderPushConstantsBindless
                    .emissiveIdx =
                    mesh.m_materialIndices[MaterialEnums::EMISSIVE];
                m_vulkan_global_state->m_fragShaderPushConstantsBindless
                    .ormIdx = mesh.m_materialIndices[MaterialEnums::ORM];

                vkCmdPushConstants(
                    commandBuffer, m_vulkan_global_state->m_pbrPipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT, 0,
                    sizeof(VertexShaderPushConstants),
                    &m_vulkan_global_state->m_vertexShaderPushConstants);
                vkCmdPushConstants(
                    commandBuffer, m_vulkan_global_state->m_pbrPipelineLayout,
                    VK_SHADER_STAGE_FRAGMENT_BIT, 64,
                    sizeof(FragShaderPushConstantsBindless),
                    &m_vulkan_global_state->m_fragShaderPushConstantsBindless);

                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(mesh.m_indexCount), 1,
                                 mesh.m_indexBufferStartIndex, 0, 0);
            }
        }
    } else {
        // We set the viewport and scissor state as dynamic in the pipeline
        // We need to set those up in the command buffer now
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width =
            static_cast<float>(m_vulkan_global_state->m_shadowMapWidth);
        viewport.height =
            static_cast<float>(m_vulkan_global_state->m_shadowMapHeight);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkExtent2D shadowMapExtent = {
            static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapWidth),
            static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapHeight)};
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = shadowMapExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkRenderPassBeginInfo shadowPassInfo{};
        shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        shadowPassInfo.renderPass = m_vulkan_global_state->m_shadowPass;
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
            m_vulkan_global_state->m_shadowPassFrameBuffers[0];
        vkCmdBeginRenderPass(commandBuffer, &shadowPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_vulkan_global_state->m_shadowPassPipeline);

        if (!m_vulkan_global_state->m_meshList.empty()) {
            VkBuffer vertexBuffers[] = {m_buffer_handler->VertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_buffer_handler->IndexBuffer,
                                 0, VK_INDEX_TYPE_UINT32);
            for (auto& mesh : m_vulkan_global_state->m_meshList) {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
                model = glm::rotate(model, -glm::radians(90.0f),
                                    glm::vec3(0.0f, 1.0f, 0.0f));
                model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));

                m_vulkan_global_state->m_shadowPassPushConstants.model = model;

                vkCmdPushConstants(
                    commandBuffer,
                    m_vulkan_global_state->m_shadowPassPipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT, 0,
                    sizeof(ShadowPassPushConstants),
                    &m_vulkan_global_state->m_shadowPassPushConstants);
                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(mesh.m_indexCount), 1,
                                 mesh.m_indexBufferStartIndex, 0, 0);
            }
        }
        vkCmdEndRenderPass(commandBuffer);
        //}

        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(
            m_vulkan_global_state->GetSwapChainExtent().width);
        viewport.height = static_cast<float>(
            m_vulkan_global_state->GetSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        scissor.offset = {0, 0};
        scissor.extent = m_vulkan_global_state->GetSwapChainExtent();
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass =
            m_vulkan_global_state->m_render_pass_data->render_pass;
        // DEBUG
        // printf("imageIndex DEBUG: %d\n", imageIndex);
        renderPassInfo.framebuffer =
            m_vulkan_global_state->m_renderPassFrameBuffers[imageIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent =
            m_vulkan_global_state->GetSwapChainExtent();

        // VK_ATTACHMENT_LOAD_OP_3 clear values for color and depth stencil
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount =
            static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                             VK_SUBPASS_CONTENTS_INLINE);

        vkCmdBindDescriptorSets(
            commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
            m_vulkan_global_state->m_pbrPipelineLayout, 0, 1,
            &m_vulkan_global_state
                 ->m_sceneDescriptorSets[m_vulkan_global_state->m_currentFrame],
            0, nullptr);
        std::vector<VkDescriptorSet> descriptorSetsToBind;
        if (!m_vulkan_global_state->m_meshList.empty()) {
            VkBuffer vertexBuffers[] = {m_buffer_handler->VertexBuffer};
            VkDeviceSize offsets[] = {0};
            vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

            vkCmdBindIndexBuffer(commandBuffer, m_buffer_handler->IndexBuffer,
                                 0, VK_INDEX_TYPE_UINT32);
            for (auto& mesh : m_vulkan_global_state->m_meshList) {
                mesh.m_scale = glm::vec3(0.01f, 0.01f, 0.01f);
                // Binding the graphics pipeline
                if (mesh.MeshType == TEXTURED) {
                    vkCmdBindPipeline(commandBuffer,
                                      VK_PIPELINE_BIND_POINT_GRAPHICS,
                                      m_vulkan_global_state->m_pbrPipeline);
                    vkCmdBindDescriptorSets(
                        commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                        m_vulkan_global_state->m_pbrPipelineLayout, 1, 1,
                        &mesh.m_descriptorSets[m_vulkan_global_state
                                                   ->m_currentFrame],
                        0, nullptr);
                }
                /*
                    vkCmdDraw(VkCommandBuffer, vertexCount, instanceCount,
                   firstVertex, firstInstance) vertexCount: Even though we don't
                   have a vertex buffer, we technically still have 3 vertices to
                   draw. instanceCount: Used for instanced rendering, use 1 if
                   you're not doing that. firstVertex: Used as an offset into
                   the vertex buffer, defines the lowest value of
                   gl_VertexIndex. firstInstance: Used as an offset for
                   instanced rendering, defines the lowest value of
                   gl_InstanceIndex.
                */
                // vkCmdDraw(commandBuffer,
                // static_cast<uint32_t>(vertices.size()), 1, 0, 0);
                // m_vulkanGlobalState->m_pbrPushConstants = {
                //    m_vulkanGlobalState->m_shadowPassPushConstants.lightViewProj,
                //    m_vulkanGlobalState->m_shadowBias};

                m_vulkan_global_state->m_pbrPushConstants.lightViewProj =
                    m_vulkan_global_state->m_shadowPassPushConstants
                        .lightViewProj;

                m_vulkan_global_state->m_pbrPushConstants.bias =
                    m_vulkan_global_state->m_shadowBias;

                vkCmdPushConstants(
                    commandBuffer, m_vulkan_global_state->m_pbrPipelineLayout,
                    VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                    0, sizeof(PBRPushConstants),
                    &m_vulkan_global_state->m_pbrPushConstants);
                vkCmdDrawIndexed(commandBuffer,
                                 static_cast<uint32_t>(mesh.m_indexCount), 1,
                                 mesh.m_indexBufferStartIndex, 0, 0);
            }
        }
    }

    ImDrawData* draw_data = ImGui::GetDrawData();

    ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
    ImGui::EndFrame();
}

void VulkanContext::RecordCommandBufferCompute(VkCommandBuffer commandBuffer,
                                               VkPipeline pipeline,
                                               VkPipelineLayout pipelineLayout,
                                               VkDescriptorSet* descriptorSet) {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error(
            "failed to begin recording compute command buffer!");
    }

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE,
                            pipelineLayout, 0, 1, descriptorSet, 0, NULL);

    constexpr uint32_t kLocalSizeX = 1024;  // same as local_size_x in shader
    constexpr uint32_t kVertexCount = 1024u * 1024u;
    vkCmdDispatch(commandBuffer, (kVertexCount + kLocalSizeX - 1) / kLocalSizeX,
                  1, 1);  // 1024 groups
    // vkCmdDispatch(commandBuffer, ((1024 * 2) + 1023) / 1024, 1, 1);

    VkMemoryBarrier mb{VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    mb.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    mb.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    vkCmdPipelineBarrier(m_vulkan_global_state->m_commandBuffersCompute[0],
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &mb, 0, nullptr, 0,
                         nullptr);

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
    viewport.width =
        static_cast<float>(m_vulkan_global_state->m_shadowMapWidth);
    viewport.height =
        static_cast<float>(m_vulkan_global_state->m_shadowMapHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkExtent2D shadowMapExtent = {
        static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapWidth),
        static_cast<uint32_t>(m_vulkan_global_state->m_shadowMapHeight)};
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = shadowMapExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo shadowPassInfo{};
    shadowPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    shadowPassInfo.renderPass = m_vulkan_global_state->m_shadowPass;
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
        m_vulkan_global_state->m_shadowPassFrameBuffers[0];
    vkCmdBeginRenderPass(commandBuffer, &shadowPassInfo,
                         VK_SUBPASS_CONTENTS_INLINE);

    // DRAW CALLS
    // SCENE
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_vulkan_global_state->m_shadowPassPipeline);

    if (!m_vulkan_global_state->m_meshList.empty()) {
        VkBuffer vertexBuffers[] = {m_buffer_handler->VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_buffer_handler->IndexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        for (auto& mesh : m_vulkan_global_state->m_meshList) {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
            model = glm::rotate(model, -glm::radians(90.0f),
                                glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));

            m_vulkan_global_state->m_shadowPassPushConstants.model = model;

            vkCmdPushConstants(
                commandBuffer,
                m_vulkan_global_state->m_shadowPassPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(ShadowPassPushConstants),
                &m_vulkan_global_state->m_shadowPassPushConstants);
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
        static_cast<float>(m_vulkan_global_state->GetSwapChainExtent().width);
    viewport.height =
        static_cast<float>(m_vulkan_global_state->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    scissor.offset = {0, 0};
    scissor.extent = m_vulkan_global_state->GetSwapChainExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass =
        m_vulkan_global_state->m_render_pass_data->render_pass;
    // DEBUG
    // printf("imageIndex DEBUG: %d\n", imageIndex);
    renderPassInfo.framebuffer =
        m_vulkan_global_state->m_renderPassFrameBuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent =
        m_vulkan_global_state->GetSwapChainExtent();

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
        m_vulkan_global_state->m_pbrPipelineLayout, 0, 1,
        &m_vulkan_global_state
             ->m_sceneDescriptorSets[m_vulkan_global_state->m_currentFrame],
        0, nullptr);
    std::vector<VkDescriptorSet> descriptorSetsToBind;
    if (!m_vulkan_global_state->m_meshList.empty()) {
        VkBuffer vertexBuffers[] = {m_buffer_handler->VertexBuffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, m_buffer_handler->IndexBuffer, 0,
                             VK_INDEX_TYPE_UINT32);
        for (auto& mesh : m_vulkan_global_state->m_meshList) {
            // Binding the graphics pipeline
            if (mesh.MeshType == TEXTURED) {
                vkCmdBindPipeline(commandBuffer,
                                  VK_PIPELINE_BIND_POINT_GRAPHICS,
                                  m_vulkan_global_state->m_pbrPipeline);
                vkCmdBindDescriptorSets(
                    commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_vulkan_global_state->m_pbrPipelineLayout, 1, 1,
                    &mesh.m_descriptorSets[m_vulkan_global_state
                                               ->m_currentFrame],
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
            m_vulkan_global_state->m_pbrPushConstants = {
                m_vulkan_global_state->m_shadowPassPushConstants.lightViewProj,
                m_vulkan_global_state->m_shadowBias};

            vkCmdPushConstants(
                commandBuffer, m_vulkan_global_state->m_pbrPipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                sizeof(PBRPushConstants),
                &m_vulkan_global_state->m_shadowPassPushConstants
                     .lightViewProj);
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
    m_vulkan_global_state->m_imageAvailableSemaphores.resize(
        m_maxFramesInFlight);
    m_vulkan_global_state->m_renderFinishedSemaphores.resize(
        m_maxFramesInFlight);
    m_vulkan_global_state->m_inFlightFences.resize(m_maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < m_maxFramesInFlight; i++) {
        if (vkCreateSemaphore(
                m_device_handler->m_logical_device, &semaphoreInfo, nullptr,
                &m_vulkan_global_state->m_imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(
                m_device_handler->m_logical_device, &semaphoreInfo, nullptr,
                &m_vulkan_global_state->m_renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(
                m_device_handler->m_logical_device, &fenceInfo, nullptr,
                &m_vulkan_global_state->m_inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create semaphores!");
        }
    }

    if (vkCreateFence(m_device_handler->m_logical_device, &fenceInfo, nullptr,
                      &m_vulkan_global_state->m_computeFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to create compute fence!");
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

    cvk_render_pass::RenderPassData::ERenderPassDataState renderPassDataState =
        m_vulkan_global_state->m_render_pass_data.get()
            ->GetRenderPassDataState();
    if (!context_vk::CheckRenderPassDataValidity(renderPassDataState)) {
        return;
    }

    vkWaitForFences(
        m_device_handler->m_logical_device, 1,
        &m_vulkan_global_state
             ->m_inFlightFences[m_vulkan_global_state->m_currentFrame],
        VK_TRUE, UINT64_MAX);

    // Acquire an image from the swap chain
    uint32_t imageIndex;

    // SWAP CHAIN RECREATION
    VkResult result = vkAcquireNextImageKHR(
        m_device_handler->m_logical_device, m_vulkan_global_state->m_swapChain,
        UINT64_MAX,
        m_vulkan_global_state
            ->m_imageAvailableSemaphores[m_vulkan_global_state->m_currentFrame],
        VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        int frameBufferWidth, frameBufferHeight;
        // TODO: Call to Renderer Front End to get framebuffer sizes
        m_windowing_backend->get_framebuffer_size_glfw(&frameBufferWidth,
                                                       &frameBufferHeight);
        m_windowing_backend->PollWindowNotMinimizedGLFW();
        vkDeviceWaitIdle(m_device_handler->m_logical_device);
        m_swap_chain_handler->Rebuild(frameBufferWidth, frameBufferHeight);
        // TODO: MUST NOT FORGET THIS
        // m_renderPassHandler->CreateRenderPass();
        VkFormat renderPassDepthAttachmentFormat =
            m_render_pass_handler->find_supported_format(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                 VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        if (!renderPassDepthAttachmentFormat) {
            std::cout << "renderPassDepthAttachmentFormat invalid\n";
        }

        cvk_render_pass::DepthResources renderPassDepthResources =
            m_render_pass_handler->create_render_pass_depth_resources(
                renderPassDepthAttachmentFormat,
                m_vulkan_global_state->m_swapChainExtent.width,
                m_vulkan_global_state->m_swapChainExtent.height);

        cvk_render_pass::RenderPassData renderPassData =
            m_render_pass_handler->create_render_pass(
                m_device_handler->m_logical_device,
                m_vulkan_global_state->GetSwapChainImageFormat(),
                renderPassDepthAttachmentFormat);

        renderPassData.depthResources = renderPassDepthResources;

        m_vulkan_global_state->m_render_pass_data =
            std::make_unique<cvk_render_pass::RenderPassData>(renderPassData);

        if (m_vulkan_global_state->m_render_pass_data == nullptr) {
            std::cout
                << "Vulkan Init Error: m_vulkanGlobalState->m_renderPassData "
                   "is INVALID!\n";
            return;
        }

        std::vector<VkImageView> renderPassPerFrameImageViews = {};

        renderPassPerFrameImageViews.push_back(
            m_vulkan_global_state->m_render_pass_data->depthResources
                .imageView);

        m_vulkan_global_state->m_render_pass_data->framebuffers =
            context_vk::CreateSwapChainFramebuffers(
                m_vulkan_global_state->m_swapChainImageViews.size(),
                renderPassPerFrameImageViews,
                m_vulkan_global_state->m_swapChainImageViews,
                m_vulkan_global_state->GetSwapChainExtent().width,
                m_vulkan_global_state->GetSwapChainExtent().height,
                m_device_handler->m_logical_device,
                *m_vulkan_global_state->m_render_pass_data);

        if (!context_vk::CheckRenderPassDataValidity(
                renderPassData.GetRenderPassDataState())) {
            return;
        }
        m_render_pass_handler->create_shadow_pass();
        m_render_pass_handler->create_shadow_pass_framebuffers();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // Only reset fences if we are submitting work
    vkResetFences(
        m_device_handler->m_logical_device, 1,
        &m_vulkan_global_state
             ->m_inFlightFences[m_vulkan_global_state->m_currentFrame]);

    // Record a command buffer which draws the scene onto that image
    vkResetCommandBuffer(
        m_vulkan_global_state
            ->m_commandBuffers[m_vulkan_global_state->m_currentFrame],
        0);

    RecordCommandBuffer(
        m_vulkan_global_state
            ->m_commandBuffers[m_vulkan_global_state->m_currentFrame],
        imageIndex);

    // Submit the recorded command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {
        m_vulkan_global_state->m_imageAvailableSemaphores
            [m_vulkan_global_state->m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers =
        &m_vulkan_global_state
             ->m_commandBuffers[m_vulkan_global_state->m_currentFrame];

    VkSemaphore signalSemaphores[] = {
        m_vulkan_global_state->m_renderFinishedSemaphores
            [m_vulkan_global_state->m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(
            m_vulkan_global_state->m_graphicsQueue, 1, &submitInfo,
            m_vulkan_global_state
                ->m_inFlightFences[m_vulkan_global_state->m_currentFrame]) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Present swap chain image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {*m_vulkan_global_state->GetRefSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;
    // presentInfo.pResults = nullptr; // Optional

    // SWAP CHAIN RECREATION
    result =
        vkQueuePresentKHR(m_vulkan_global_state->m_presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
        m_windowing_backend->framebufferResized) {
        m_windowing_backend->framebufferResized = false;
        int frameBufferWidth, frameBufferHeight;
        m_windowing_backend->get_framebuffer_size_glfw(&frameBufferWidth,
                                                       &frameBufferHeight);
        m_windowing_backend->PollWindowNotMinimizedGLFW();
        vkDeviceWaitIdle(m_device_handler->m_logical_device);
        m_swap_chain_handler->Rebuild(frameBufferWidth, frameBufferHeight);
        // TODO: MUST NOT FORGET THIS
        // m_renderPassHandler->CreateRenderPass();
        VkFormat renderPassDepthAttachmentFormat =
            m_render_pass_handler->find_supported_format(
                {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT,
                 VK_FORMAT_D24_UNORM_S8_UINT},
                VK_IMAGE_TILING_OPTIMAL,
                VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

        if (!renderPassDepthAttachmentFormat) {
            std::cout << "renderPassDepthAttachmentFormat invalid\n";
        }

        cvk_render_pass::DepthResources renderPassDepthResources =
            m_render_pass_handler->create_render_pass_depth_resources(
                renderPassDepthAttachmentFormat,
                m_vulkan_global_state->m_swapChainExtent.width,
                m_vulkan_global_state->m_swapChainExtent.height);

        cvk_render_pass::RenderPassData renderPassData =
            m_render_pass_handler->create_render_pass(
                m_device_handler->m_logical_device,
                m_vulkan_global_state->GetSwapChainImageFormat(),
                renderPassDepthAttachmentFormat);

        renderPassData.depthResources = renderPassDepthResources;

        m_vulkan_global_state->m_render_pass_data =
            std::make_unique<cvk_render_pass::RenderPassData>(renderPassData);

        if (m_vulkan_global_state->m_render_pass_data == nullptr) {
            std::cout
                << "Vulkan Init Error: m_vulkanGlobalState->m_renderPassData "
                   "is INVALID!\n";
            return;
        }
        std::vector<VkImageView> renderPassPerFrameImageViews = {};

        renderPassPerFrameImageViews.push_back(
            m_vulkan_global_state->m_render_pass_data->depthResources
                .imageView);

        m_vulkan_global_state->m_render_pass_data->framebuffers =
            context_vk::CreateSwapChainFramebuffers(
                m_vulkan_global_state->m_swapChainImageViews.size(),
                renderPassPerFrameImageViews,
                m_vulkan_global_state->m_swapChainImageViews,
                m_vulkan_global_state->GetSwapChainExtent().width,
                m_vulkan_global_state->GetSwapChainExtent().height,
                m_device_handler->m_logical_device,
                *m_vulkan_global_state->m_render_pass_data);
        m_render_pass_handler->create_shadow_pass();
        m_render_pass_handler->create_shadow_pass_framebuffers();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to present swap chain image");
    }

    // Progress to next frame
    m_vulkan_global_state->m_currentFrame =
        (m_vulkan_global_state->m_currentFrame + 1) % m_maxFramesInFlight;
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

void VulkanContext::destroy() {
    vkDeviceWaitIdle(m_device_handler->m_logical_device);
    m_swap_chain_handler->Destroy();
    // DestroySwapChain();

    for (auto& mesh : m_vulkan_global_state->m_meshList) {
        vkDestroySampler(m_device_handler->m_logical_device,
                         mesh.m_textureSampler, nullptr);
        vkDestroyImageView(m_device_handler->m_logical_device,
                           mesh.m_textureImageView, nullptr);
        vkDestroyImage(m_device_handler->m_logical_device, mesh.m_textureImage,
                       nullptr);
        vkFreeMemory(m_device_handler->m_logical_device,
                     mesh.m_textureImageMemory, nullptr);
    }

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        vkDestroyBuffer(m_device_handler->m_logical_device,
                        m_buffer_handler->UniformBuffers[i], nullptr);
        vkFreeMemory(m_device_handler->m_logical_device,
                     m_buffer_handler->UniformBuffersMemory[i], nullptr);
    }

    vkDestroyDescriptorPool(m_device_handler->m_logical_device,
                            m_vulkan_global_state->m_descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(
        m_device_handler->m_logical_device,
        m_vulkan_global_state->m_meshDescriptorSetLayout, nullptr);

    // Buffer cleanup
    // Call BufferHandler::DestroyBuffers
    m_buffer_handler->DestroyBuffers();

    vkDestroyPipeline(m_device_handler->m_logical_device,
                      m_vulkan_global_state->m_pbrPipeline, nullptr);
    vkDestroyPipelineLayout(m_device_handler->m_logical_device,
                            m_vulkan_global_state->m_pbrPipelineLayout,
                            nullptr);

    vkDestroyRenderPass(m_device_handler->m_logical_device,
                        m_render_pass_handler->m_renderPass, nullptr);

    for (size_t i = 0; i < m_maxFramesInFlight; i++) {
        vkDestroySemaphore(m_device_handler->m_logical_device,
                           m_vulkan_global_state->m_renderFinishedSemaphores[i],
                           nullptr);
        vkDestroySemaphore(m_device_handler->m_logical_device,
                           m_vulkan_global_state->m_imageAvailableSemaphores[i],
                           nullptr);
        vkDestroyFence(m_device_handler->m_logical_device,
                       m_vulkan_global_state->m_inFlightFences[i], nullptr);
    }
    vkDestroyCommandPool(m_device_handler->m_logical_device,
                         m_vulkan_global_state->m_commandPool, nullptr);

    // DEVICE DESTRUCTION
    m_device_handler->Destroy();
    // GLFW DESTRUCTION
    m_windowing_backend->DestroyGLFW();
}

VkShaderModule VulkanContext::CreateShaderModule(
    const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(m_device_handler->m_logical_device, &createInfo,
                             nullptr, &shaderModule) != VK_SUCCESS) {
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
    VkCommandBuffer commandBuffer = begin_single_time_commands();
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
    end_single_time_commands(commandBuffer);
}

// beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
VkCommandBuffer VulkanContext::begin_single_time_commands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_vulkan_global_state->m_commandPool;
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

void VulkanContext::end_single_time_commands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(m_vulkan_global_state->m_graphicsQueue, 1, &submitInfo,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkan_global_state->m_graphicsQueue);

    vkFreeCommandBuffers(m_device_handler->m_logical_device,
                         m_vulkan_global_state->m_commandPool, 1,
                         &commandBuffer);
}

void VulkanContext::run_compute_shader() {
    vkWaitForFences(m_device_handler->m_logical_device, 1,
                    &m_vulkan_global_state->m_computeFence, VK_TRUE,
                    UINT64_MAX);

    VkResult r = vkResetFences(m_device_handler->m_logical_device, 1,
                               &m_vulkan_global_state->m_computeFence);
    // Do compute stuff
    std::vector<char> computeShaderSpv = read_file(SHADERS_DIR "compute.spv");
    VkDescriptorSetLayout layout =
        m_descriptor_set_handler->CreateComputeDescriptorSetLayout(
            m_device_handler->m_logical_device);
    VkDescriptorPool descriptorPool =
        m_descriptor_set_handler->CreateDescriptorPoolCompute(
            m_device_handler->m_logical_device);

    VkDescriptorSet descriptorSet =
        m_descriptor_set_handler->CreateDescriptorSetCompute(
            descriptorPool, layout, m_device_handler->m_logical_device);
    std::vector<VkDescriptorSetLayout> layouts = {};
    layouts.push_back(layout);
    VkPipelineLayout pipelineLayout;
    VkPipeline computePipeline = context_vk::CreateComputePipeline(
        computeShaderSpv, m_device_handler->m_logical_device, layouts,
        pipelineLayout);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 200.0f);
    std::uniform_real_distribution<float> amplitude(0.0f, 0.99f);
    std::uniform_real_distribution<float> freq(0.0f, 0.99f);
    std::uniform_real_distribution<float> octaves(1, 8);

    float* in = static_cast<float*>(
        m_vulkan_global_state->m_computeInputBufferMemoryMapped);
    in[0] = dist(gen);
    in[1] = dist(gen);
    in[2] = 2.0;
    in[3] = 0.52;
    in[4] = freq(gen);
    in[5] = 8;

    RecordCommandBufferCompute(
        m_vulkan_global_state->m_commandBuffersCompute[0], computePipeline,
        pipelineLayout, &descriptorSet);
    VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.commandBufferCount = 1;
    si.pCommandBuffers = &m_vulkan_global_state->m_commandBuffersCompute[0];

    //   printf("reset = %d\n", r);
    r = vkQueueSubmit(m_vulkan_global_state->m_computeQueue, 1, &si,
                      m_vulkan_global_state->m_computeFence);
    //    printf("submit = %d\n", r);
    // vkDeviceWaitIdle(m_deviceHandler->LogicalDevice);
}
