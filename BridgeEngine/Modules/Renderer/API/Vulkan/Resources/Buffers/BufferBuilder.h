#ifndef BUFFERBUILDER_H
#define BUFFERBUILDER_H
#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;

struct Vertex;
struct UniformBufferObject;

enum BufferBuildType;

class BufferBuilder
{
public:
	BufferBuilder(VulkanContext* vulkanContext) {
		this->vulkanContext = vulkanContext;
	}

	void BuildVertexBuffer(BufferBuildType buildType, std::vector<Vertex> vertices);

	void BuildIndexBuffer(BufferBuildType buildType, std::vector<uint32_t> indices);

	void BuildUniformBuffer(BufferBuildType buildType, std::vector<UniformBufferObject> ubos);

	void BuildCommandBuffers();

	VkBuffer VertexBuffer = nullptr;

	VkBuffer IndexBuffer = nullptr;

	std::vector<VkCommandBuffer> CommandBuffers = {};

	std::vector<UniformBufferObject> UniformBufferObjects = {};

private:

	VulkanContext* vulkanContext = nullptr;

	VkCommandPool m_commandPool = nullptr;

	void buildBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	// COPY BUFFER
	
	VkCommandBuffer beginSingleTimeCommands();

	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

};
#endif

