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

    // Mesh Properties
    int m_indexBufferStartIndex = 0;
    int m_indexBufferEndIndex = 0;
    int m_vertexBufferStartIndex = 0;
    int m_vertexBufferEndIndex = 0;

    int m_indexCount = 0;
    // Vulkan Texture Properties
    VkImage m_textureImage = VK_NULL_HANDLE;
    VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
    VkImageView m_textureImageView = VK_NULL_HANDLE;
    VkSampler m_textureSampler = VK_NULL_HANDLE;

    // Vulkan Object Resources
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
    std::vector<VkBuffer> m_uniformBuffers = {};
    std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
	std::vector<void*> m_uniformBuffersMapped = {};

    std::vector<VkDescriptorSet> m_descriptorSets = {};

    // Transform
    glm::vec3 position = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 rotation = glm::vec3(1.0f,1.0f,1.0f);
    glm::vec3 scale = glm::vec3(1.0f,1.0f,1.0f);
};
#endif
