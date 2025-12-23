#ifndef DESCRIPTORSETHANDLER_H
#define DESCRIPTORSETHANDLER_H
#include <vulkan/vulkan.h>

#include <vector>

#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/Mesh/Mesh3D.h"

class DescriptorSetHandler {
   public:
    DescriptorSetHandler(VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkanGlobalState = VulkanGlobalState;
    }

    void CreateDescriptorPool();
    void CreateDescriptorSetLayout(DsLayoutInfo* dsLayout,
                                   std::vector<DSBindingInfo> bindings);
    void CreateSceneDescriptorSetLayout();
    void CreateSceneDescriptorSets();
    void CreateShadowPassDescriptorSetLayout();
    void CreateShadowPassDescriptorSet();
    void CreateTexturedMeshDescriptorSetLayout();
    void CreateTexturedMeshDescriptorSets(Mesh3D* mesh);

   private:
    VulkanGlobalState* m_vulkanGlobalState = nullptr;
};

#endif
