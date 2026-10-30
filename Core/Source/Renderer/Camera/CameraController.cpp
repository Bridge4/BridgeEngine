#include "CameraController.h"

#include "Source/Renderer/API/Vulkan/Components/VulkanGlobalState/VulkanGlobalState.h"
// GLM INCLUDES

#include <../imgui/imgui.h>
#include <stdlib.h>

#include <format>
#include <iostream>

#include "GLFW/glfw3.h"
#include "Source/Math/Math.h"
#include "Source/Renderer/API/Vulkan/Components/SwapChain/SwapChainHandler.h"
#include "Source/Renderer/Camera/OrbitCamera.h"
#include "Source/Window/WindowingBackend.h"

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
                                   10.0f, 2.0f, 45.0f, 45.0f);
        m_cameraViewMatrix = GetViewMatrix();
        m_lookActive = false;
        m_lookToggled = false;
    }

    glfwGetCursorPos(m_windowingBackend->m_window, &m_mouseXBeforeLook,
                     &m_mouseYBeforeLook);
}

void CameraController::HandleInputNoClip(float deltaTime) {
    ImGui::Text("Hello, world %d", 123);
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_W)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f,
        // 0.001f, 0.001f)); cameraViewMatrix = glm::lookAt(eyePosition,
        // viewDirection+eyePosition, upVector);
        m_eyePosition += deltaTime * m_cameraSpeed * m_viewDirection;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_S)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix,
        // -glm::vec3(0.001f, 0.001f, 0.001f));
        m_eyePosition -= deltaTime * m_cameraSpeed * m_viewDirection;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_A)) {
        // glm::rotate(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(45.0f),
        // glm::vec3(0.0f, 0.0f, 1.0f)); cameraViewMatrix =
        // glm::rotate(cameraViewMatrix, glm::radians(0.01f), glm::vec3(0.0f,
        // 0.0f, 1.0f));
        m_eyePosition +=
            deltaTime * m_cameraSpeed *
            -glm::normalize(glm::cross(m_viewDirection, m_upVector));
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_D)) {
        // cameraViewMatrix = glm::rotate(cameraViewMatrix,
        // glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition -=
            deltaTime * m_cameraSpeed *
            -glm::normalize(glm::cross(m_viewDirection, m_upVector));
    }

    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_SPACE)) {
        // cameraViewMatrix = glm::rotate(cameraViewMatrix,
        // glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition += deltaTime * m_cameraSpeed *
                         -glm::normalize(glm::cross(
                             m_viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_LEFT_CONTROL)) {
        // cameraViewMatrix = glm::rotate(cameraViewMatrix,
        // glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        m_eyePosition -= deltaTime * m_cameraSpeed *
                         -glm::normalize(glm::cross(
                             m_viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_ESCAPE)) {
        exit(0);
    }

    if (glfwGetMouseButton(m_windowingBackend->m_window, GLFW_MOUSE_BUTTON_2) ==
        GLFW_PRESS) {
        if (!m_lookActive) {
            // glfwSetCursorPos(
            //     m_windowingBackend->m_window,
            //     (m_swapChainHandler->m_swapChainExtent.width / 2.0f),
            //     (m_swapChainHandler->m_swapChainExtent.height / 2.0f));
            m_lookToggled = true;
        }
        m_lookActive = true;
    }
    if (glfwGetMouseButton(m_windowingBackend->m_window, GLFW_MOUSE_BUTTON_2) ==
        GLFW_RELEASE) {
        if (m_lookActive) m_lookActive = false;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_L)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f,
        // 0.001f, 0.001f)); cameraViewMatrix = glm::lookAt(eyePosition,
        // viewDirection+eyePosition, upVector);
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_U)) {
        // cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f,
        // 0.001f, 0.001f)); cameraViewMatrix = glm::lookAt(eyePosition,
        // viewDirection+eyePosition, upVector);
    }
    if (m_lookActive) {
        double xPos, yPos;
        glfwSetInputMode(m_windowingBackend->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_HIDDEN);
        glfwGetCursorPos(m_windowingBackend->m_window, &xPos, &yPos);
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
    } else if (!m_lookActive/* &&
               glfwGetInputMode(m_windowingBackend->m_window, GLFW_CURSOR) ==
                   GLFW_CURSOR_HIDDEN*/) {
        std::cout << "DISABLED" << "\n";
        glfwSetInputMode(m_windowingBackend->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL);
    }
}

void CameraController::HandleInputOrbit(float deltaTime) {
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_ESCAPE)) {
        exit(0);
    }

    if (glfwGetMouseButton(m_windowingBackend->m_window, GLFW_MOUSE_BUTTON_2) ==
        GLFW_PRESS) {
        if (!m_lookActive) {
            glfwGetCursorPos(m_windowingBackend->m_window, &m_mouseXBeforeLook,
                             &m_mouseYBeforeLook);
            // glfwSetCursorPos(
            //     m_windowingBackend->m_window,
            //     m_swapChainHandler->m_swapChainExtent.width / 2.0,
            //     m_swapChainHandler->m_swapChainExtent.height / 2.0);

            glfwSetCursorPos(m_windowingBackend->m_window, m_mouseXBeforeLook,
                             m_mouseYBeforeLook);

            m_lookToggled = true;
        }
        m_lookActive = true;
        glfwSetInputMode(m_windowingBackend->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_DISABLED);
    }
    if (glfwGetMouseButton(m_windowingBackend->m_window, GLFW_MOUSE_BUTTON_2) ==
            GLFW_RELEASE &&
        m_lookActive) {
        std::cout << "shouldnt be here" << std::endl;
        m_lookActive = false;
        glfwSetInputMode(m_windowingBackend->m_window, GLFW_CURSOR,
                         GLFW_CURSOR_NORMAL);
        glfwSetCursorPos(m_windowingBackend->m_window, m_mouseXBeforeLook,
                         m_mouseYBeforeLook);
    }

    if (m_lookActive) {
        double xPos, yPos;
        glfwGetCursorPos(m_windowingBackend->m_window, &xPos, &yPos);

        float deltaX, deltaY;
        // deltaX = (xPos - m_mouseXBeforeLook) * m_cameraSpeed;
        deltaX = (xPos - m_prevMouseX) * m_cameraSpeed;
        if (m_invertHorizontal) deltaX = -deltaX;
        // deltaY = (yPos - m_mouseYBeforeLook) * m_cameraSpeed;
        deltaY = (yPos - m_prevMouseY) * m_cameraSpeed;
        if (!m_lookToggled) {
            if (deltaX != 0 || deltaY != 0) {
                orbitCam->RotateAzimuth(deltaX);
                orbitCam->RotatePolar(deltaY);
            }
        } else {
            m_lookToggled = false;
        }
        glfwSetCursorPos(m_windowingBackend->m_window, m_mouseXBeforeLook,
                         m_mouseYBeforeLook);
        m_prevMouseX = m_mouseXBeforeLook;
        m_prevMouseY = m_mouseYBeforeLook;
    } else {
        // std::cout << "DISABLEDDD\n";
        // glfwSetInputMode(m_windowingBackend->m_window, GLFW_CURSOR,
        //                  GLFW_CURSOR_NORMAL);
    }

    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_P)) {
        m_vulkanGlobalState->m_lights.lights[0].intensity.x +=
            100.0f * deltaTime;
        m_vulkanGlobalState->m_lights.lights[2].intensity.x +=
            100.0f * deltaTime;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_O)) {
        m_vulkanGlobalState->m_lights.lights[0].intensity.x -=
            100.0f * deltaTime;
        m_vulkanGlobalState->m_lights.lights[2].intensity.x -=
            100.0f * deltaTime;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_H)) {
        m_vulkanGlobalState->m_lights.lights[0].position.x -= 2.0f * deltaTime;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_L)) {
        m_vulkanGlobalState->m_lights.lights[0].position.x += 2.0f * deltaTime;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_J)) {
        m_vulkanGlobalState->m_lights.lights[0].position.y -= 2.0f * deltaTime;
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_K)) {
        m_vulkanGlobalState->m_lights.lights[0].position.y += 2.0f * deltaTime;
    }

    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_Q)) {
        orbitCam->Zoom(10.0f * deltaTime);
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_E)) {
        orbitCam->Zoom(-10.0f * deltaTime);
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_A)) {
        orbitCam->RotateAzimuth(10.0f * deltaTime);
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_D)) {
        orbitCam->RotateAzimuth(-10.0f * deltaTime);
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_W)) {
        orbitCam->RotatePolar(10.0f * deltaTime);
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_S)) {
        orbitCam->RotatePolar(-10.0f * deltaTime);
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_I)) {
        m_invertHorizontal = !m_invertHorizontal;
    }

    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_B)) {
        m_vulkanGlobalState->m_pbrPushConstants.bias.x += 0.001;
    }

    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_X)) {
    }
    if (glfwGetKey(m_windowingBackend->m_window, GLFW_KEY_Z)) {
    }
    glm::vec3 camPos = orbitCam->GetEye();

    float orthoSize = 500.0f;
}

void CameraController::UpdateCameraPosition(uint32_t currentImage,
                                            float deltaTime) {
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
}
