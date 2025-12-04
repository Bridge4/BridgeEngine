#pragma once
#include <optional>
#include <vulkan/vulkan.h>
#include <vector>

struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete() {
        return graphicsFamily.has_value();
    }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

enum BufferBuildType {
    STAGED,
    UNSTAGED
};

enum BeDrawType {
    MESH_TEXTURED,
    MESH_UNTEXTURED,
    GENERATED_TERRAIN_MESH
};

