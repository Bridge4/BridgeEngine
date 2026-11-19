#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

// VULKAN PLATFORM DEFINITION
#include <vulkan/vulkan_core.h>

#include <iostream>

#include "Source/Renderer/API/Vulkan/Components/DescriptorSets/DescriptorSetHandler.h"
#include "Source/Renderer/API/Vulkan/Components/RenderPass/RenderPassHandler.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/Camera/CameraController.h"
#include "Source/Renderer/Mesh/Mesh3D.h"
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif  // !VK_USE_PLATFORM_WIN32_KHR

//// GLM FORCE USE RADIANS
// #ifndef GLM_FORCE_RADIANS
// #define GLM_FORCE_RADIANS
// #endif  // !GLM_FORCE_RADIANS
//
//// GLM VULKAN DATA ALIGNMENT
// #ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// #endif  // !GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
//
// #ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
// #define GLM_FORCE_DEPTH_ZERO_TO_ONE
// #endif  // !GLM_FORCE_DEPTH_ZERO_TO_ONE

// VULKAN INCLUDE

//// GLM INCLUDES
// #include <../glm/glm.hpp>
// #include <../glm/gtc/matrix_transform.hpp>
// #include <../glm/gtx/hash.hpp>
#include <vulkan/vulkan.h>

#include "Source/Math/Math.h"

// C++ STANDARD INCLUDES
#include <cstdint>  // Necessary for uint32_t
#include <filesystem>
#include <fstream>
#include <vector>
// Forward Declarations
class SwapChainHandler;
class BufferHandler;
class WindowingBackend;
class CameraController;
class VulkanGlobalState;
class Mesh3D;

namespace backend_vk {

enum EVulkanPipeline { BindlessPBR };
enum EShaderType { VERT, FRAG, COMP };
struct Shader {
    std::string source_name;
    std::string file_path;
    EShaderType type;
    std::filesystem::file_time_type last_write_time;
    std::filesystem::file_time_type current_write_time;
    std::vector<uint32_t> spirv_code;
    EVulkanPipeline pipeline;
    std::string entry_point = "main";
};

// TODO: Create a tagged union to represent this
enum EPipelineEditType { PIPELINE_INFO };

enum EPipelineInfoType { INPUT_ASSEMBLY };

// Contains the index of the edited shader
struct ShaderEdit {
    int idx;
};

// Contains the type of the pipeline info edited
struct PipelineInfoEdit {
    EPipelineInfoType type;
};

// Tagged Union to represent an edit made to a VkPipeline
struct PipelineEdit {
    EPipelineEditType edit_type;

    union Data {
        ShaderEdit shader;
        PipelineInfoEdit pipeline_info;
    } edit_data;
};

enum EPipelineType { GRAPHICS, COMPUTE };
struct GraphicsPipelineStateData {
    VkPipelineVertexInputStateCreateInfo vertex_state_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info;
    VkPipelineViewportStateCreateInfo viewport_state_info;
    VkPipelineRasterizationStateCreateInfo rasterization_state_info;
    VkPipelineMultisampleStateCreateInfo multisampler_state_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info;
    VkPipelineColorBlendStateCreateInfo color_blend_state_info;
    VkPipelineDynamicStateCreateInfo dynamic_state_info;

    std::vector<VkVertexInputBindingDescription> vertex_bindings;
    std::vector<VkVertexInputAttributeDescription> vertex_attributes;
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
    std::vector<VkDynamicState> dynamic_states;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
};

struct ComputePipelineStateData {
    VkPipelineVertexInputStateCreateInfo vertex_state_info;
    VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info;
    VkPipelineViewportStateCreateInfo viewport_state_info;
    VkPipelineRasterizationStateCreateInfo rasterization_state_info;
    VkPipelineMultisampleStateCreateInfo multisampler_state_info;
    VkPipelineDepthStencilStateCreateInfo depth_stencil_state_info;
    VkPipelineColorBlendStateCreateInfo color_blend_state_info;
    VkPipelineDynamicStateCreateInfo dynamic_state_info;

    std::vector<VkVertexInputBindingDescription> vertex_bindings;
    std::vector<VkVertexInputAttributeDescription> vertex_attributes;
    std::vector<VkPipelineColorBlendAttachmentState> color_blend_attachments;
    std::vector<VkDynamicState> dynamic_states;
    std::vector<VkPipelineShaderStageCreateInfo> shader_stages;
};
struct Pipeline {
    EPipelineType type = GRAPHICS;

    VkGraphicsPipelineCreateInfo graphics_create_info;
    GraphicsPipelineStateData graphics_pipeline_state_data;
    VkComputePipelineCreateInfo compute_create_info;
    ComputePipelineStateData compute_pipeline_state_data;

    VkPipeline* pipeline_ptr;
    VkPipelineLayout* pipeline_layout_ptr;
    VkPipeline pipeline;
    VkPipelineLayout pipeline_layout;
    std::vector<Shader> shaders;
    std::vector<PipelineEdit> edits;
    bool is_dirty;
};

std::string read_shader(const std::string& file_name);
void render_ui(VulkanContext& context_vk);

Pipeline CreateComputePipeline(
    std::vector<uint32_t> shader_code_spirv,
    const std::string& shader_file_path, VkDevice& logical_device,
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts,
    VkPipelineLayout& pipeline_layout);

std::vector<VkFramebuffer> create_swapchain_framebuffers(
    int numberOfFramebuffers, std::vector<VkImageView> perFrameImageViews,
    std::vector<VkImageView> swapChainImageViews, uint32_t framebufferWidth,
    uint32_t framebufferHeight, VkDevice& logical_device,
    render_pass::RenderPassData renderPassData);

VkCommandBuffer begin_single_time_commands(VkDevice& logical_device,
                                           VkCommandPool& commandPool);

void end_single_time_commands(VkDevice& logical_device,
                              VkCommandPool& command_pool,
                              VkQueue& graphics_queue,
                              VkCommandBuffer command_buffer);

uint32_t find_memory_type(VkPhysicalDevice& physical_device,
                          uint32_t type_filter,
                          VkMemoryPropertyFlags properties);

void set_indices(std::vector<uint32_t>& indices, Vertex* vertices,
                 int worldHeight, int worldWidth);
}  // namespace backend_vk

struct InputSSBO {
    float dataIn[1024];
};

struct OutputSSBO {
    Vertex dataOut[2048 * 2048];
};

class VulkanContext {
   public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    VulkanContext() {};
    void init_vulkan(ERenderingMode renderingMode = ERenderingMode::Standard);

    void tick(std::vector<ObjResource> objectsToRender, float deltaTime);

    void load_scene_objects();

    void unload_scene_objects();

    void new_landscape(bool rebuild_indices = false);
    bool RebuildPBRPipelineBindless(backend_vk::Pipeline& cached_pipeline);

    VulkanGlobalState* m_vulkan_global_state;
    WindowingBackend* m_windowing_backend = nullptr;

    const std::vector<const char*> m_validationLayers = {
        "VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    std::vector<Vertex> m_vertices;
    std::vector<uint32_t> m_indices;
    // TODO: Figure out what the fuck this does
    void transition_image_layout(VkImage image, VkFormat format,
                                 VkImageLayout old_layout,
                                 VkImageLayout new_layout);

    // beginSingleTimeCommands and endSingleTimeCommands are helpers for
    // copyBuffer
    VkCommandBuffer begin_single_time_commands();

    void end_single_time_commands(VkCommandBuffer command_buffer);
    // CLEANUP
    void destroy();
    void describe_scene(std::vector<ObjResource> objects_to_render,
                        std::vector<Light> lights_to_render);
    uint32_t register_texture(VkImageView* texture_image_view,
                              VkSampler* texture_sampler);

    void run_compute_shader(VkPipeline computePipeline,
                            VkPipelineLayout pipelineLayout,
                            VkDescriptorSet& descriptorSet,
                            bool is_random = false,
                            bool rebuild_indices = false);

    bool rebuild_compute_shader_pipeline(backend_vk::Pipeline& cached_pipeline);
    std::vector<ObjResource> m_objects_to_render = {};

    // Reading in SPIRV shaders
    static std::vector<char> read_file(const std::string& file_name) {
        std::ifstream file(file_name, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    static const std::string read_shader(const std::string& file_name) {
        std::ifstream file(file_name, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::string buffer;

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
    }

    float m_landscape_animation_time = 0.0f;
    float m_frame_time = 0.0f;
    int m_target_fps = 60;
    float m_monitor_refresh_rate = 60;
    bool m_vsync = false;

    CameraController* m_cameraController;

    // We need some sort of shader watch list vector that we will loop through
    // every renderer tick and check for changes to shaders
    std::vector<backend_vk::Pipeline> m_pipelines = {};
    std::vector<backend_vk::Shader> m_shaders = {};

   private:
    // DeviceHandler* m_deviceHandler;
    SwapChainHandler* m_swap_chain_handler;
    BufferHandler* m_buffer_handler;
    DescriptorSetHandler* m_descriptor_set_handler;

    int m_textureHeap[22000] = {0};

    uint32_t m_windowWidth = 800;
    uint32_t m_windowHeight = 600;

    VkDebugUtilsMessengerEXT m_debugMessenger;
    int m_maxFramesInFlight = 2;
    /*
     * Window is polled and frames are drawn until window is closed
     */

    // GRAPHICS PIPELINE
    /*
     *   Create a graphics pipeline to render
     *
     */
    void CreatePBRPipeline(std::vector<uint32_t> vertShaderCode,
                           std::vector<uint32_t> fragShaderCode,
                           VkPipeline* pipeline);

    void CreateShadowPassPipeline(std::vector<uint32_t> vertShaderCode,
                                  std::vector<uint32_t> fragShaderCode,
                                  VkPipeline* pipeline);
    // === BINDLESS === //
    backend_vk::Pipeline CreatePBRPipelineBindless(
        std::vector<uint32_t>& vert_shader_code,
        std::vector<uint32_t>& frag_shader_code,
        const std::string& vert_file_path, const std::string& frag_file_path,
        VkPipeline* pipeline);
    void CreateShadowPassPipelineBindless(std::vector<uint32_t> vertShaderCode,
                                          std::vector<uint32_t> fragShaderCode,
                                          VkPipeline* pipeline);
    // COMMAND POOL
    /*
        Command pools manage the memory used to store command buffers
        Allows for multithreaded command recording since all commands are
       available together in the buffers
    */
    void create_command_pool();
    VkCommandPool create_command_pool_compute(
        VkDevice& logical_device, QueueFamilyIndices queueFamilyIndices);

    // VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
    // VkImageTiling tiling, VkFormatFeatureFlags features);

    // TEXTURE IMAGE
    void create_texture_image(TextureResource props, Mesh3D* mesh,
                              MaterialEnums materialType,
                              bool is_cube_map = false);

    // TEXTURE IMAGE VIEW
    void create_texture_image_view(Mesh3D* mesh, MaterialEnums materialType,
                                   bool is_cube_map = false);

    // TEXTURE SAMPLER
    void create_texture_sampler(Mesh3D* mesh, MaterialEnums materialType);
    void CreateTextureSamplerShadowPass();

    // helper function used in createTextureImage();
    void CreateImage(uint32_t width, uint32_t height, VkFormat format,
                     VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image,
                     VkDeviceMemory& imageMemory);

    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);

    Mesh3D LoadMesh(ObjProperties props,
                    glm::vec3 scenePosition = glm::vec3(1.0f, 1.0f, 1.0f),
                    glm::vec3 objectRotation = glm::vec3(0.0f, 0.0f, 0.0f),
                    glm::vec3 objectScale = glm::vec3(1.0f, 1.0f, 1.0f));

    // COPY BUFFER
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    uint32_t FindMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);

    void RecordCommandBuffer(VkCommandBuffer commandBuffer,
                             uint32_t imageIndex);

    void record_command_buffer_compute(VkCommandBuffer commandBuffer,
                                       VkPipeline pipeline,
                                       VkPipelineLayout pipelineLayout,
                                       VkDescriptorSet* descriptorSet,
                                       bool rebuild_indices = false);

    // SEMAPHORES AND FENCES
    // Creates semaphores and fences for each frame in flight
    void CreateSyncObjects();

    // DRAW FRAME
    void DrawFrame(float deltaTime);

    // void updateUniformBuffer(uint32_t currentImage);

    // SHADER MODULES
    VkShaderModule CreateShaderModule(std::vector<uint32_t>& code);
};
#endif  // !VULKANCONTEXT_H
