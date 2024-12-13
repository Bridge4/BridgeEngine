#ifndef RENDERPASSHANDLER_H
#define RENDERPASSHANDLER_H
#include <vulkan/vulkan.h>
#include <vector>
#include "../ComponentDeclarations.h"
//class VulkanContext;
//class SwapChainHandler;
//class DeviceHandler;

//enum VkFormat;

class RenderPassHandler
{
public:
	RenderPassHandler(VulkanContext* vulkanContext, SwapChainHandler* swapChainHandler, DeviceHandler* deviceHandler, VulkanInstanceManager* vulkanInstanceManager) {
		this->vulkanContext = vulkanContext;
		this->swapChainHandler = swapChainHandler;
		this->deviceHandler = deviceHandler;
		this->m_vulkanInstanceManager = vulkanInstanceManager;
	}

	void Initialize();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	
	VkRenderPass renderPass = 0;
	VulkanContext* vulkanContext = 0;
	SwapChainHandler* swapChainHandler = 0;
	DeviceHandler* deviceHandler = 0;
	VulkanInstanceManager* m_vulkanInstanceManager = nullptr;
};
#endif

