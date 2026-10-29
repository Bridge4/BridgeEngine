#ifndef UBO_H
#define UBO_H
#include <vector>

#include "Source/Renderer/API/Vulkan/Components/Buffers/BufferHandler.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"

class UBO : BufferHandler {
    BufferHandler* m_bufferHandler = 0;
    std::vector<VkBuffer> m_uniformBuffers = {};
    std::vector<VkDeviceMemory> m_uniformBuffersMemory = {};
    std::vector<void*> m_uniformBuffersMapped = {};

    template <typename T>
    void Create(T Type);
};

#endif
