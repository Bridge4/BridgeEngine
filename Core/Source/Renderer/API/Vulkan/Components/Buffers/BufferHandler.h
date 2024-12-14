#ifndef BUFFERBUILDER_H
#define BUFFERBUILDER_H
#include <vulkan/vulkan.h>
#include <vector>
#include "../ComponentDeclarations.h"
//class VulkanContext;
//class DeviceHandler;
//class SwapChainHandler;

struct Vertex;
struct UniformBufferObject;


class BufferHandler
{
public:
	BufferHandler(VulkanInstanceManager* vulkanInstanceManager) {
		this->m_vulkanInstanceManager = vulkanInstanceManager;
	}

	void DestroyBuffers();

	void BuildVertexBuffer(std::vector<Vertex> vertices);

	void BuildIndexBuffer(std::vector<uint32_t> indices);

	void BuildUniformBuffers();

	void BuildCommandBuffers();

	void BuildBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

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


	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	// COPY BUFFER
	
	

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

};
#endif

