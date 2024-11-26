#pragma once
#include "BufferHandler.h"
#include "../../VulkanContext.h"
#include "../Devices/DeviceHandler.h"
#include "../../VulkanDataStructures.h"
#include "../SwapChain/SwapChainHandler.h"
#include <vulkan/vulkan.h>
#include <chrono>


/*
    *   STAGED BUFFER CREATION
    *
    *   Goal is to take data from sourceDataObject and put it into a destination buffer
    *
    *   1 - Create a staging buffer with size = sizeof(sourceDataObject[0]) * sourceDataObject.size()
    *   2 - Map data into a VkDeviceMemory stagingBufferMemory
    *   3 - Copy data from sourceDataObject to a void* data object
    *   4 - Unmap stagingBufferMemory
    *   5 - Create destination buffer
    *   6 - Copy staging buffer into destination buffer
    *   7 - Destroy staging buffer
    *   8 - Free stagingBufferMemory
    *
    *   UNSTAGED BUFFER CREATION
    *   1 - Create a destination buffer with size = sizeof(sourceDataObject) || size = sizeof(sourceDataObject[0]) * sourceDataObject.size()
    *   2 - Map memory
    * 
STAGED example:


    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(devices->LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, m_indices.data(), (size_t)bufferSize);
    vkUnmapMemory(devices->LogicalDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_indexBuffer, m_indexBufferMemory);

    copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(devices->LogicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(devices->LogicalDevice, stagingBufferMemory, nullptr);
*/

void BufferHandler::DestroyBuffers() {
    vkDestroyBuffer(deviceHandler->LogicalDevice, IndexBuffer, nullptr);
    vkFreeMemory(deviceHandler->LogicalDevice, IndexBufferMemory, nullptr);
    vkDestroyBuffer(deviceHandler->LogicalDevice, VertexBuffer, nullptr);
    vkFreeMemory(deviceHandler->LogicalDevice, VertexBufferMemory, nullptr);
}

void BufferHandler::BuildVertexBuffer(BufferBuildType buildType, std::vector<Vertex> vertices) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
     

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    BuildBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    // Map it
    void* data;
    vkMapMemory(deviceHandler->LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(deviceHandler->LogicalDevice, stagingBufferMemory);

    // Create vertex buffer
    BuildBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VertexBuffer, VertexBufferMemory);

    // Copy data from staging buffer to vertex buffer
    copyBuffer(stagingBuffer, VertexBuffer, bufferSize);

    // cleanup
    vkDestroyBuffer(deviceHandler->LogicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(deviceHandler->LogicalDevice, stagingBufferMemory, nullptr);
}

void BufferHandler::BuildIndexBuffer(BufferBuildType buildType, std::vector<uint32_t> indices) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    
    /*
    *   STAGED BUFFER CREATION
    *
    *   Goal is to take data from sourceDataObject and put it into a destination buffer
    *
    *   1 - Create a staging buffer with size = sizeof(sourceDataObject[0]) * sourceDataObject.size()
    *   2 - Map data into a VkDeviceMemory stagingBufferMemory
    *   3 - Copy data from sourceDataObject to a void* data object
    *   4 - Unmap stagingBufferMemory
    *   5 - Create destination buffer
    *   6 - Copy staging buffer into destination buffer
    *   7 - Destroy staging buffer
    *   8 - Free stagingBufferMemory
    *
    *   UNSTAGED BUFFER CREATION
    *   1 - Create a destination buffer with size = sizeof(sourceDataObject) || size = sizeof(sourceDataObject[0]) * sourceDataObject.size()
    *   2 - Map memory
    */

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    BuildBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(deviceHandler->LogicalDevice, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(deviceHandler->LogicalDevice, stagingBufferMemory);

    BuildBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

    copyBuffer(stagingBuffer, IndexBuffer, bufferSize);

    vkDestroyBuffer(deviceHandler->LogicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(deviceHandler->LogicalDevice, stagingBufferMemory, nullptr);

}

void BufferHandler::BuildUniformBuffers(BufferBuildType buildType)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // Create a uniform buffer per frame in flight
    UniformBuffers.resize(vulkanContext->MAX_FRAMES_IN_FLIGHT);
    UniformBuffersMemory.resize(vulkanContext->MAX_FRAMES_IN_FLIGHT);
    UniformBuffersMapped.resize(vulkanContext->MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < vulkanContext->MAX_FRAMES_IN_FLIGHT; i++) {
        // bufferBuilder->BuildUniformBuffer(m_uniformBuffers[i], m_uniformBuffersMemory[i], UNSTAGED)
        // Create then Map to memory
        BuildBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, UniformBuffers[i], UniformBuffersMemory[i]);
        vkMapMemory(deviceHandler->LogicalDevice, UniformBuffersMemory[i], 0, bufferSize, 0, &UniformBuffersMapped[i]);
    }
}

void BufferHandler::UpdateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(70.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, -2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChainHandler->SwapChainExtent.width / (float)swapChainHandler->SwapChainExtent.height, 0.1f, 10.0f);
    // IMPORTANT: VULKAN HAS INVERTED Y AXIS TO OPENGL AND GLM WAS DESIGNED FOR OPENGL. THIS CONVERTS TO VULKAN.
    ubo.proj[1][1] *= -1;

    memcpy(UniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void BufferHandler::BuildCommandBuffers() {
    CommandBuffers.resize(vulkanContext->MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = vulkanContext->m_commandPool;
    /*
        VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
        VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)CommandBuffers.size();

    if (vkAllocateCommandBuffers(deviceHandler->LogicalDevice, &allocInfo, CommandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void BufferHandler::BuildBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
{
    // bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i], m_uniformBuffersMemory[i]
    // Creation
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(deviceHandler->LogicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    // Allocation
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(deviceHandler->LogicalDevice, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // VkAllocateMemory is an expensive operation on the CPU side, so we should minimize calls to it
    // "Aggressively allocate your buffer as large as you believe it will grow" - dude on stackoverflow
    if (vkAllocateMemory(deviceHandler->LogicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory.");
    }

    // Binding
    vkBindBufferMemory(deviceHandler->LogicalDevice, buffer, bufferMemory, 0);
}

void BufferHandler::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
    EndSingleTimeCommands(commandBuffer);
}

// beginSingleTimeCommands and endSingleTimeCommands are helpers for copyBuffer
VkCommandBuffer BufferHandler::BeginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = vulkanContext->m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(deviceHandler->LogicalDevice, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}


void BufferHandler::EndSingleTimeCommands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(vulkanContext->m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(vulkanContext->m_graphicsQueue);

    vkFreeCommandBuffers(deviceHandler->LogicalDevice, vulkanContext->m_commandPool, 1, &commandBuffer);
}


uint32_t BufferHandler::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(deviceHandler->PhysicalDevice, &memProperties);

    // Go over this section
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}
