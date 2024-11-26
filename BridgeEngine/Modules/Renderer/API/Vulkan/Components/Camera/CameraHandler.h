#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H
#include <iostream>
#include "../ComponentDeclarations.h"

// Find a way to point to current API rather than hardcoding vulkan

//class VulkanContext;
//class WindowHandler;
//class SwapChainHandler;
//class BufferHandler;

class CameraHandler
{
public:
	
	CameraHandler(VulkanContext* vulkanContext, WindowHandler* windowHandler, SwapChainHandler* swapChainHandler, BufferHandler* bufferHandler) {
		this->vulkanContext = vulkanContext;
		this->bufferHandler = bufferHandler;
		this->windowHandler = windowHandler;
		this->swapChainHandler = swapChainHandler;
	}
	void Move(uint32_t currentImage);
private:
	float x = 0.0f;
	float y = 0.0f;
	VulkanContext* vulkanContext = 0;
	WindowHandler* windowHandler = 0;
	SwapChainHandler* swapChainHandler = 0;
	BufferHandler* bufferHandler = 0;

};

#endif // !CAMERAHANDLER_H