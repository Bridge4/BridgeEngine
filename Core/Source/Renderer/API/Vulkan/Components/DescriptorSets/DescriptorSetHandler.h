#ifndef DESCRIPTORSETHANDLER_H
#define DESCRIPTORSETHANDLER_H
#include <vulkan/vulkan.h>

#include <vector>

#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/Mesh/Mesh3D.h"

class DescriptorSetHandler {
   public:
    DescriptorSetHandler(VulkanGlobalState* VulkanGlobalState,
                         DeviceHandler* deviceHandler) {
        this->m_vulkanGlobalState = VulkanGlobalState;
        this->m_deviceHandler = deviceHandler;
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
    VkDescriptorPool CreateDescriptorPoolCompute(VkDevice& logicalDevice);
    VkDescriptorSetLayout CreateComputeDescriptorSetLayout(
        VkDevice& logicalDevice);
    VkDescriptorSet CreateDescriptorSetCompute(VkDescriptorPool& descriptorPool,
                                               VkDescriptorSetLayout& layout,
                                               VkDevice& logicalDevice);

   private:
    VulkanGlobalState* m_vulkanGlobalState = nullptr;
    DeviceHandler* m_deviceHandler = nullptr;
};

#endif
