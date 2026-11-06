#ifndef DESCRIPTORSETHANDLER_H
#define DESCRIPTORSETHANDLER_H
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <cstdint>
#include <optional>
#include <unordered_map>
#include <variant>
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

enum class DescriptorSetLayoutExtensionType {
    NONE,
    BINDING_FLAGS,
    MUTABLE_DESCRIPTOR
};

// Using tagged union instead of std::variant for compile time benefit
struct DescriptorSetLayoutExtension {
    DescriptorSetLayoutExtensionType type =
        DescriptorSetLayoutExtensionType::NONE;

    union Data {
        VkDescriptorSetLayoutBindingFlagsCreateInfo bindings_flags_create_info;
        VkMutableDescriptorTypeCreateInfoEXT
            mutable_descriptor_type_create_info;
    } data;

    void set_null() { type = DescriptorSetLayoutExtensionType::NONE; }
    void set_bindings_flags_create_info(
        VkDescriptorSetLayoutBindingFlagsCreateInfo val) {
        data.bindings_flags_create_info = val;
        type = DescriptorSetLayoutExtensionType::BINDING_FLAGS;
    }

    void set_mutable_descriptor_type_create_info(
        VkMutableDescriptorTypeCreateInfoEXT val) {
        data.mutable_descriptor_type_create_info = val;
        type = DescriptorSetLayoutExtensionType::MUTABLE_DESCRIPTOR;
    }
};

struct DescriptorSetLayoutData {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    VkDescriptorSetLayoutCreateFlags flags;
    // used for pNext
    DescriptorSetLayoutExtension p_next;
};

std::optional<VkDescriptorPool> create_descriptor_pool(
    VkDevice& logical_device, DescriptorPoolData dsp_data);

std::optional<VkDescriptorSetLayout> create_descriptor_set_layout(
    VkDevice& logical_device, DescriptorSetLayoutData dsl_data);

std::optional<std::vector<VkDescriptorSet>> allocate_descriptor_sets(
    VkDevice& logical_device, VkDescriptorSetLayout dsl, VkDescriptorPool& dsp,
    uint32_t descriptor_count);

}  // namespace backend_vk::descriptor_sets

class DescriptorSetHandler {
   public:
    DescriptorSetHandler(VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkan_global_state = VulkanGlobalState;
    }

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
