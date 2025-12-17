#ifndef CAMERACONTROLLER_H
#define CAMERACONTROLLER_H
#include "../ComponentDeclarations.h"
#include "Source/Renderer/API/Vulkan/Components/Camera/OrbitCamera.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <../glm/glm.hpp>

#include "Source/Renderer/DataStructures.h"

class CameraController {
   public:
    enum CameraType { NO_CLIP, ORBIT };
    CameraController(VulkanContext* vulkanContext, WindowHandler* windowHandler,
                     SwapChainHandler* swapChainHandler,
                     BufferHandler* bufferHandler,
                     VulkanInstanceManager* vulkanInstanceManager,
                     CameraType camType = ORBIT) {
        this->m_vulkanContext = vulkanContext;
        this->m_bufferHandler = bufferHandler;
        this->m_windowHandler = windowHandler;
        this->m_swapChainHandler = swapChainHandler;
        this->m_vulkanGlobalState = vulkanInstanceManager;
        this->m_cameraType = camType;
        Initialize();
    }
    glm::mat4 GetViewMatrix();
    void HandleInputNoClip(float deltaTime);
    void HandleInputOrbit(float deltaTime);
    void UpdateCameraUBO(uint32_t currentImage, float deltaTime);
    float m_cameraSpeed = 0.012f;
    CameraUBO m_cameraUBO;
    glm::vec3 m_eyePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_viewDirection = glm::vec3(0.0f, 0.0f, 2.0f);
    glm::vec3 m_upVector = glm::vec3(0.0f, 0.0f, 1.0f);
    CameraType m_cameraType;
    OrbitCamera* orbitCam = nullptr;
    bool m_invertHorizontal = false;

   private:
    void Initialize();
    VulkanContext* m_vulkanContext = 0;
    WindowHandler* m_windowHandler = 0;
    SwapChainHandler* m_swapChainHandler = 0;
    BufferHandler* m_bufferHandler = 0;
    VulkanInstanceManager* m_vulkanGlobalState = 0;

    glm::mat4 m_cameraViewMatrix = glm::mat4(1.0f);

    bool m_lookActive = false;
    bool m_lookToggled = false;
    double m_prevMouseX, m_prevMouseY = 0.0;
};

#endif  // !CAMERACONTROLLER_H
