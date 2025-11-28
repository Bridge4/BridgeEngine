#include "CameraHandler.h"
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
#include <chrono>
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
#include "../../../../DataStructures.h"
#include "../Window/WindowHandler.h"
#include "GLFW/glfw3.h"
#include <iostream>

glm::mat4 CameraHandler::getViewMatrix() {
    //glm::mat4 tempMat = glm::lookAt(eyePosition, viewDirection, upVector);

    //std::cout << glm::to_string(tempMat) << std::endl;
    return(glm::lookAt(eyePosition, eyePosition + viewDirection, upVector));
}

void CameraHandler::Initialize() {
   eyePosition = glm::vec3(0.0f, 0.0f, 2.0f);
   viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
   upVector = glm::vec3(0.0f, 1.0f, 0.0f);
   cameraViewMatrix = getViewMatrix();
   lookActive = false;
   lookToggled = false;
}


void CameraHandler::HandleInput(float deltaTime) {
    if (glfwGetKey(windowHandler->m_window, GLFW_KEY_W)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f, 0.001f, 0.001f));
        //cameraViewMatrix = glm::lookAt(eyePosition, viewDirection+eyePosition, upVector);
        eyePosition += deltaTime * cameraSpeed * viewDirection;
    }
    if (glfwGetKey(windowHandler->m_window, GLFW_KEY_S)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, -glm::vec3(0.001f, 0.001f, 0.001f));
        eyePosition -= deltaTime * cameraSpeed * viewDirection;

    }
    if (glfwGetKey(windowHandler->m_window, GLFW_KEY_A)) {
        //glm::rotate(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition += deltaTime * cameraSpeed * -glm::normalize(glm::cross(viewDirection, upVector));
    }
    if (glfwGetKey(windowHandler->m_window, GLFW_KEY_D)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition -= deltaTime * cameraSpeed * -glm::normalize(glm::cross(viewDirection, upVector));
    }

    if (glfwGetKey(windowHandler->m_window, GLFW_KEY_SPACE)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition += deltaTime * cameraSpeed * -glm::normalize(glm::cross(viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(windowHandler->m_window, GLFW_KEY_LEFT_CONTROL)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition -= deltaTime * cameraSpeed * -glm::normalize(glm::cross(viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(windowHandler->m_window, GLFW_KEY_ESCAPE)) {
        exit(0);
    }

    if (glfwGetMouseButton(windowHandler->m_window, GLFW_MOUSE_BUTTON_2) == GLFW_PRESS) {
        if (!lookActive) {
            glfwSetCursorPos(windowHandler->m_window, (swapChainHandler->SwapChainExtent.width / 2), (swapChainHandler->SwapChainExtent.height / 2));
            lookToggled = true;

        }
        lookActive = true;

    }
    if (glfwGetMouseButton(windowHandler->m_window, GLFW_MOUSE_BUTTON_2) == GLFW_RELEASE) {
        lookActive = false;
    }
}
void CameraHandler::UpdateUniformBuffer(uint32_t currentImage, float deltaTime)
{
    if (lookActive) {
        double xPos, yPos;
        glfwSetInputMode(windowHandler->m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwGetCursorPos(windowHandler->m_window, &xPos, &yPos);
        //std::cout << "X: " << xPos << " " << "Y: " << yPos << std::endl;
        float deltaX, deltaY;
        deltaX = xPos - prevMouseX;
        deltaY = yPos - prevMouseY;
        if (!lookToggled) {
            if (deltaX != 0 || deltaY != 0) {
                glm::vec3 newViewDirection = glm::rotate(viewDirection, glm::radians(-deltaY), glm::normalize(glm::cross(viewDirection, upVector)));
                if (abs(glm::angle(newViewDirection, upVector) - glm::radians(90.0f)) <= glm::radians(85.0f))
                {
                    viewDirection = newViewDirection;
                }

                viewDirection = glm::rotate(viewDirection, glm::radians(-deltaX), upVector);
            }
        }
        else {
            lookToggled = false;
        }
        
        prevMouseX = xPos;
        prevMouseY = yPos;
    }
    else if (!lookActive && glfwGetInputMode(windowHandler->m_window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN){
        glfwSetInputMode(windowHandler->m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    


    
    for(auto& mesh: m_vulkanInstanceManager->m_meshList) {
        UniformBufferObject ubo{}; 
        
        // Where the object/model is placed in the world
        ubo.model = glm::mat4(1.0f);
        ubo.model = glm::rotate(ubo.model, -glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        ubo.model = glm::rotate(ubo.model, -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));


        if (glfwGetMouseButton(windowHandler->m_window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
            ubo.model = glm::rotate(ubo.model, -glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        }
        //ubo.model = glm::rotate(ubo.model, -glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //ubo.model = glm::rotate(ubo.model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        // Where our camera is placed in the world
        ubo.model = glm::scale(ubo.model, mesh.scale);
        ubo.view = getViewMatrix();


        ubo.proj = glm::perspective(glm::radians(90.0f), m_vulkanInstanceManager->GetSwapChainExtent().width / (float)m_vulkanInstanceManager->GetSwapChainExtent().height, 0.001f, 100000.0f);
        // IMPORTANT: VULKAN HAS INVERTED Y AXIS TO OPENGL AND GLM WAS DESIGNED FOR OPENGL. THIS CONVERTS TO VULKAN.
        ubo.proj[1][1] *= -1;

        memcpy(mesh.m_uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));

    }
 }
