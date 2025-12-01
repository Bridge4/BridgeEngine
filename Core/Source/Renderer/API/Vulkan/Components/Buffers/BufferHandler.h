#ifndef BUFFERHANDLER_H
#define BUFFERHANDLER_H
#include <vulkan/vulkan.h>
#include <vector>
#include "../ComponentDeclarations.h"

struct Vertex;
struct UniformBufferObject;
struct LightUBO;


class BufferHandler
{
public:
	BufferHandler(VulkanInstanceManager* vulkanInstanceManager) {
		this->m_vulkanInstanceManager = vulkanInstanceManager;
	}

	void DestroyBuffers();

	void CreateVertexBuffer(std::vector<Vertex> vertices);

	void CreateIndexBuffer(std::vector<uint32_t> indices);

	void CreateUniformBuffers();

	void CreateCommandBuffers();

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

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
	VulkanInstanceManager* m_vulkanInstanceManager = nullptr;


	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	// COPY BUFFER
	
	

	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

};
#endif

