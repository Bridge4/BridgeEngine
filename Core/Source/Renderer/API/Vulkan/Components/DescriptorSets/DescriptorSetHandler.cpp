#include "DescriptorSetHandler.h"

#include <vulkan/vulkan_core.h>

#include <iostream>
#include <stdexcept>

#include "Source/Renderer/API/Vulkan/Components/Devices/DeviceHandler.h"
#include "Source/Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.h"
#include "Source/Renderer/RendererTypes.h"

namespace backend_vk::descriptor_sets {

std::optional<VkDescriptorPool> create_descriptor_pool(
    VkDevice& logical_device, DescriptorPoolData dsp_data) {
    if (dsp_data.descriptor_types.empty()) {
        std::cout
            << "descriptor types not provided cannot create descriptor pool\n";
        return nullptr;
    }
    if (dsp_data.max_sets == 0) {
        std::cout << "max_sets set to 0 cannot create descriptor pool";
        return nullptr;
    }

    VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

    std::vector<VkDescriptorPoolSize> dsp_sizes = {};

    for (auto i : dsp_data.descriptor_types) {
        VkDescriptorPoolSize dsp_size;
        dsp_size.type = i.first;
        dsp_size.descriptorCount = i.second;
        dsp_sizes.push_back(dsp_size);
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    if (dsp_data.flags) poolInfo.flags = dsp_data.flags;
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(dsp_sizes.size());
    poolInfo.pPoolSizes = dsp_sizes.data();
    poolInfo.maxSets = dsp_data.max_sets;

    if (vkCreateDescriptorPool(logical_device, &poolInfo, nullptr,
                               &descriptor_pool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    return descriptor_pool;
}

// NOTE: Our bindings will be determined by our shaders so this function expects
// that valid bindings have been generated outside and then passed in
std::optional<VkDescriptorSetLayout> create_descriptor_set_layout(
    VkDevice& logical_device, DescriptorSetLayoutData dsl_data) {
    // lol
    VkDescriptorSetLayout dsl;

    bool has_mutable_binding = false;
    uint32_t i = 0;
    for (auto binding : dsl_data.bindings) {
        if (binding.descriptorType == VK_DESCRIPTOR_TYPE_MUTABLE_EXT) {
            has_mutable_binding = true;
            if (dsl_data.p_next.type !=
                DescriptorSetLayoutExtensionType::MUTABLE_DESCRIPTOR) {
                std::cout << "vulkan spec states that if any binding is a "
                             "mutable descriptor the pNext extension structure "
                             "must be VK_DESCRIPTOR_TYPE_MUTABLE_EXT\n";
                return nullptr;
            }
            if (dsl_data.p_next.data.mutable_descriptor_type_create_info
                    .mutableDescriptorTypeListCount <= i) {
                std::cout
                    << "Vulkan spec error: If any element pBindings[i] has a "
                       "descriptorType of "
                       "VK_DESCRIPTOR_TYPE_MUTABLE_EXT, then the pNext chain "
                       "must include a VkMutableDescriptorTypeCreateInfoEXT "
                       "structure, and mutableDescriptorTypeListCount must be "
                       "greater than i \n";
            }
        }
        break;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(dsl_data.bindings.size());
    switch (dsl_data.p_next.type) {
        case backend_vk::descriptor_sets::DescriptorSetLayoutExtensionType::
            BINDING_FLAGS:
            layoutInfo.pNext = &dsl_data.p_next.data.bindings_flags_create_info;
            break;
        case backend_vk::descriptor_sets::DescriptorSetLayoutExtensionType::
            MUTABLE_DESCRIPTOR:
            layoutInfo.pNext =
                &dsl_data.p_next.data.mutable_descriptor_type_create_info;
            break;
        default:
            break;
    }
    layoutInfo.flags = dsl_data.flags;
    layoutInfo.pBindings = dsl_data.bindings.data();

    if (vkCreateDescriptorSetLayout(logical_device, &layoutInfo, nullptr,
                                    &dsl) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }

    return dsl;
    // NOTE: Vulkan Spec:
    //  If any element pBindings[i] has a descriptorType of
    //  VK_DESCRIPTOR_TYPE_MUTABLE_EXT, then the pNext chain must include a
    //  VkMutableDescriptorTypeCreateInfoEXT structure, and
    //  mutableDescriptorTypeListCount must be greater than i
}

std::optional<std::vector<VkDescriptorSet>> allocate_descriptor_sets(
    VkDevice& logical_device, VkDescriptorSetLayout dsl, VkDescriptorPool& dsp,
    uint32_t descriptor_count) {
    std::vector<VkDescriptorSet> descriptor_sets = {};

    std::vector<VkDescriptorSetLayout> layouts(descriptor_count, dsl);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = dsp;
    allocInfo.descriptorSetCount = descriptor_count;

    allocInfo.pSetLayouts = layouts.data();

    descriptor_sets.resize(descriptor_count);
    if (vkAllocateDescriptorSets(logical_device, &allocInfo,
                                 descriptor_sets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    return descriptor_sets;
}

}  // namespace backend_vk::descriptor_sets

void DescriptorSetHandler::CreateDescriptorSetLayout(
    DescriptorSetLayoutInfo* dsLayout,
    std::vector<DescriptorSetBindingInfo> bindingInfoList) {
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};
    uint32_t bindingIndex = 0;
    for (auto& bindingInfo : bindingInfoList) {
        VkDescriptorSetLayoutBinding dslBinding{};
        dslBinding.binding = bindingIndex;
        dslBinding.descriptorType = bindingInfo.DescriptorType;
        dslBinding.stageFlags = bindingInfo.ShaderStages;
        dslBinding.descriptorCount = bindingInfo.DescriptorCount;
        dsLayout->Bindings.push_back(dslBinding);

        auto descriptorTypeExists =
            dsLayout->BindingTypeCount.count(dslBinding.descriptorType);
        if (descriptorTypeExists) {
            dsLayout->BindingTypeCount[dslBinding.descriptorType] += 1;
        } else {
            dsLayout->BindingTypeCount[dslBinding.descriptorType] = 1;
        }
        bindingIndex++;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings = layoutBindings.data();

    if (vkCreateDescriptorSetLayout(
            m_vulkan_global_state->m_logical_device, &layoutInfo, nullptr,
            &m_vulkan_global_state->m_meshDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetHandler::CreateSceneDescriptorSetLayout() {
    // Binding 0 = CameraUBO
    VkDescriptorSetLayoutBinding cameraUBO{};
    cameraUBO.binding = 0;
    cameraUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    cameraUBO.descriptorCount = 1;
    cameraUBO.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    cameraUBO.pImmutableSamplers = nullptr;  // Optional
    // Binding 1 = LightUBO
    VkDescriptorSetLayoutBinding lightUBO{};
    lightUBO.binding = 1;
    lightUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightUBO.descriptorCount = 1;
    lightUBO.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    lightUBO.pImmutableSamplers = nullptr;  // Optional

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {cameraUBO,
                                                            lightUBO};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(
            m_vulkan_global_state->m_logical_device, &layoutInfo, nullptr,
            &m_vulkan_global_state->m_sceneDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

VkDescriptorPool DescriptorSetHandler::CreateDescriptorPoolCompute(
    VkDevice& logical_device) {
    VkDescriptorPool descriptorPool;
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[0].descriptorCount = 2;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = 1;
    poolInfo.poolSizeCount = 1;

    if (vkCreateDescriptorPool(logical_device, &poolInfo, nullptr,
                               &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
    return descriptorPool;
}

VkDescriptorSetLayout DescriptorSetHandler::CreateComputeDescriptorSetLayout(
    VkDevice& logical_device) {
    VkDescriptorSetLayout layout;
    // Binding 0 = InputBuffer
    VkDescriptorSetLayoutBinding inputBuffer{};
    inputBuffer.binding = 0;
    inputBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    inputBuffer.descriptorCount = 1;
    inputBuffer.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
    // Binding 1 = OutputBuffer
    VkDescriptorSetLayoutBinding outputBuffer{};
    outputBuffer.binding = 1;
    outputBuffer.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    outputBuffer.descriptorCount = 1;
    outputBuffer.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {inputBuffer,
                                                            outputBuffer};

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(logical_device, &layoutInfo, nullptr,
                                    &layout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
    return layout;
}

VkDescriptorSet DescriptorSetHandler::CreateDescriptorSetCompute(
    VkDescriptorPool& descriptorPool, VkDescriptorSetLayout& layout,
    VkDevice& logical_device) {
    VkDescriptorSet descriptorSet;

    std::vector<VkDescriptorSetLayout> layouts(1, layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = layouts.data();

    if (vkAllocateDescriptorSets(logical_device, &allocInfo, &descriptorSet) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    VkDescriptorBufferInfo inInfo{};
    inInfo.buffer = m_vulkan_global_state->m_computeInputBuffer;
    inInfo.offset = 0;
    inInfo.range = VK_WHOLE_SIZE;

    VkDescriptorBufferInfo outInfo{};
    outInfo.buffer = m_vulkan_global_state->m_computeOutputBuffer;
    outInfo.offset = 0;
    outInfo.range = VK_WHOLE_SIZE;

    std::array<VkWriteDescriptorSet, 2> writes{};

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = descriptorSet;
    writes[0].dstBinding = 0;
    writes[0].dstArrayElement = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[0].pBufferInfo = &inInfo;

    writes[1] = writes[0];
    writes[1].dstBinding = 1;
    writes[1].pBufferInfo = &outInfo;

    vkUpdateDescriptorSets(logical_device, static_cast<uint32_t>(writes.size()),
                           writes.data(), 0, nullptr);

    return descriptorSet;
}

void DescriptorSetHandler::CreateShadowPassDescriptorSetLayout() {
    // Binding 0 = LightUBO
    VkDescriptorSetLayoutBinding lightUBO{};
    lightUBO.binding = 0;
    lightUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    lightUBO.descriptorCount = 1;
    lightUBO.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    lightUBO.pImmutableSamplers = nullptr;  // Optional

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {lightUBO};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(
            m_vulkan_global_state->m_logical_device, &layoutInfo, nullptr,
            &m_vulkan_global_state->m_shadowPassDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

// descriptor_set_layout
// descriptor_count
void DescriptorSetHandler::CreateSceneDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkan_global_state->m_maxFramesInFlight,
        m_vulkan_global_state->m_sceneDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    if (m_vulkan_global_state->m_renderingMode == ERenderingMode::Bindless) {
        allocInfo.descriptorPool =
            m_vulkan_global_state->m_descriptor_pool_bindless_scene;
    } else {
        allocInfo.descriptorPool = m_vulkan_global_state->m_descriptor_pool;
    }
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkan_global_state->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    m_vulkan_global_state->m_sceneDescriptorSets.resize(
        m_vulkan_global_state->m_maxFramesInFlight);
    if (vkAllocateDescriptorSets(
            m_vulkan_global_state->m_logical_device, &allocInfo,
            m_vulkan_global_state->m_sceneDescriptorSets.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo cameraUBOInfo{};
        cameraUBOInfo.buffer = m_vulkan_global_state->m_cameraUBO[i];
        cameraUBOInfo.offset = 0;
        cameraUBOInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo lightUBOInfo{};
        lightUBOInfo.buffer = m_vulkan_global_state->m_lightUBO[i];
        lightUBOInfo.offset = 0;
        lightUBOInfo.range = VK_WHOLE_SIZE;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            m_vulkan_global_state->m_sceneDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &cameraUBOInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet =
            m_vulkan_global_state->m_sceneDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightUBOInfo;

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(m_vulkan_global_state->m_logical_device,
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
    }
}

void DescriptorSetHandler::CreateTexturedMeshDescriptorSetLayout() {
    // Binding 0 = ModelUBO
    VkDescriptorSetLayoutBinding modelUBO{};
    modelUBO.binding = 0;
    modelUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelUBO.descriptorCount = 1;
    modelUBO.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    modelUBO.pImmutableSamplers = nullptr;  // Optional
    // Binding 1 = Sampler2D
    VkDescriptorSetLayoutBinding textureSampler{};
    textureSampler.binding = 1;
    textureSampler.descriptorCount = 1;
    textureSampler.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureSampler.pImmutableSamplers = nullptr;
    textureSampler.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 2> bindings = {modelUBO,
                                                            textureSampler};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(
            m_vulkan_global_state->m_logical_device, &layoutInfo, nullptr,
            &m_vulkan_global_state->m_texturedMeshDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetHandler::CreateTexturedMeshDescriptorSets(Mesh3D* mesh) {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkan_global_state->m_maxFramesInFlight,
        m_vulkan_global_state->m_texturedMeshDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkan_global_state->m_descriptor_pool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkan_global_state->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    mesh->m_descriptorSets.resize(m_vulkan_global_state->m_maxFramesInFlight);
    if (vkAllocateDescriptorSets(m_vulkan_global_state->m_logical_device,
                                 &allocInfo,
                                 mesh->m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mesh->m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ModelUBO);

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = mesh->m_textureImageView;
        imageInfo.sampler = mesh->m_textureSampler;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mesh->m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = mesh->m_descriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(m_vulkan_global_state->m_logical_device,
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
    }
}

void DescriptorSetHandler::CreateTexturedPBRDescriptorSetLayoutBindless() {
    VkDescriptorSetLayoutBinding modelUBO{};
    modelUBO.binding = 0;
    modelUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelUBO.descriptorCount = 1;
    modelUBO.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    modelUBO.pImmutableSamplers = nullptr;  // Optional

    std::array<VkDescriptorSetLayoutBinding, 1> bindings = {modelUBO};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(
            m_vulkan_global_state->m_logical_device, &layoutInfo, nullptr,
            &m_vulkan_global_state->m_texturedPBRDescriptorSetLayoutBindless) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}
void DescriptorSetHandler::CreateTexturedPBRDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding modelUBO{};
    modelUBO.binding = 0;
    modelUBO.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    modelUBO.descriptorCount = 1;
    modelUBO.stageFlags =
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    modelUBO.pImmutableSamplers = nullptr;  // Optional

    VkDescriptorSetLayoutBinding albedoMap{};
    albedoMap.binding = 1;
    albedoMap.descriptorCount = 1;
    albedoMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    albedoMap.pImmutableSamplers = nullptr;
    albedoMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding metallicMap{};
    metallicMap.binding = 2;
    metallicMap.descriptorCount = 1;
    metallicMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    metallicMap.pImmutableSamplers = nullptr;
    metallicMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding roughnessMap{};
    roughnessMap.binding = 3;
    roughnessMap.descriptorCount = 1;
    roughnessMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    roughnessMap.pImmutableSamplers = nullptr;
    roughnessMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding aoMap{};
    aoMap.binding = 4;
    aoMap.descriptorCount = 1;
    aoMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    aoMap.pImmutableSamplers = nullptr;
    aoMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding normalMap{};
    normalMap.binding = 5;
    normalMap.descriptorCount = 1;
    normalMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalMap.pImmutableSamplers = nullptr;
    normalMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding emissiveMap{};
    emissiveMap.binding = 6;
    emissiveMap.descriptorCount = 1;
    emissiveMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    emissiveMap.pImmutableSamplers = nullptr;
    emissiveMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutBinding shadowMap{};
    shadowMap.binding = 7;
    shadowMap.descriptorCount = 1;
    shadowMap.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    shadowMap.pImmutableSamplers = nullptr;
    shadowMap.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    std::array<VkDescriptorSetLayoutBinding, 8> bindings = {
        modelUBO, albedoMap, metallicMap, roughnessMap,
        aoMap,    normalMap, emissiveMap, shadowMap};
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(
            m_vulkan_global_state->m_logical_device, &layoutInfo, nullptr,
            &m_vulkan_global_state->m_texturedPBRDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetHandler::CreateTexturedPBRDescriptorSetsBindless(
    Mesh3D* mesh) {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkan_global_state->m_maxFramesInFlight,
        m_vulkan_global_state->m_texturedPBRDescriptorSetLayoutBindless);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool =
        m_vulkan_global_state->m_descriptor_pool_per_mesh_bindless;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkan_global_state->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    mesh->m_descriptorSets.resize(m_vulkan_global_state->m_maxFramesInFlight);
    if (vkAllocateDescriptorSets(m_vulkan_global_state->m_logical_device,
                                 &allocInfo,
                                 mesh->m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mesh->m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ModelUBO);

        std::array<VkWriteDescriptorSet, 1> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mesh->m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        int j = 1;

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(m_vulkan_global_state->m_logical_device,
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
    }
}
void DescriptorSetHandler::CreateTexturedPBRDescriptorSets(Mesh3D* mesh) {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkan_global_state->m_maxFramesInFlight,
        m_vulkan_global_state->m_texturedPBRDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkan_global_state->m_descriptor_pool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkan_global_state->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    mesh->m_descriptorSets.resize(m_vulkan_global_state->m_maxFramesInFlight);
    if (vkAllocateDescriptorSets(m_vulkan_global_state->m_logical_device,
                                 &allocInfo,
                                 mesh->m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mesh->m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ModelUBO);

        VkDescriptorImageInfo albedoImageInfo{};
        albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoImageInfo.imageView =
            mesh->m_materialDataMap[ALBEDO].m_textureImageView;
        albedoImageInfo.sampler =
            mesh->m_materialDataMap[ALBEDO].m_textureSampler;

        VkDescriptorImageInfo metallicImageInfo{};
        metallicImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        metallicImageInfo.imageView =
            mesh->m_materialDataMap[METALLIC].m_textureImageView;
        metallicImageInfo.sampler =
            mesh->m_materialDataMap[METALLIC].m_textureSampler;

        VkDescriptorImageInfo roughnessImageInfo{};
        roughnessImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        roughnessImageInfo.imageView =
            mesh->m_materialDataMap[ROUGHNESS].m_textureImageView;
        roughnessImageInfo.sampler =
            mesh->m_materialDataMap[ROUGHNESS].m_textureSampler;

        VkDescriptorImageInfo aoImageInfo{};
        aoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        aoImageInfo.imageView = mesh->m_materialDataMap[AO].m_textureImageView;
        aoImageInfo.sampler = mesh->m_materialDataMap[AO].m_textureSampler;

        VkDescriptorImageInfo normalImageInfo{};
        normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageInfo.imageView =
            mesh->m_materialDataMap[NORMAL].m_textureImageView;
        normalImageInfo.sampler =
            mesh->m_materialDataMap[NORMAL].m_textureSampler;

        VkDescriptorImageInfo emissiveImageInfo{};
        emissiveImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        emissiveImageInfo.imageView =
            mesh->m_materialDataMap[EMISSIVE].m_textureImageView;
        emissiveImageInfo.sampler =
            mesh->m_materialDataMap[EMISSIVE].m_textureSampler;

        VkDescriptorImageInfo shadowImageInfo{};
        shadowImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        shadowImageInfo.imageView =
            m_vulkan_global_state->m_shadowPassDepthImageView;
        shadowImageInfo.sampler =
            m_vulkan_global_state->m_shadowPassTextureSampler;

        std::vector<VkDescriptorImageInfo> imageInfos = {
            albedoImageInfo, metallicImageInfo, roughnessImageInfo, aoImageInfo,
            normalImageInfo, emissiveImageInfo, shadowImageInfo};
        std::array<VkWriteDescriptorSet, 8> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = mesh->m_descriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;

        int j = 1;
        for (auto& imageInfo : imageInfos) {
            descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstSet = mesh->m_descriptorSets[i];
            descriptorWrites[j].dstBinding = j;
            descriptorWrites[j].dstArrayElement = 0;
            descriptorWrites[j].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j].descriptorCount = 1;
            descriptorWrites[j].pImageInfo = &imageInfo;
            j++;
        }

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(m_vulkan_global_state->m_logical_device,
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
    }
}

void DescriptorSetHandler::CreateShadowPassDescriptorSet() {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkan_global_state->m_maxFramesInFlight,
        m_vulkan_global_state->m_shadowPassDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    if (m_vulkan_global_state->m_renderingMode == ERenderingMode::Bindless) {
        allocInfo.descriptorPool =
            m_vulkan_global_state->m_descriptor_pool_bindless_scene;
    } else {
        allocInfo.descriptorPool = m_vulkan_global_state->m_descriptor_pool;
    }
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkan_global_state->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    m_vulkan_global_state->m_shadowPassDescriptorSets.resize(
        m_vulkan_global_state->m_maxFramesInFlight);

    if (vkAllocateDescriptorSets(
            m_vulkan_global_state->m_logical_device, &allocInfo,
            m_vulkan_global_state->m_shadowPassDescriptorSets.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo lightUBOInfo{};
        lightUBOInfo.buffer = m_vulkan_global_state->m_lightUBO[i];
        lightUBOInfo.offset = 0;
        lightUBOInfo.range = VK_WHOLE_SIZE;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            m_vulkan_global_state->m_shadowPassDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &lightUBOInfo;

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(m_vulkan_global_state->m_logical_device,
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
    }
}
