#include "CameraController.h"
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
// GLM INCLUDES

#include "../SwapChain/SwapChainHandler.h"
#include "../Buffers/BufferHandler.h"
//#include <glm/glm.hpp>
#include <../glm/gtc/matrix_transform.hpp>
#include <../glm/gtx/transform.hpp>
#include <../glm/gtx/rotate_vector.hpp>
#include <../glm/gtx/hash.hpp>
#include <../glm/gtx/string_cast.hpp>
#include <../glm/gtx/vector_angle.hpp>
#include <iostream>
#include "../../../../DataStructures.h"
#include "../Window/WindowHandler.h"
#include "GLFW/glfw3.h"
#include "../../VulkanContext.h"

glm::mat4 CameraController::GetViewMatrix() {
    return(glm::lookAt(m_eyePosition, m_eyePosition + m_viewDirection, m_upVector));
}

void CameraController::Initialize() {
   m_eyePosition = glm::vec3(0.0f, 0.0f, 0.0f);
   m_viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
   m_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
   m_cameraViewMatrix = GetViewMatrix();
   m_lookActive = false;
   m_lookToggled = false;
}


void CameraController::HandleInput(float deltaTime) {
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_W)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f, 0.001f, 0.001f));
        //cameraViewMatrix = glm::lookAt(eyePosition, viewDirection+eyePosition, upVector);
        m_eyePosition += deltaTime * m_cameraSpeed * m_viewDirection;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_S)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, -glm::vec3(0.001f, 0.001f, 0.001f));
        m_eyePosition -= deltaTime * m_cameraSpeed * m_viewDirection;

    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_A)) {
        //glm::rotate(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition += deltaTime * m_cameraSpeed * -glm::normalize(glm::cross(m_viewDirection, m_upVector));
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_D)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition -= deltaTime * m_cameraSpeed * -glm::normalize(glm::cross(m_viewDirection, m_upVector));
    }

    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_SPACE)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition += deltaTime * m_cameraSpeed * -glm::normalize(glm::cross(m_viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_LEFT_CONTROL)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition -= deltaTime * m_cameraSpeed * -glm::normalize(glm::cross(m_viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_ESCAPE)) {
        exit(0);
    }

    if (glfwGetMouseButton(m_windowHandler->m_window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
        if (!m_lookActive) {
            glfwSetCursorPos(m_windowHandler->m_window, (m_swapChainHandler->SwapChainExtent.width / 2.0f), (m_swapChainHandler->SwapChainExtent.height / 2.0f));
            m_lookToggled = true;

        }
        m_lookActive = true;

    }
    if (glfwGetMouseButton(m_windowHandler->m_window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
        m_lookActive = false;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_L)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f, 0.001f, 0.001f));
        //cameraViewMatrix = glm::lookAt(eyePosition, viewDirection+eyePosition, upVector);
        m_vulkanContext->LoadSceneObjects();
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_U)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f, 0.001f, 0.001f));
        //cameraViewMatrix = glm::lookAt(eyePosition, viewDirection+eyePosition, upVector);
        m_vulkanContext->UnloadSceneObjects();
    }
}
void CameraController::UpdateCameraUBO(uint32_t currentImage, float deltaTime)
{
    if (m_lookActive) {
        double xPos, yPos;
        glfwSetInputMode(m_windowHandler->m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwGetCursorPos(m_windowHandler->m_window, &xPos, &yPos);
        float deltaX, deltaY;
        deltaX = xPos - m_prevMouseX;
        deltaY = yPos - m_prevMouseY;
        if (!m_lookToggled) {
            if (deltaX != 0 || deltaY != 0) {
                glm::vec3 newViewDirection = glm::rotate(m_viewDirection, glm::radians(-deltaY), glm::normalize(glm::cross(m_viewDirection, m_upVector)));
                if (abs(glm::angle(newViewDirection, m_upVector) - glm::radians(90.0f)) <= glm::radians(85.0f))
                {
                    m_viewDirection = newViewDirection;
                }

                m_viewDirection = glm::rotate(m_viewDirection, glm::radians(-deltaX), m_upVector);
            }
        }
        else {
            m_lookToggled = false;
        }
        m_prevMouseX = xPos;
        m_prevMouseY = yPos;
    }
    else if (!m_lookActive && glfwGetInputMode(m_windowHandler->m_window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN){
        glfwSetInputMode(m_windowHandler->m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }


    if (!m_vulkanInstanceManager->m_meshList.empty()){
        for(auto& mesh: m_vulkanInstanceManager->m_meshList) {
            ModelUBO modelUBO{}; 
            modelUBO.model = glm::mat4(1.0f);
            modelUBO.model = glm::translate(modelUBO.model, mesh.m_position);
            modelUBO.model = glm::rotate(modelUBO.model, -glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            modelUBO.model = glm::rotate(modelUBO.model, -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            if (glfwGetMouseButton(m_windowHandler->m_window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
                modelUBO.model = glm::rotate(modelUBO.model, -glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            modelUBO.model = glm::scale(modelUBO.model, mesh.m_scale);

            memcpy(mesh.m_uniformBuffersMapped[m_vulkanInstanceManager->m_currentFrame], &modelUBO, sizeof(modelUBO));
        }
    }
    m_cameraUBO.view = GetViewMatrix();
    m_cameraUBO.cameraPos = m_eyePosition;
    m_cameraUBO.proj = glm::perspective(glm::radians(90.0f), m_vulkanInstanceManager->GetSwapChainExtent().width / (float)m_vulkanInstanceManager->GetSwapChainExtent().height, 0.001f, 1000.0f);
    // IMPORTANT: VULKAN HAS INVERTED Y AXIS TO OPENGL AND GLM WAS DESIGNED FOR OPENGL. THIS CONVERTS TO VULKAN.
    m_cameraUBO.proj[1][1] *= -1;
    memcpy(m_vulkanInstanceManager->m_cameraUBOMapped[currentImage], &m_cameraUBO, sizeof(m_cameraUBO));

 }
