#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <unordered_map>
#include <vector>

enum DescriptorSetLayoutType {
    SCENE,
    TEXTURED_MESH,
    UNTEXTURED_MESH,
    HEIGHT_MAP
};

struct DsLayoutInfo {
    DescriptorSetLayoutType Type;
    std::vector<VkDescriptorSetLayoutBinding> Bindings;
    std::unordered_map<VkDescriptorType, uint32_t> BindingTypeCount;
    uint32_t BindingCount = Bindings.size();
};

struct DSBindingInfo {
    uint32_t BindingCount;
    VkDescriptorType DescriptorType;
    VkShaderStageFlags ShaderStages;
    uint32_t DescriptorCount;
};
