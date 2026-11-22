#ifndef MESH3D_H
#define MESH3D_H

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

// #include <../glm/glm.hpp>
// #include <../glm/gtc/matrix_transform.hpp>
// #include <../glm/gtx/hash.hpp>
#include <unordered_map>

#include "Source/Math/Math.h"
#include "Source/Renderer/RendererTypes.h"

struct TextureData {
    VkImage m_texture_image;
    VkDeviceMemory m_texture_image_memory;
    VkImageView m_texture_image_view;
    VkSampler m_texture_sampler;
};

// Per-Mesh Data
struct ModelUBO {
    alignas(16) glm::mat4 model;
};

class Mesh3D {
   public:
    Mesh3D() {}

    Mesh3DTypes m_mesh_type = TEXTURED;
    // Offsets for shared index and vertex buffers during vkDraw calls
    int m_index_buffer_start_idx = 0;
    int m_index_buffer_end_idx = 0;

    int m_vertex_buffer_start_idx = 0;
    int m_vertex_buffer_end_idx = 0;

    int m_index_count = 0;

    VkImage m_texture_image = VK_NULL_HANDLE;
    VkDeviceMemory m_texture_image_memory = VK_NULL_HANDLE;
    VkImageView m_texture_image_view = VK_NULL_HANDLE;
    VkSampler m_texture_sampler = VK_NULL_HANDLE;
    bool should_draw = true;

    TextureData m_texture_data = {};
    std::unordered_map<MaterialEnums, TextureData> m_material_data_map = {};
    std::unordered_map<MaterialEnums, int32_t> m_material_indices = {};
    std::vector<VkBuffer> m_uniform_buffers = {};
    std::vector<VkDeviceMemory> m_uniform_buffers_memory = {};
    std::vector<void*> m_uniform_buffers_mapped = {};

    std::vector<VkDescriptorSet> m_descriptor_sets = {};

    // Transform
    glm::vec3 m_position = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 m_rotation = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::mat4 m_model_matrix = glm::mat4(1);
};
#endif
