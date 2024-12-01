#include "CameraHandler.h"
#include <chrono>
// GLM INCLUDES

#include "../SwapChain/SwapChainHandler.h"
#include "../Buffers/BufferHandler.h"
//#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/vector_angle.hpp>
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
}

void CameraHandler::UpdateUniformBuffer(uint32_t currentImage)
{
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_W)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, glm::vec3(0.001f, 0.001f, 0.001f));
        //cameraViewMatrix = glm::lookAt(eyePosition, viewDirection+eyePosition, upVector);
        eyePosition += 0.001f * viewDirection;
    }
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_S)) {
        //cameraViewMatrix = glm::translate(cameraViewMatrix, -glm::vec3(0.001f, 0.001f, 0.001f));
        eyePosition -= 0.001f * viewDirection;

    }
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_A)) {
        //glm::rotate(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition += 0.001f * -glm::normalize(glm::cross(viewDirection, upVector));
    }
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_D)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition -= 0.001f * -glm::normalize(glm::cross(viewDirection, upVector));
    }

    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_SPACE)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition += 0.001f * -glm::normalize(glm::cross(viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }
    if (glfwGetKey(windowHandler->r_window, GLFW_KEY_LEFT_CONTROL)) {
        //cameraViewMatrix = glm::rotate(cameraViewMatrix, glm::radians(-0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
        eyePosition -= 0.001f * -glm::normalize(glm::cross(viewDirection, glm::vec3(1.0f, 0.0f, 0.0f)));
    }

    if (glfwGetMouseButton(windowHandler->r_window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
        lookActive = !lookActive;
        glfwSetCursorPos(windowHandler->r_window, (swapChainHandler->SwapChainExtent.width / 2), (swapChainHandler->SwapChainExtent.height / 2));
    }
    
    std::cout << "eyePosition: " << eyePosition.x << ", " << eyePosition.y << ", " << eyePosition.z << "\n";

    if (lookActive) {
        double xPos, yPos;
        glfwSetInputMode(windowHandler->r_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        glfwGetCursorPos(windowHandler->r_window, &xPos, &yPos);
        //std::cout << "X: " << xPos << " " << "Y: " << yPos << std::endl;
        if (prevMouseX != xPos || prevMouseY != yPos) {
            // No mouse movement
            float rotX = 1.0f * (float) ( (yPos - (swapChainHandler->SwapChainExtent.height / 2)) / swapChainHandler->SwapChainExtent.height );
            float rotY = 1.0f * (float) ( (xPos - (swapChainHandler->SwapChainExtent.width / 2)) / swapChainHandler->SwapChainExtent.width );

            std::cout << "rotX: " << rotX << "\n";
            std::cout << "roxY: " << rotY << "\n";


            glm::vec3 newViewDirection = glm::rotate(viewDirection, glm::radians(-rotX), glm::normalize(glm::cross(viewDirection, upVector)));
            /*if (abs(glm::angle(newOrientation, upVector) - glm::radians(90.0f)) <= glm::radians(85.0f))
            {
                viewDirection = newOrientation;
            }*/
            viewDirection = glm::rotate(viewDirection, glm::radians(rotY), upVector);
        }
        prevMouseX = xPos;
        prevMouseY = yPos;
    }
    else if (!lookActive && glfwGetInputMode(windowHandler->r_window, GLFW_CURSOR) == GLFW_CURSOR_HIDDEN){
        glfwSetInputMode(windowHandler->r_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    


    
    UniformBufferObject ubo{}; 
    
    // Where the object/model is placed in the world
    ubo.model = glm::mat4(1.0f);
    ubo.model = glm::rotate(ubo.model, -glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.model = glm::rotate(ubo.model, -glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    
    if (glfwGetMouseButton(windowHandler->r_window, GLFW_MOUSE_BUTTON_3) == GLFW_PRESS) {
        ubo.model = glm::rotate(ubo.model, -glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    //ubo.model = glm::rotate(ubo.model, -glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //ubo.model = glm::rotate(ubo.model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    // Where our camera is placed in the world
    ubo.view = getViewMatrix();


    ubo.proj = glm::perspective(glm::radians(90.0f), swapChainHandler->SwapChainExtent.width / (float)swapChainHandler->SwapChainExtent.height, 0.001f, 100000.0f);
    // IMPORTANT: VULKAN HAS INVERTED Y AXIS TO OPENGL AND GLM WAS DESIGNED FOR OPENGL. THIS CONVERTS TO VULKAN.
    ubo.proj[1][1] *= -1;

    memcpy(bufferHandler->UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
 }
