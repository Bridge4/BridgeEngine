#ifndef VKTYPES_H
#define VKTYPES_H
#include <vulkan/vulkan_core.h>

#include <array>
#include <optional>

// #include "../glm/gtx/hash.m_vulkan_global_state"
#include "Source/Math/Math.h"
#include "Source/Renderer/Lights/Lights.h"
#include "vulkan/vulkan.h"

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        /*
            VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each
           vertex VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry
           after each instance
        */
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::vector<VkVertexInputAttributeDescription>
    GetAttributeDescriptions() {
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

        // position description
        VkVertexInputAttributeDescription attribute_desc;
        attribute_desc.location = 0;
        attribute_desc.binding = 0;
        attribute_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc.offset = offsetof(Vertex, pos);
        attributeDescriptions.push_back(attribute_desc);
        /*
            float: VK_FORMAT_R32_SFLOAT
            vec2:  VK_FORMAT_R32G32_SFLOAT
            vec3:  VK_FORMAT_R32G32B32_SFLOAT
            vec4:  VK_FORMAT_R32G32B32A32_SFLOAT
        */
        // color description
        attribute_desc.binding = 0;
        attribute_desc.location = 1;
        /*
            Color type (SFLOAT, UINT, SINT) and bit width should match to type
           of shader input ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of
           32-bit signed integers uvec4: VK_FORMAT_R32G32B32A32_UINT, a
           4-component vector of 32-bit unsigned integers double:
           VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float
        */
        attribute_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc.offset = offsetof(Vertex, color);
        attributeDescriptions.push_back(attribute_desc);

        attribute_desc.binding = 0;
        attribute_desc.location = 2;
        attribute_desc.format = VK_FORMAT_R32G32B32_SFLOAT;
        attribute_desc.offset = offsetof(Vertex, normal);
        attributeDescriptions.push_back(attribute_desc);

        attribute_desc.binding = 0;
        attribute_desc.location = 3;
        attribute_desc.format = VK_FORMAT_R32G32_SFLOAT;
        attribute_desc.offset = offsetof(Vertex, texCoord);
        attributeDescriptions.push_back(attribute_desc);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color &&
               normal == other.normal && texCoord == other.texCoord;
    }
};

// This is doing some weird stuff I got from the vulkan tutorial I followed.
// Something about hashing vertex data
namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const {
        return ((hash<glm::vec3>()(vertex.pos) ^
                 (hash<glm::vec3>()(vertex.color) << 1)) >>
                1) ^
               (hash<glm::vec2>()(vertex.texCoord) << 1);
    }
};
}  // namespace std

enum DescriptorSetLayoutType {
    SCENE,
    TEXTURED_MESH,
    UNTEXTURED_MESH,
    HEIGHT_MAP
};

struct DescriptorSetLayoutInfo {
    DescriptorSetLayoutType Type;
    std::vector<VkDescriptorSetLayoutBinding> Bindings;
    std::unordered_map<VkDescriptorType, uint32_t> BindingTypeCount;
    uint32_t BindingCount = Bindings.size();
};

struct DescriptorSetBindingInfo {
    uint32_t BindingCount;
    VkDescriptorType DescriptorType;
    VkShaderStageFlags ShaderStages;
    uint32_t DescriptorCount;
};

struct QueueFamilyIndices {
    std::optional<uint32_t> graphics_family;
    std::optional<uint32_t> present_family;
    std::optional<uint32_t> compute_family;

    bool isComplete() { return graphics_family.has_value(); }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

enum BufferBuildType { STAGED, UNSTAGED };

enum BeDrawType { MESH_TEXTURED, MESH_UNTEXTURED, GENERATED_TERRAIN_MESH };
#endif
