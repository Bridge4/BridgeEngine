#ifndef VULKANGLOBALSTATE_H
#define VULKANGLOBALSTATE_H
#include <vulkan/vulkan_core.h>

#include <memory>
#include <unordered_map>
#include <vector>

#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/Components/RenderPass/RenderPassHandler.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/Mesh/Mesh3D.h"
#include "vulkan/vulkan.h"

struct VertexShaderPushConstants {
    glm::mat4 lightViewProj;
};

struct HighResQuadComputePushConstants {};

struct ShadowPassPushConstants {
    glm::mat4 lightViewProj;
    glm::mat4 model;
};

struct PBRPushConstants {
    glm::mat4 lightViewProj;
    glm::vec4 bias;
    glm::vec4 hasEmissive;
};
struct FragShaderPushConstantsBindless {
    glm::vec4 bias;
    uint32_t albedoIdx;
    uint32_t metallicIdx;
    uint32_t roughnessIdx;
    uint32_t aoIdx;
    uint32_t normalIdx;
    uint32_t emissiveIdx;
    uint32_t ormIdx;
};

class VulkanGlobalState {
   public:
    // The VulkanGlobalState will hold the resources created by the various
    // handlers

    VulkanGlobalState(VulkanContext* vulkanContext,
                      ERenderingMode renderingMode) {
        this->m_vulkanContext = vulkanContext;
        this->m_renderingMode = renderingMode;
    }

    void create_vulkan_instance();

    // Getters
    VkInstance GetVulkanInstance() { return m_instance; }
    // std::vector<VkImage>* GetRefSwapChainImages() { return
    // &m_swapChainImages; }
    std::vector<VkImageView>* GetRefSwapChainImageViews() {
        return &m_swapChainImageViews;
    }
    // TODO: Move this to SwapChainHandler
    VkFormat GetSwapChainImageFormat() { return m_swapChainImageFormat; }
    // TODO: Move this to SwapChainHandler
    VkSwapchainKHR* GetRefSwapChain() { return &m_swapChain; }
    // TODO: Move this to SwapChainHandler
    std::vector<VkImage>* GetRefSwapChainImages() { return &m_swapChainImages; }
    VkExtent2D GetSwapChainExtent() { return m_swapChainExtent; }
    // TODO: Replace all getters with direct access to member variables
    VkSurfaceKHR GetSurface() { return m_surface; }

    void SetSwapChainImageFormat(VkFormat surfaceFormat) {
        this->m_swapChainImageFormat = surfaceFormat;
    };
    void SetSwapChainExtent(VkExtent2D extent) {
        this->m_swapChainExtent = extent;
    }
    void SetSwapChainImageViews(std::vector<VkImageView> swapChainImageViews) {
        this->m_swapChainImageViews = swapChainImageViews;
    }
    void SetSwapChainImages(std::vector<VkImage> swapChainImages) {
        this->m_swapChainImages = swapChainImages;
    }

    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice m_logical_device = nullptr;

    // VkInstance m_vulkanInstance = 0;
    VulkanContext* m_vulkanContext = 0;
    ERenderingMode m_renderingMode = ERenderingMode::Standard;

    // TODO: We want to create structs for data based on lifetime
    VkSwapchainKHR m_swapChain = nullptr;
    std::vector<VkImage> m_swapChainImages = {};
    VkFormat m_swapChainImageFormat = VK_FORMAT_UNDEFINED;
    VkExtent2D m_swapChainExtent;

    std::unique_ptr<backend_vk::render_pass::RenderPassData>
        m_render_pass_data = {};

    VkImage m_shadowPassDepthImage = 0;
    VkDeviceMemory m_shadowPassDepthImageMemory = 0;
    VkImageView m_shadowPassDepthImageView = 0;
    std::vector<VkImageView> m_swapChainImageViews = {};
    VkSampler m_shadowPassTextureSampler;

    // VkRenderPass m_renderPass;
    VkRenderPass m_shadowPass;
    ShadowPassPushConstants m_shadowPassPushConstants;
    PBRPushConstants m_pbrPushConstants;
    VertexShaderPushConstants m_vertexShaderPushConstants;
    FragShaderPushConstantsBindless m_fragShaderPushConstantsBindless;

    glm::vec4 m_shadowBias;
    VkSurfaceKHR m_surface;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkQueue m_computeQueue;

    VkDescriptorSetLayout m_meshDescriptorSetLayout;
    VkDescriptorSetLayout m_lightDescriptorSetLayout;

    VkDescriptorSetLayout m_sceneDescriptorSetLayout;
    std::vector<VkDescriptorSet> m_sceneDescriptorSets;
    VkDescriptorSetLayout m_texturedMeshDescriptorSetLayout;
    VkDescriptorSetLayout m_texturedPBRDescriptorSetLayout;
    // === BINDLESS === //
    VkDescriptorPool m_descriptor_pool_bindless;
    VkDescriptorPool m_descriptor_pool_bindless_scene;
    VkDescriptorSetLayout m_bindlessDescriptorSetLayout;
    VkDescriptorSetLayout m_bindlessFrameBindingDescriptorSetLayout;
    VkDescriptorPool m_descriptor_pool_per_mesh_bindless;
    // Per mesh model matrix
    VkDescriptorSetLayout m_texturedPBRDescriptorSetLayoutBindless;
    VkDescriptorSet m_global_bindless_descriptor_set;

    std::vector<VkDescriptorSetLayout> m_texturedMeshDescriptorSetLayouts;
    std::vector<VkDescriptorSetLayout> m_shadowPassDescriptorSetLayouts;
    VkDescriptorPool m_descriptor_pool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    std::vector<VkDescriptorSet> m_lightDescriptorSets;

    std::vector<VkDescriptorSet> m_shadowPassDescriptorSets;
    VkDescriptorSetLayout m_shadowPassDescriptorSetLayout;

    VkPipelineLayout m_pbrPipelineLayout;
    VkPipelineLayout m_shadowPassPipelineLayout;

    VkPipeline m_pbrPipeline;
    VkPipeline m_shadowPassPipeline;

    std::unordered_map<BeDrawType, VkPipeline*> m_graphicsPipelines = {};
    std::unordered_map<BeDrawType, std::vector<VkDescriptorSetLayout*>>
        m_descriptorSetLayouts = {};

    DescriptorSetLayoutInfo m_dsLayoutInfoScene;
    DescriptorSetLayoutInfo m_dsLayoutInfoTexturedMesh;
    DescriptorSetLayoutInfo m_dsLayoutInfoUntexturedMesh;
    DescriptorSetLayoutInfo m_dsLayoutInfoHeightMap;
    std::vector<DescriptorSetLayoutInfo> m_dsLayoutInfos = {
        m_dsLayoutInfoScene, m_dsLayoutInfoTexturedMesh,
        m_dsLayoutInfoUntexturedMesh, m_dsLayoutInfoHeightMap};

    std::vector<VkFramebuffer> m_renderPassFrameBuffers;
    std::vector<VkFramebuffer> m_shadowPassFrameBuffers;
    VkCommandPool m_commandPool;
    // === COMPUTE === //
    VkCommandPool m_commandPoolCompute;
    std::vector<VkCommandBuffer> m_commandBuffersCompute;
    VkBuffer m_computeInputBuffer;
    VkDeviceMemory m_computeInputBufferMemory = {};
    void* m_computeInputBufferMemoryMapped = {};
    VkBuffer m_computeOutputBuffer;
    VkDeviceMemory m_computeOutputBufferMemory = {};
    void* m_computeOutputBufferMemoryMapped = {};

    // === UBOS === //
    std::vector<VkBuffer> m_cameraUBO = {};
    std::vector<VkDeviceMemory> m_cameraUBOMemory = {};
    std::vector<void*> m_cameraUBOMapped = {};

    std::vector<VkBuffer> m_lightUBO = {};
    std::vector<VkDeviceMemory> m_lightUBOMemory = {};
    std::vector<void*> m_lightUBOMapped = {};

    LightUBO m_lights = {};

    std::vector<VkBuffer> m_uniformBuffers = {};
    std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
    std::vector<void*> m_uniformBuffersMapped = {};

    // Frames in flight require their own command buffers, semaphores and fences
    std::vector<VkCommandBuffer> m_commandBuffers;
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    VkFence m_computeFence;
    std::vector<Mesh3D> m_meshList = {};
    uint32_t m_currentFrame = 0;

    // Window window;
    uint32_t WIDTH = 800;
    uint32_t HEIGHT = 600;
    int m_shadowMapWidth = 4096;
    int m_shadowMapHeight = 4096;
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debug_messenger;
    int m_maxFramesInFlight = 2;
    int m_maxMeshes = 10;
    int m_frameBufferHeight = 0;
    int m_frameBufferWidth = 0;

   private:
};
#endif
