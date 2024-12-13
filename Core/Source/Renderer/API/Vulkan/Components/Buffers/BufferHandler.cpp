#pragma once
#include "BufferHandler.h"
#include "../../VulkanContext.h"
#include "../Devices/DeviceHandler.h"
#include "../../VulkanDataStructures.h"
#include "../SwapChain/SwapChainHandler.h"
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
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
    vkDestroyBuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), IndexBuffer, nullptr);
    vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), IndexBufferMemory, nullptr);
    vkDestroyBuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), VertexBuffer, nullptr);
    vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), VertexBufferMemory, nullptr);
}

void BufferHandler::BuildVertexBuffer(BufferBuildType buildType, std::vector<Vertex> vertices) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
     

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    BuildBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
    
    // Map it
    void* data;
    vkMapMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory);

    // Create vertex buffer
    BuildBuffer(bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        VertexBuffer, VertexBufferMemory);

    // Copy data from staging buffer to vertex buffer
    copyBuffer(stagingBuffer, VertexBuffer, bufferSize);

    // cleanup
    vkDestroyBuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory, nullptr);
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
    vkMapMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory);

    BuildBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

    copyBuffer(stagingBuffer, IndexBuffer, bufferSize);

    vkDestroyBuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBuffer, nullptr);
    vkFreeMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), stagingBufferMemory, nullptr);

}

void BufferHandler::BuildUniformBuffers(BufferBuildType buildType)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    // Create a uniform buffer per frame in flight
    m_vulkanInstanceManager->m_uniformBuffers.resize(m_vulkanInstanceManager->MAX_FRAMES_IN_FLIGHT);
    m_vulkanInstanceManager->m_uniformBuffersMemory.resize(m_vulkanInstanceManager->MAX_FRAMES_IN_FLIGHT);
    m_vulkanInstanceManager->m_uniformBuffersMapped.resize(m_vulkanInstanceManager->MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < m_vulkanInstanceManager->MAX_FRAMES_IN_FLIGHT; i++) {
        // bufferBuilder->BuildUniformBuffer(m_uniformBuffers[i], m_uniformBuffersMemory[i], UNSTAGED)
        // Create then Map to memory
        BuildBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vulkanInstanceManager->m_uniformBuffers[i], m_vulkanInstanceManager->m_uniformBuffersMemory[i]);
        vkMapMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_uniformBuffersMemory[i], 0, bufferSize, 0, &m_vulkanInstanceManager->m_uniformBuffersMapped[i]);
    }
}


void BufferHandler::BuildCommandBuffers() {
    m_vulkanInstanceManager->m_commandBuffers.resize(m_vulkanInstanceManager->MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_vulkanInstanceManager->m_commandPool;
    /*
        VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for execution, but cannot be called from other command buffers.
        VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can be called from primary command buffers.
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)m_vulkanInstanceManager->m_commandBuffers.size();

    if (vkAllocateCommandBuffers(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, m_vulkanInstanceManager->m_commandBuffers.data()) != VK_SUCCESS) {
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

    if (vkCreateBuffer(*m_vulkanInstanceManager->GetRefLogicalDevice(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    // Allocation
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(*m_vulkanInstanceManager->GetRefLogicalDevice(), buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

    // VkAllocateMemory is an expensive operation on the CPU side, so we should minimize calls to it
    // "Aggressively allocate your buffer as large as you believe it will grow" - dude on stackoverflow
    if (vkAllocateMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory.");
    }

    // Binding
    vkBindBufferMemory(*m_vulkanInstanceManager->GetRefLogicalDevice(), buffer, bufferMemory, 0);
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
    allocInfo.commandPool = m_vulkanInstanceManager->m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(*m_vulkanInstanceManager->GetRefLogicalDevice(), &allocInfo, &commandBuffer);

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

    vkQueueSubmit(m_vulkanInstanceManager->m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkanInstanceManager->m_graphicsQueue);

    vkFreeCommandBuffers(*m_vulkanInstanceManager->GetRefLogicalDevice(), m_vulkanInstanceManager->m_commandPool, 1, &commandBuffer);
}


uint32_t BufferHandler::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(m_vulkanInstanceManager->GetPhysicalDevice(), &memProperties);

    // Go over this section
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}
