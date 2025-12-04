#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <../glm/glm.hpp>
#include "../glm/gtx/hash.hpp"
#include "vulkan/vulkan.h"
#include <array>

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
            VK_VERTEX_INPUT_RATE_VERTEX: Move to the next data entry after each vertex
            VK_VERTEX_INPUT_RATE_INSTANCE: Move to the next data entry after each instance
        */
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> GetAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        // position description
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].binding = 0;
        /*
            float: VK_FORMAT_R32_SFLOAT
            vec2:  VK_FORMAT_R32G32_SFLOAT
            vec3:  VK_FORMAT_R32G32B32_SFLOAT
            vec4:  VK_FORMAT_R32G32B32A32_SFLOAT
        */
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        // color description
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        /*
            Color type (SFLOAT, UINT, SINT) and bit width should match to type of shader input
            ivec2: VK_FORMAT_R32G32_SINT, a 2-component vector of 32-bit signed integers
            uvec4: VK_FORMAT_R32G32B32A32_UINT, a 4-component vector of 32-bit unsigned integers
            double: VK_FORMAT_R64_SFLOAT, a double-precision (64-bit) float
        */
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, normal);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && normal == other.normal && texCoord == other.texCoord;
    }
};

// This is doing some weird stuff I got from the vulkan tutorial I followed. Something about hashing vertex data
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}


// Scene-Wide Data
struct CameraUBO {
    alignas(16) glm::mat4 view;       // 64 bytes
    alignas(16) glm::mat4 proj;       // 64 bytes
    alignas(16) glm::vec4 cameraPos;  // 16 bytes (use xyz, ignore w)
};

struct Light {
    alignas(16) glm::vec4 position;   // xyz used, w ignored
    alignas(16) glm::vec4 color;      // rgb used, w ignored
    alignas(16) float intensity;      // pad to 16
    float pad[3];                     // explicit padding
};

struct LightUBO {
    alignas(16) Light lights[16];
    alignas(16)  int numLights;
    int pad[3]; // pad to 16 bytes
};

// Per-Mesh Data
struct ModelUBO {
    alignas(16) glm::mat4 model;
};
