#ifndef MESH3D_H
#define MESH3D_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <../glm/glm.hpp>
#include <../glm/gtc/matrix_transform.hpp>
#include <../glm/gtx/hash.hpp>
#include <string>
#include <unordered_map>

enum Mesh3DTypes { TEXTURED, UNTEXTURED };

struct TextureData {
    VkImage m_textureImage;
    VkDeviceMemory m_textureImageMemory;
    VkImageView m_textureImageView;
    VkSampler m_textureSampler;
};

enum MaterialEnums { ALBEDO, METALLIC, ROUGHNESS, AO, NORMAL, EMISSIVE };
class Mesh3D {
   public:
    Mesh3D() {}

    Mesh3DTypes MeshType = TEXTURED;
    // Offsets for shared index and vertex buffers during vkDraw calls
    int m_indexBufferStartIndex = 0;
    int m_indexBufferEndIndex = 0;

    int m_vertexBufferStartIndex = 0;
    int m_vertexBufferEndIndex = 0;

    int m_indexCount = 0;

    VkImage m_textureImage = VK_NULL_HANDLE;
    VkDeviceMemory m_textureImageMemory = VK_NULL_HANDLE;
    VkImageView m_textureImageView = VK_NULL_HANDLE;
    VkSampler m_textureSampler = VK_NULL_HANDLE;

    TextureData m_textureData = {};
    std::unordered_map<MaterialEnums, TextureData> m_materials = {};
    std::vector<VkBuffer> m_uniformBuffers = {};
    std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
    std::vector<void*> m_uniformBuffersMapped = {};

    std::vector<VkDescriptorSet> m_descriptorSets = {};

    // Transform
    glm::vec3 m_position = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 m_rotation = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::mat4 modelMatrix = glm::mat4(1);
};
#endif
