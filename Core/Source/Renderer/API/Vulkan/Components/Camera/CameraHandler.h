#ifndef CAMERAHANDLER_H
#define CAMERAHANDLER_H
#include <iostream>
#include "../ComponentDeclarations.h"
#include "../glm/glm.hpp"


class CameraHandler
{
public:
	
	CameraHandler(VulkanContext* vulkanContext, WindowHandler* windowHandler, SwapChainHandler* swapChainHandler, BufferHandler* bufferHandler, VulkanInstanceManager* vulkanInstanceManager) {
		this->vulkanContext = vulkanContext;
		this->bufferHandler = bufferHandler;
		this->windowHandler = windowHandler;
		this->swapChainHandler = swapChainHandler;
		this->m_vulkanInstanceManager = vulkanInstanceManager;
		Initialize();
		/*eyePosition = glm::vec3(2.0f, -2.0f, 2.0f);
		viewDirection = glm::vec3(0.0f, 0.0f, 0.0f);
		upVector = glm::vec3(0.0f, 0.0f, 1.0f);*/

	}
	glm::mat4 getViewMatrix();
	void Initialize();
	void HandleInput();
	void UpdateUniformBuffer(uint32_t currentImage);
private:

	
	
	VulkanContext* vulkanContext = 0;
	WindowHandler* windowHandler = 0;
	SwapChainHandler* swapChainHandler = 0;
	BufferHandler* bufferHandler = 0;
	VulkanInstanceManager* m_vulkanInstanceManager = 0;
	glm::vec3 eyePosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 viewDirection = glm::vec3(0.0f, 0.0f, 2.0f);
	glm::vec3 upVector = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::mat4 cameraViewMatrix = glm::mat4(1.0f);

	bool lookActive = false;
	bool lookToggled = false;
	double prevMouseX, prevMouseY = 0.0;
};

#endif // !CAMERAHANDLER_H
