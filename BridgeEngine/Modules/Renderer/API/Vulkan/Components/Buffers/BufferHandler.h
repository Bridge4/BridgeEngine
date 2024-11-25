#ifndef BUFFERBUILDER_H
#define BUFFERBUILDER_H
#include <vulkan/vulkan.h>
#include <vector>

class VulkanContext;
class DeviceHandler;
class SwapChainHandler;

struct Vertex;
struct UniformBufferObject;

enum BufferBuildType;

class BufferHandler
{
public:
	BufferHandler(VulkanContext* vulkanContext, DeviceHandler* deviceHandler, SwapChainHandler* swapChainHandler) {
		this->vulkanContext = vulkanContext;
		this->deviceHandler = deviceHandler;
		this->swapChainHandler = swapChainHandler;
	}

	void DestroyBuffers();

	void BuildVertexBuffer(BufferBuildType buildType, std::vector<Vertex> vertices);

	void BuildIndexBuffer(BufferBuildType buildType, std::vector<uint32_t> indices);

	void BuildUniformBuffers(BufferBuildType buildType);

	void UpdateUniformBuffer(uint32_t currentImage);

	void BuildCommandBuffers();

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

	VulkanContext* vulkanContext = nullptr;

	DeviceHandler* deviceHandler = nullptr;
	SwapChainHandler* swapChainHandler = nullptr;

	VkCommandPool m_commandPool = nullptr;

	void buildBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	// COPY BUFFER
	
	

	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

};
#endif

