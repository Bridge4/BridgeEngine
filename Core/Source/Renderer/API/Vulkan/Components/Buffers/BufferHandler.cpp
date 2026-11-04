#include "BufferHandler.h"

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "../../VulkanContext.h"
#include "../Devices/DeviceHandler.h"
#include "../SwapChain/SwapChainHandler.h"
#include "../VulkanGlobalState/VulkanGlobalState.h"
#include "Source/Renderer/Lights/Lights.h"
#include "Source/Renderer/Mesh/Mesh3D.h"

/*
    *   STAGED BUFFER CREATION
    *
    *   Goal is to take data from sourceDataObject and put it into a
destination buffer
    *
    *   1 - Create a staging buffer with size = sizeof(sourceDataObject[0])
* sourceDataObject.size()
    *   2 - Map data into a VkDeviceMemory stagingBufferMemory
    *   3 - Copy data from sourceDataObject to a void* data object
    *   4 - Unmap stagingBufferMemory
    *   5 - Create destination buffer
    *   6 - Copy staging buffer into destination buffer
    *   7 - Destroy staging buffer
    *   8 - Free stagingBufferMemory
    *
    *   UNSTAGED BUFFER CREATION
    *   1 - Create a destination buffer with size = sizeof(sourceDataObject)
|| size = sizeof(sourceDataObject[0]) * sourceDataObject.size()
    *   2 - Map memory
    *
STAGED example:


    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(devices->LogicalDevice, stagingBufferMemory, 0, bufferSize,
0, &data); memcpy(data, m_indices.data(), (size_t)bufferSize);
    vkUnmapMemory(devices->LogicalDevice, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
m_indexBuffer, m_indexBufferMemory);

    copyBuffer(stagingBuffer, m_indexBuffer, bufferSize);

    vkDestroyBuffer(devices->LogicalDevice, stagingBuffer, nullptr);
    vkFreeMemory(devices->LogicalDevice, stagingBufferMemory, nullptr);
*/

void BufferHandler::DestroyBuffers() {
    vkQueueWaitIdle(m_vulkan_global_state->m_presentQueue);
    vkDestroyBuffer(m_vulkan_global_state->m_logical_device, IndexBuffer,
                    nullptr);
    vkFreeMemory(m_vulkan_global_state->m_logical_device, IndexBufferMemory,
                 nullptr);
    vkDestroyBuffer(m_vulkan_global_state->m_logical_device, VertexBuffer,
                    nullptr);
    vkFreeMemory(m_vulkan_global_state->m_logical_device, VertexBufferMemory,
                 nullptr);
}

void BufferHandler::CreateVertexBuffer(std::vector<Vertex> vertices) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    // Create staging buffer
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    // Map it
    void* data;
    vkMapMemory(m_vulkan_global_state->m_logical_device, stagingBufferMemory, 0,
                bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_vulkan_global_state->m_logical_device, stagingBufferMemory);

    // Create vertex buffer
    CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VertexBuffer, VertexBufferMemory);

    // Copy data from staging buffer to vertex buffer
    CopyBuffer(stagingBuffer, VertexBuffer, bufferSize);

    // cleanup
    vkDestroyBuffer(m_vulkan_global_state->m_logical_device, stagingBuffer,
                    nullptr);
    vkFreeMemory(m_vulkan_global_state->m_logical_device, stagingBufferMemory,
                 nullptr);
}

void BufferHandler::CreateIndexBuffer(std::vector<uint32_t> indices) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    /*
     *   STAGED BUFFER CREATION
     *
     *   Goal is to take data from sourceDataObject and put it into a
     * destination buffer
     *
     *   1 - Create a staging buffer with size = sizeof(sourceDataObject[0]) *
     * sourceDataObject.size() 2 - Map data into a VkDeviceMemory
     * stagingBufferMemory 3 - Copy data from sourceDataObject to a void* data
     * object 4 - Unmap stagingBufferMemory 5 - Create destination buffer 6 -
     * Copy staging buffer into destination buffer 7 - Destroy staging buffer 8
     * - Free stagingBufferMemory
     *
     *   UNSTAGED BUFFER CREATION
     *   1 - Create a destination buffer with size = sizeof(sourceDataObject) ||
     * size = sizeof(sourceDataObject[0]) * sourceDataObject.size() 2 - Map
     * memory
     */

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(m_vulkan_global_state->m_logical_device, stagingBufferMemory, 0,
                bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(m_vulkan_global_state->m_logical_device, stagingBufferMemory);

    CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, IndexBuffer, IndexBufferMemory);

    CopyBuffer(stagingBuffer, IndexBuffer, bufferSize);

    vkDestroyBuffer(m_vulkan_global_state->m_logical_device, stagingBuffer,
                    nullptr);
    vkFreeMemory(m_vulkan_global_state->m_logical_device, stagingBufferMemory,
                 nullptr);
}

template <typename T>
void BufferHandler::CreateUBO(T BufferType) {}

void BufferHandler::CreateCameraUBO() {
    VkDeviceSize bufferSize = sizeof(CameraUBO);

    // Create a uniform buffer per frame in flight
    m_vulkan_global_state->m_cameraUBO.resize(
        m_vulkan_global_state->m_maxFramesInFlight);
    m_vulkan_global_state->m_cameraUBOMemory.resize(
        m_vulkan_global_state->m_maxFramesInFlight);
    m_vulkan_global_state->m_cameraUBOMapped.resize(
        m_vulkan_global_state->m_maxFramesInFlight);

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        // bufferBuilder->BuildUniformBuffer(m_uniformBuffers[i],
        // m_uniformBuffersMemory[i], UNSTAGED) Create then Map to memory
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_vulkan_global_state->m_cameraUBO[i],
                     m_vulkan_global_state->m_cameraUBOMemory[i]);
        vkMapMemory(m_vulkan_global_state->m_logical_device,
                    m_vulkan_global_state->m_cameraUBOMemory[i], 0, bufferSize,
                    0, &m_vulkan_global_state->m_cameraUBOMapped[i]);
    }
}

void BufferHandler::CreateLightUBO() {
    VkDeviceSize bufferSize = sizeof(LightUBO);

    // Create a uniform buffer per frame in flight
    m_vulkan_global_state->m_lightUBO.resize(
        m_vulkan_global_state->m_maxFramesInFlight);
    m_vulkan_global_state->m_lightUBOMemory.resize(
        m_vulkan_global_state->m_maxFramesInFlight);
    m_vulkan_global_state->m_lightUBOMapped.resize(
        m_vulkan_global_state->m_maxFramesInFlight);

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        // bufferBuilder->BuildUniformBuffer(m_uniformBuffers[i],
        // m_uniformBuffersMemory[i], UNSTAGED) Create then Map to memory
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_vulkan_global_state->m_lightUBO[i],
                     m_vulkan_global_state->m_lightUBOMemory[i]);
        vkMapMemory(m_vulkan_global_state->m_logical_device,
                    m_vulkan_global_state->m_lightUBOMemory[i], 0, bufferSize,
                    0, &m_vulkan_global_state->m_lightUBOMapped[i]);
    }
}

void BufferHandler::CreateComputeSSBOs() {
    VkDeviceSize bufferSize = sizeof(InputSSBO);

    // bufferBuilder->BuildUniformBuffer(m_uniformBuffers[i],
    // m_uniformBuffersMemory[i], UNSTAGED) Create then Map to memory
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 m_vulkan_global_state->m_computeInputBuffer,
                 m_vulkan_global_state->m_computeInputBufferMemory);
    vkMapMemory(m_vulkan_global_state->m_logical_device,
                m_vulkan_global_state->m_computeInputBufferMemory, 0,
                bufferSize, 0,
                &m_vulkan_global_state->m_computeInputBufferMemoryMapped);

    bufferSize = sizeof(OutputSSBO);
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                     VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                 m_vulkan_global_state->m_computeOutputBuffer,
                 m_vulkan_global_state->m_computeOutputBufferMemory);
    vkMapMemory(m_vulkan_global_state->m_logical_device,
                m_vulkan_global_state->m_computeOutputBufferMemory, 0,
                bufferSize, 0,
                &m_vulkan_global_state->m_computeOutputBufferMemoryMapped);
}

void BufferHandler::CreateModelUBO(Mesh3D* mesh) {
    VkDeviceSize bufferSize = sizeof(ModelUBO);

    mesh->m_uniformBuffers.resize(m_vulkan_global_state->m_maxFramesInFlight);
    mesh->m_uniformBuffersMemory.resize(
        m_vulkan_global_state->m_maxFramesInFlight);
    mesh->m_uniformBuffersMapped.resize(
        m_vulkan_global_state->m_maxFramesInFlight);

    for (size_t i = 0; i < m_vulkan_global_state->m_maxFramesInFlight; i++) {
        // bufferBuilder->BuildUniformBuffer(m_uniformBuffers[i],
        // m_uniformBuffersMemory[i], UNSTAGED) Create then Map to memory
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     mesh->m_uniformBuffers[i],
                     mesh->m_uniformBuffersMemory[i]);
        vkMapMemory(m_vulkan_global_state->m_logical_device,
                    mesh->m_uniformBuffersMemory[i], 0, bufferSize, 0,
                    &mesh->m_uniformBuffersMapped[i]);
    }
}

void BufferHandler::CreateCommandBuffersCompute(VkDevice& logical_device,
                                                VkCommandPool& commandPool) {
    m_vulkan_global_state->m_commandBuffersCompute.resize(
        m_vulkan_global_state->m_maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    /*
        VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for
       execution, but cannot be called from other command buffers.
        VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can
       be called from primary command buffers.
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        (uint32_t)m_vulkan_global_state->m_commandBuffersCompute.size();

    if (vkAllocateCommandBuffers(
            logical_device, &allocInfo,
            m_vulkan_global_state->m_commandBuffersCompute.data()) !=
        VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void BufferHandler::CreateCommandBuffers() {
    m_vulkan_global_state->m_commandBuffers.resize(
        m_vulkan_global_state->m_maxFramesInFlight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_vulkan_global_state->m_commandPool;
    /*
        VK_COMMAND_BUFFER_LEVEL_PRIMARY: Can be submitted to a queue for
       execution, but cannot be called from other command buffers.
        VK_COMMAND_BUFFER_LEVEL_SECONDARY: Cannot be submitted directly, but can
       be called from primary command buffers.
    */
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount =
        (uint32_t)m_vulkan_global_state->m_commandBuffers.size();

    if (vkAllocateCommandBuffers(
            m_vulkan_global_state->m_logical_device, &allocInfo,
            m_vulkan_global_state->m_commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void BufferHandler::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                 VkMemoryPropertyFlags properties,
                                 VkBuffer& buffer,
                                 VkDeviceMemory& bufferMemory) {
    // bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
    // VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_uniformBuffers[i],
    // m_uniformBuffersMemory[i] Creation
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(m_vulkan_global_state->m_logical_device, &bufferInfo,
                       nullptr, &buffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create vertex buffer!");
    }

    // Allocation
    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(m_vulkan_global_state->m_logical_device,
                                  buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        FindMemoryType(memRequirements.memoryTypeBits, properties);

    // VkAllocateMemory is an expensive operation on the CPU side, so we should
    // minimize calls to it "Aggressively allocate your buffer as large as you
    // believe it will grow" - dude on stackoverflow
    if (vkAllocateMemory(m_vulkan_global_state->m_logical_device, &allocInfo,
                         nullptr, &bufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate buffer memory.");
    }

    // Binding
    vkBindBufferMemory(m_vulkan_global_state->m_logical_device, buffer,
                       bufferMemory, 0);
}

void BufferHandler::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer,
                               VkDeviceSize size) {
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
    allocInfo.commandPool = m_vulkan_global_state->m_commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(m_vulkan_global_state->m_logical_device,
                             &allocInfo, &commandBuffer);

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

    vkQueueSubmit(m_vulkan_global_state->m_graphicsQueue, 1, &submitInfo,
                  VK_NULL_HANDLE);
    vkQueueWaitIdle(m_vulkan_global_state->m_graphicsQueue);

    vkFreeCommandBuffers(m_vulkan_global_state->m_logical_device,
                         m_vulkan_global_state->m_commandPool, 1,
                         &commandBuffer);
}

uint32_t BufferHandler::FindMemoryType(uint32_t typeFilter,
                                       VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(
        m_vulkan_global_state->m_physical_device, &memProperties);

    // Go over this section
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type");
}
