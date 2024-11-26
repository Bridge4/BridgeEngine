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
	RenderPassHandler(VulkanContext* vulkanContext, SwapChainHandler* swapChainHandler, DeviceHandler* deviceHandler) {
		this->vulkanContext = vulkanContext;
		this->swapChainHandler = swapChainHandler;
		this->deviceHandler = deviceHandler;
	}

	void Initialize();
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	
	VkRenderPass renderPass = 0;
	VulkanContext* vulkanContext = 0;
	SwapChainHandler* swapChainHandler = 0;
	DeviceHandler* deviceHandler = 0;
};
#endif

