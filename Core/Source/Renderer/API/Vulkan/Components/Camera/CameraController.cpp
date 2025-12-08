#include "CameraController.h"

#include "../VulkanInstanceManager/VulkanInstanceManager.h"
// GLM INCLUDES

#include "../SwapChain/SwapChainHandler.h"
// #include <glm/glm.hpp>
#include <../glm/gtc/matrix_transform.hpp>
#include <../glm/gtx/hash.hpp>
#include <../glm/gtx/rotate_vector.hpp>
#include <../glm/gtx/string_cast.hpp>
#include <../glm/gtx/transform.hpp>
#include <../glm/gtx/vector_angle.hpp>

#include "../../../../DataStructures.h"
#include "../../VulkanContext.h"
#include "../Window/WindowHandler.h"
#include "GLFW/glfw3.h"
#include "Source/Renderer/API/Vulkan/Components/Camera/OrbitCamera.h"

glm::mat4 CameraController::GetViewMatrix() {
    m_viewDirection = glm::normalize(m_viewDirection);
    return (glm::lookAt(orbitCam->GetEye(), glm::vec3(0.0f, 0.0f, 0.0f),
                        m_upVector));
}

void CameraController::Initialize() {
    if (m_cameraType == NO_CLIP) {
        m_eyePosition = glm::vec3(0.0f, 0.0f, 2.0f);
        m_viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        m_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
        m_cameraViewMatrix = GetViewMatrix();
        m_lookActive = false;
        m_lookToggled = false;
    } else if (m_cameraType == ORBIT) {
        m_eyePosition = glm::vec3(0.0f, 0.0f, 2.0f);
        m_viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
        m_upVector = glm::vec3(0.0f, 1.0f, 0.0f);
        orbitCam = new OrbitCamera(glm::vec3(0.0f, 0.0f, 0.0f), m_upVector,
                                   3.0f, 100.0f, 45.0f, 45.0f);
        m_cameraViewMatrix = GetViewMatrix();
        m_lookActive = false;
        m_lookToggled = false;
    }
}

void CameraController::HandleInputNoClip(float deltaTime) {
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_W)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f,
        // 0.001f, 0.001f)); cameraViewMatrix = glm::lookAt(eyePosition,
        // viewDirection+eyePosition, upVector);
        m_eyePosition += deltaTime * m_cameraSpeed * m_viewDirection;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_S)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix,
        // -glm::vec3(0.001f, 0.001f, 0.001f));
        m_eyePosition -= deltaTime * m_cameraSpeed * m_viewDirection;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_A)) {
        // glm::rotate(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(45.0f),
        // glm::vec3(0.0f, 0.0f, 1.0f)); cameraViewMatrix =
        // glm::rotate(cameraViewMatrix, glm::radians(0.01f), glm::vec3(0.0f,
        // 0.0f, 1.0f));
        m_eyePosition +=
            deltaTime * m_cameraSpeed *
            -glm::normalize(glm::cross(m_viewDirection, m_upVector));
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_D)) {
        // cameraViewMatrix = glm::rotate(cameraViewMatrix,
        // glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition -=
            deltaTime * m_cameraSpeed *
            -glm::normalize(glm::cross(m_viewDirection, m_upVector));
    }

    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_SPACE)) {
        // cameraViewMatrix = glm::rotate(cameraViewMatrix,
        // glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition += deltaTime * m_cameraSpeed *
                         -glm::normalize(glm::cross(
                             m_viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_LEFT_CONTROL)) {
        // cameraViewMatrix = glm::rotate(cameraViewMatrix,
        // glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition -= deltaTime * m_cameraSpeed *
                         -glm::normalize(glm::cross(
                             m_viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_ESCAPE)) {
        exit(0);
    }

    if (glfwGetMouseButton(m_windowHandler->m_window, GLFW_MOUSE_BUTTON_2) ==
        GLFW_PRESS) {
        if (!m_lookActive) {
            glfwSetCursorPos(
                m_windowHandler->m_window,
                (m_swapChainHandler->SwapChainExtent.width / 2.0f),
                (m_swapChainHandler->SwapChainExtent.height / 2.0f));
            m_lookToggled = true;
        }
        m_lookActive = true;
    }
    if (glfwGetMouseButton(m_windowHandler->m_window, GLFW_MOUSE_BUTTON_2) ==
        GLFW_RELEASE) {
        m_lookActive = false;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_L)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f,
        // 0.001f, 0.001f)); cameraViewMatrix = glm::lookAt(eyePosition,
        // viewDirection+eyePosition, upVector);
        m_vulkanContext->LoadSceneObjects();
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_U)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f,
        // 0.001f, 0.001f)); cameraViewMatrix = glm::lookAt(eyePosition,
        // viewDirection+eyePosition, upVector);
        m_vulkanContext->UnloadSceneObjects();
    }
    if (m_lookActive) {
        double xPos, yPos;
        glfwSetInputMode(m_windowHandler->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_HIDDEN);
        glfwGetCursorPos(m_windowHandler->m_window, &xPos, &yPos);
        float deltaX, deltaY;
        deltaX = xPos - m_prevMouseX;
        deltaY = yPos - m_prevMouseY;
        if (!m_lookToggled) {
            if (deltaX != 0 || deltaY != 0) {
                glm::vec3 newViewDirection = glm::rotate(
                    m_viewDirection, glm::radians(-deltaY),
                    glm::normalize(glm::cross(m_viewDirection, m_upVector)));
                if (abs(glm::angle(newViewDirection, m_upVector) -
                        glm::radians(90.0f)) <= glm::radians(85.0f)) {
                    m_viewDirection = newViewDirection;
                }

                m_viewDirection = glm::rotate(
                    m_viewDirection, glm::radians(-deltaX), m_upVector);
            }
        } else {
            m_lookToggled = false;
        }
        m_prevMouseX = xPos;
        m_prevMouseY = yPos;
    } else if (!m_lookActive &&
               glfwGetInputMode(m_windowHandler->m_window, GLFW_CURSOR) ==
                   GLFW_CURSOR_HIDDEN) {
        glfwSetInputMode(m_windowHandler->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL);
    }
}

void CameraController::HandleInputOrbit(float deltaTime) {
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_ESCAPE)) {
        exit(0);
    }

    if (glfwGetMouseButton(m_windowHandler->m_window, GLFW_MOUSE_BUTTON_2) ==
        GLFW_PRESS) {
        if (!m_lookActive) {
            glfwSetCursorPos(
                m_windowHandler->m_window,
                m_swapChainHandler->m_swapChainExtent.width / 2.0,
                m_swapChainHandler->m_swapChainExtent.height / 2.0);
            m_lookToggled = true;
        }
        m_lookActive = true;
    } else if (glfwGetMouseButton(m_windowHandler->m_window,
                                  GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
        m_lookActive = false;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_L)) {
        m_vulkanContext->LoadSceneObjects();
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_U)) {
        m_vulkanContext->UnloadSceneObjects();
    }

    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_W)) {
        m_cameraSpeed += 0.0001f;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_S)) {
        m_cameraSpeed -= 0.0001f;
    }
    if (glfwGetKey(m_windowHandler->m_window, GLFW_KEY_I)) {
        m_invertHorizontal = !m_invertHorizontal;
    }
    if (m_lookActive) {
        double xPos, yPos;
        glfwGetCursorPos(m_windowHandler->m_window, &xPos, &yPos);

        float deltaX, deltaY;
        glfwSetInputMode(m_windowHandler->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_HIDDEN);
        deltaX = (xPos - m_prevMouseX) * m_cameraSpeed;
        if (m_invertHorizontal) deltaX = -deltaX;
        deltaY = (yPos - m_prevMouseY) * m_cameraSpeed;
        if (!m_lookToggled) {
            if (deltaX != 0 || deltaY != 0) {
                orbitCam->RotateAzimuth(deltaX);
                orbitCam->RotatePolar(deltaY);
            }
        } else {
            m_lookToggled = false;
        }
        glfwSetCursorPos(m_windowHandler->m_window,
                         m_vulkanGlobalState->m_swapChainExtent.width / 2.0,
                         m_vulkanGlobalState->m_swapChainExtent.height / 2.0);
        m_prevMouseX = m_vulkanGlobalState->m_swapChainExtent.width / 2.0;
        m_prevMouseY = m_vulkanGlobalState->m_swapChainExtent.height / 2.0;
    } else {
        glfwSetInputMode(m_windowHandler->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL);
    }
}

void CameraController::UpdateCameraUBO(uint32_t currentImage, float deltaTime) {
    m_viewDirection = glm::normalize(m_viewDirection);

    m_cameraUBO.view = GetViewMatrix();
    m_cameraUBO.cameraPos = glm::vec4(orbitCam->GetEye(), 0.0f);
    m_cameraUBO.proj = glm::perspectiveZO(
        glm::radians(90.0f),
        m_vulkanGlobalState->GetSwapChainExtent().width /
            (float)m_vulkanGlobalState->GetSwapChainExtent().height,
        0.5f, 2000.0f);
    // Invert GLM for Vulkan
    m_cameraUBO.proj[1][1] *= -1;
    memcpy(m_vulkanGlobalState
               ->m_cameraUBOMapped[m_vulkanGlobalState->m_currentFrame],
           &m_cameraUBO, sizeof(m_cameraUBO));
    if (!m_vulkanGlobalState->m_meshList.empty()) {
        int count = 0;
        for (auto& mesh : m_vulkanGlobalState->m_meshList) {
            ModelUBO modelUBO{};
            modelUBO.model = glm::mat4(1.0f);
            modelUBO.model =
                glm::translate(modelUBO.model, glm::vec3(0.0f, 0.0f, 0.0f));
            modelUBO.model = glm::rotate(modelUBO.model, -glm::radians(90.0f),
                                         glm::vec3(1.0f, 0.0f, 0.0f));
            modelUBO.model =
                glm::scale(modelUBO.model, glm::vec3(0.1f, 0.1f, 0.1f));
            // if (count == 1) {
            //     modelUBO.model = glm::translate(
            //         modelUBO.model,
            //         mesh.m_position + glm::vec3(10.0f, 0.0f, 0.0f));
            //     modelUBO.model =
            //         glm::rotate(modelUBO.model, -glm::radians(180.0f),
            //                     glm::vec3(0.0f, 1.0f, 0.0f));
            // } else {
            //     modelUBO.model =
            //
            //         glm::translate(modelUBO.model, glm::vec3(0.0f, 0.0f,
            //         0.0f));
            //     modelUBO.model =
            //         glm::rotate(modelUBO.model, -glm::radians(90.0f),
            //                     glm::vec3(1.0f, 0.0f, 0.0f));
            //     modelUBO.model =
            //         glm::scale(modelUBO.model, glm::vec3(0.5f, 0.5f, 0.5f));
            // }
            if (glfwGetMouseButton(m_windowHandler->m_window,
                                   GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
            }

            memcpy(mesh.m_uniformBuffersMapped[m_vulkanGlobalState
                                                   ->m_currentFrame],
                   &modelUBO, sizeof(modelUBO));
            count++;
        }
    }
}
