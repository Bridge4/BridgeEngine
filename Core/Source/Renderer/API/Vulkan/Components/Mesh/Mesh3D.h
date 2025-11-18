#ifndef MESH3D_H
#define MESH3D_H

#include <string>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.h>

#include <../glm/glm.hpp>
#include <../glm/gtc/matrix_transform.hpp>
#include <../glm/gtx/hash.hpp>

class Mesh3D {
public:
    Mesh3D(){

    }

    // Vulkan Texture Properties
    VkImage image = VK_NULL_HANDLE;
    VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
    VkImageView imageView = VK_NULL_HANDLE;
    VkSampler sampler = VK_NULL_HANDLE;

    // Vulkan Object Resources
    std::vector<VkBuffer> m_uniformBuffers = {};
    std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
	std::vector<void*> m_uniformBuffersMapped = {};

    std::vector<VkDescriptorSet> descriptorSets = {};

    // Transform
    glm::vec3 position = {0.0f, 0.0f, 0.0f};
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
};
#endif
