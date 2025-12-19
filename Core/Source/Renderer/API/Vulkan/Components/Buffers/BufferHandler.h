#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H
#include <vulkan/vulkan.h>

#include <vector>

#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/Components/Mesh/Mesh3D.h"

struct Vertex;
struct LightUBO;

class BufferHandler {
   public:
    BufferHandler(VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkanGlobalState = VulkanGlobalState;
    }

    void DestroyBuffers();

    void CreateVertexBuffer(std::vector<Vertex> vertices);

    void CreateIndexBuffer(std::vector<uint32_t> indices);

    void CreateUniformBuffers();

    void CreateCameraUBO();

    void CreateLightUBO();

    void CreateModelUBO(Mesh3D* mesh);

    void CreateCommandBuffers();

    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);

    VkBuffer VertexBuffer = 0;
    VkDeviceMemory VertexBufferMemory = 0;

    VkBuffer IndexBuffer = 0;
    VkDeviceMemory IndexBufferMemory = 0;

    std::vector<VkCommandBuffer> CommandBuffers = {};

    std::vector<VkBuffer> UniformBuffers = {};
    std::vector<VkDeviceMemory> UniformBuffersMemory = {};
    std::vector<void*> UniformBuffersMapped = {};

    VkCommandBuffer BeginSingleTimeCommands();

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);

   private:
    VulkanGlobalState* m_vulkanGlobalState = nullptr;

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    // COPY BUFFER

    uint32_t FindMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties);
};
#endif
