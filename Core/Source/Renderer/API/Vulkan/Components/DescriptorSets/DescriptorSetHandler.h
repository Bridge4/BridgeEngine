#ifndef DESCRIPTORSETHANDLER_H
#define DESCRIPTORSETHANDLER_H
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <optional>
#include <unordered_map>
#include <vector>

#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/Mesh/Mesh3D.h"

namespace backend_vk::descriptor_sets {

struct DescriptorPoolData {
    // descriptor_type : descriptor_count
    VkDescriptorPoolCreateFlagBits flags;
    std::unordered_map<VkDescriptorType, uint32_t> descriptor_types;
    uint32_t max_sets;
};

std::optional<VkDescriptorPool> create_descriptor_pool(
    VkDevice& logical_device, DescriptorPoolData dsp_data);

}  // namespace backend_vk::descriptor_sets

class DescriptorSetHandler {
   public:
    DescriptorSetHandler(VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkan_global_state = VulkanGlobalState;
    }

    void CreateDescriptorPool();
    void CreateDescriptorSetLayout(
        DescriptorSetLayoutInfo* dsLayout,
        std::vector<DescriptorSetBindingInfo> bindings);
    void CreateSceneDescriptorSetLayout();
    void CreateSceneDescriptorSets();
    void CreateShadowPassDescriptorSetLayout();
    void CreateShadowPassDescriptorSet();
    void CreateTexturedMeshDescriptorSetLayout();
    void CreateTexturedMeshDescriptorSets(Mesh3D* mesh);
    void CreateTexturedPBRDescriptorSetLayout();
    void CreateTexturedPBRDescriptorSets(Mesh3D* mesh);

    // === BINDLESS === //
    void CreateDescriptorPoolBindless();
    void CreateDescriptorPoolBindlessScene();
    void CreateDescriptorPoolPerMeshBindless();
    void CreateDescriptorSetLayoutBindless();
    void CreateDescriptorSetBindless();
    void CreateTexturedPBRDescriptorSetLayoutBindless();
    void CreateTexturedPBRDescriptorSetsBindless(Mesh3D* mesh);
    void RegisterTexture();
    // === COMPUTE === //
    VkDescriptorPool CreateDescriptorPoolCompute(VkDevice& logical_device);
    VkDescriptorSetLayout CreateComputeDescriptorSetLayout(
        VkDevice& logical_device);
    VkDescriptorSet CreateDescriptorSetCompute(VkDescriptorPool& descriptorPool,
                                               VkDescriptorSetLayout& layout,
                                               VkDevice& logical_device);

   private:
    VulkanGlobalState* m_vulkan_global_state = nullptr;
};

#endif
