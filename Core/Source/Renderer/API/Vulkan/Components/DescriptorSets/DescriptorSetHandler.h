#ifndef DESCRIPTORSETHANDLER_H
#define DESCRIPTORSETHANDLER_H
#include <vulkan/vulkan.h>
#include <vector>
#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/Components/Mesh/Mesh3D.h"
#include "Source/Renderer/API/Vulkan/VulkanAbstractions.h"

class DescriptorSetHandler {
public:
    DescriptorSetHandler(VulkanInstanceManager* vulkanInstanceManager) {
        this->m_vulkanGlobalState = vulkanInstanceManager;
    }

    void CreateDescriptorPool();

    void CreateDescriptorSetLayout(BeDescriptorSetLayout dSetLayout);

    void CreateSceneDescriptorSetLayout();

    void CreateSceneDescriptorSets();

    void CreateTexturedMeshDescriptorSetLayout();

    void CreateTexturedMeshDescriptorSets(Mesh3D* mesh);

private:
    VulkanInstanceManager* m_vulkanGlobalState = nullptr;
};

#endif
