#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H
#include "../ComponentDeclarations.h"
#include "../glm/glm.hpp"


class CameraController
{
public:
	
	CameraController(VulkanContext* vulkanContext, WindowHandler* windowHandler, SwapChainHandler* swapChainHandler, BufferHandler* bufferHandler, VulkanInstanceManager* vulkanInstanceManager) {
		this->m_vulkanContext = vulkanContext;
		this->m_bufferHandler = bufferHandler;
		this->m_windowHandler = windowHandler;
		this->m_swapChainHandler = swapChainHandler;
		this->m_vulkanInstanceManager = vulkanInstanceManager;
		Initialize();
	}
	glm::mat4 GetViewMatrix();
	void HandleInput(float deltaTime);
	void UpdateCameraUBO(uint32_t currentImage, float deltaTime);
    float m_cameraSpeed = 5.0f;
private:
	void Initialize();
	VulkanContext* m_vulkanContext = 0;
	WindowHandler* m_windowHandler = 0;
	SwapChainHandler* m_swapChainHandler = 0;
	BufferHandler* m_bufferHandler = 0;
	VulkanInstanceManager* m_vulkanInstanceManager = 0;
	glm::vec3 m_eyePosition = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 m_viewDirection = glm::vec3(0.0f, 0.0f, 2.0f);
	glm::vec3 m_upVector = glm::vec3(0.0f, 0.0f, 1.0f);

	glm::mat4 m_cameraViewMatrix = glm::mat4(1.0f);

	bool m_lookActive = false;
	bool m_lookToggled = false;
	double m_prevMouseX, m_prevMouseY = 0.0;
};

#endif // !CAMERACONTROLLER_H
