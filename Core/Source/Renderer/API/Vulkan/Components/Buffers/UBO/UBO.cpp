#include "UBO.h"

template <typename T>
void UBO::Create(T Type, int FramesInFlight, VkDevice LogicalDevice) {
    VkDeviceSize bufferSize = sizeof(T);

    // Create a uniform buffer per frame in flight
    m_uniformBuffers.resize(FramesInFlight);
    m_uniformBuffersMemory.resize(FramesInFlight);
    m_uniformBuffersMapped.resize(FramesInFlight);

    for (size_t i = 0; i < FramesInFlight; i++) {
        // bufferBuilder->BuildUniformBuffer(m_uniformBuffers[i],
        // m_uniformBuffersMemory[i], UNSTAGED) Create then Map to memory
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                     m_uniformBuffers[i], m_uniformBuffersMemory[i]);
        vkMapMemory(m_bufferHandler->m_deviceHandler->LogicalDevice,
                    m_uniformBuffersMemory[i], 0, bufferSize, 0,
                    &m_uniformBuffersMapped[i]);
    }
}
