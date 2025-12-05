#pragma once
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

enum DescriptorSetLayoutType {
    SCENE,
    TEXTURED_MESH,
    UNTEXTURED_MESH,
    HEIGHT_MAP
};

struct BeDescriptorSetBinding {
    uint32_t Index;
    VkDescriptorType DescriptorType;
    uint32_t DescriptorCount;
    std::vector<VkShaderStageFlagBits> ListOfShaderStages;
    VkShaderStageFlagBits ShaderStages;
};

struct BeDescriptorSetLayout {
    DescriptorSetLayoutType Type;
    std::vector<BeDescriptorSetBinding> Bindings;
    std::unordered_map<VkDescriptorType, uint32_t> BindingTypeCount;
    uint32_t BindingCount;
    // Add Binding
    // 1. Create BeDescriptorSetBinding
    // 2. Push to Bindings
    // 3. Increment BindingTypeCount[Binding.Type]
};

