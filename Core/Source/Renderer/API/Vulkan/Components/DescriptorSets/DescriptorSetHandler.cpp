#include "DescriptorSetHandler.h"

#include <vulkan/vulkan_core.h>

#include <iostream>
#include <stdexcept>

#include "Source/Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.h"

void DescriptorSetHandler::CreateDescriptorPool() {
    // Get number of descriptor sets
    // Create array poolSizes = # of descriptor types
    // Need list of descriptor types used across all sets
    // Need number of each type used
    // Need number of frames in flight
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount =
        (2 * m_vulkanGlobalState->m_maxFramesInFlight) +
        (2 * m_vulkanGlobalState->m_maxMeshes);

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount =
        (2 * m_vulkanGlobalState->m_maxFramesInFlight) +
        (2 * m_vulkanGlobalState->m_maxMeshes);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = m_vulkanGlobalState->m_maxFramesInFlight *
                       m_vulkanGlobalState->m_maxMeshes;

    if (vkCreateDescriptorPool(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &poolInfo, nullptr,
            &m_vulkanGlobalState->m_descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorSetHandler::CreateDescriptorSetLayout(
    DsLayoutInfo* dsLayout, std::vector<DSBindingInfo> bindingInfoList) {
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
            *m_vulkanGlobalState->GetRefLogicalDevice(), &layoutInfo, nullptr,
            &m_vulkanGlobalState->m_meshDescriptorSetLayout) != VK_SUCCESS) {
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
            *m_vulkanGlobalState->GetRefLogicalDevice(), &layoutInfo, nullptr,
            &m_vulkanGlobalState->m_sceneDescriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
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
            *m_vulkanGlobalState->GetRefLogicalDevice(), &layoutInfo, nullptr,
            &m_vulkanGlobalState->m_shadowPassDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetHandler::CreateSceneDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkanGlobalState->m_maxFramesInFlight,
        m_vulkanGlobalState->m_sceneDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkanGlobalState->m_descriptorPool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkanGlobalState->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    m_vulkanGlobalState->m_sceneDescriptorSets.resize(
        m_vulkanGlobalState->m_maxFramesInFlight);
    if (vkAllocateDescriptorSets(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &allocInfo,
            m_vulkanGlobalState->m_sceneDescriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkanGlobalState->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo cameraUBOInfo{};
        cameraUBOInfo.buffer = m_vulkanGlobalState->m_cameraUBO[i];
        cameraUBOInfo.offset = 0;
        cameraUBOInfo.range = VK_WHOLE_SIZE;

        VkDescriptorBufferInfo lightUBOInfo{};
        lightUBOInfo.buffer = m_vulkanGlobalState->m_lightUBO[i];
        lightUBOInfo.offset = 0;
        lightUBOInfo.range = VK_WHOLE_SIZE;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            m_vulkanGlobalState->m_sceneDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &cameraUBOInfo;

        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet =
            m_vulkanGlobalState->m_sceneDescriptorSets[i];
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pBufferInfo = &lightUBOInfo;

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(*m_vulkanGlobalState->GetRefLogicalDevice(),
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
            *m_vulkanGlobalState->GetRefLogicalDevice(), &layoutInfo, nullptr,
            &m_vulkanGlobalState->m_texturedMeshDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

void DescriptorSetHandler::CreateTexturedMeshDescriptorSets(Mesh3D* mesh) {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkanGlobalState->m_maxFramesInFlight,
        m_vulkanGlobalState->m_texturedMeshDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkanGlobalState->m_descriptorPool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkanGlobalState->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    mesh->m_descriptorSets.resize(m_vulkanGlobalState->m_maxFramesInFlight);
    if (vkAllocateDescriptorSets(*m_vulkanGlobalState->GetRefLogicalDevice(),
                                 &allocInfo,
                                 mesh->m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkanGlobalState->m_maxFramesInFlight; i++) {
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
        vkUpdateDescriptorSets(*m_vulkanGlobalState->GetRefLogicalDevice(),
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
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
            *m_vulkanGlobalState->GetRefLogicalDevice(), &layoutInfo, nullptr,
            &m_vulkanGlobalState->m_texturedPBRDescriptorSetLayout) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}
void DescriptorSetHandler::CreateTexturedPBRDescriptorSets(Mesh3D* mesh) {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkanGlobalState->m_maxFramesInFlight,
        m_vulkanGlobalState->m_texturedPBRDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkanGlobalState->m_descriptorPool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkanGlobalState->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    mesh->m_descriptorSets.resize(m_vulkanGlobalState->m_maxFramesInFlight);
    if (vkAllocateDescriptorSets(*m_vulkanGlobalState->GetRefLogicalDevice(),
                                 &allocInfo,
                                 mesh->m_descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkanGlobalState->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = mesh->m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ModelUBO);

        VkDescriptorImageInfo albedoImageInfo{};
        albedoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        albedoImageInfo.imageView =
            mesh->m_materials[ALBEDO].m_textureImageView;
        albedoImageInfo.sampler = mesh->m_materials[ALBEDO].m_textureSampler;

        VkDescriptorImageInfo metallicImageInfo{};
        metallicImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        metallicImageInfo.imageView =
            mesh->m_materials[METALLIC].m_textureImageView;
        metallicImageInfo.sampler =
            mesh->m_materials[METALLIC].m_textureSampler;

        VkDescriptorImageInfo roughnessImageInfo{};
        roughnessImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        roughnessImageInfo.imageView =
            mesh->m_materials[ROUGHNESS].m_textureImageView;
        roughnessImageInfo.sampler =
            mesh->m_materials[ROUGHNESS].m_textureSampler;

        VkDescriptorImageInfo aoImageInfo{};
        aoImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        aoImageInfo.imageView = mesh->m_materials[AO].m_textureImageView;
        aoImageInfo.sampler = mesh->m_materials[AO].m_textureSampler;

        VkDescriptorImageInfo normalImageInfo{};
        normalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        normalImageInfo.imageView =
            mesh->m_materials[NORMAL].m_textureImageView;
        normalImageInfo.sampler = mesh->m_materials[NORMAL].m_textureSampler;

        VkDescriptorImageInfo emissiveImageInfo{};
        emissiveImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        emissiveImageInfo.imageView =
            mesh->m_materials[EMISSIVE].m_textureImageView;
        emissiveImageInfo.sampler =
            mesh->m_materials[EMISSIVE].m_textureSampler;

        VkDescriptorImageInfo shadowImageInfo{};
        shadowImageInfo.imageLayout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        shadowImageInfo.imageView =
            m_vulkanGlobalState->m_shadowPassDepthImageView;
        shadowImageInfo.sampler =
            m_vulkanGlobalState->m_shadowPassTextureSampler;

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
        vkUpdateDescriptorSets(*m_vulkanGlobalState->GetRefLogicalDevice(),
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
    }
}

void DescriptorSetHandler::CreateShadowPassDescriptorSet() {
    std::vector<VkDescriptorSetLayout> layouts(
        m_vulkanGlobalState->m_maxFramesInFlight,
        m_vulkanGlobalState->m_shadowPassDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_vulkanGlobalState->m_descriptorPool;
    allocInfo.descriptorSetCount =
        static_cast<uint32_t>(m_vulkanGlobalState->m_maxFramesInFlight);
    allocInfo.pSetLayouts = layouts.data();

    m_vulkanGlobalState->m_shadowPassDescriptorSets.resize(
        m_vulkanGlobalState->m_maxFramesInFlight);

    if (vkAllocateDescriptorSets(
            *m_vulkanGlobalState->GetRefLogicalDevice(), &allocInfo,
            m_vulkanGlobalState->m_shadowPassDescriptorSets.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < m_vulkanGlobalState->m_maxFramesInFlight; i++) {
        VkDescriptorBufferInfo lightUBOInfo{};
        lightUBOInfo.buffer = m_vulkanGlobalState->m_lightUBO[i];
        lightUBOInfo.offset = 0;
        lightUBOInfo.range = VK_WHOLE_SIZE;

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet =
            m_vulkanGlobalState->m_shadowPassDescriptorSets[i];
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &lightUBOInfo;

        uint32_t descriptorCopyCount = 0;
        vkUpdateDescriptorSets(*m_vulkanGlobalState->GetRefLogicalDevice(),
                               (uint32_t)descriptorWrites.size(),
                               descriptorWrites.data(), descriptorCopyCount,
                               nullptr);
    }
}
