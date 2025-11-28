#pragma once
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif // !VK_USE_PLATFORM_WIN32_KHR

#include "../../VulkanDataStructures.h"
#include "../ComponentDeclarations.h"
//class WindowHandler;
//class VulkanContext;

/*
*   Provides Physical and Logical Devices for use in Vulkan Context
*   This class also sets up validation layers and the debug messenger.Will extract that out of the class in the future but for now prioritizing other tasks
*/ 
class DeviceHandler {

public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    DeviceHandler(VulkanInstanceManager* vulkanInstanceManager) {
        this->m_vulkanInstanceManager = vulkanInstanceManager;
    }

    void Initialize();

    void Destroy();

    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice);

    // Used by isDeviceSuitable() and createLogicalDevice()
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice);

    
    VkPhysicalDevice PhysicalDevice = nullptr;
    VkDevice LogicalDevice = nullptr;
private:
    // CONSTANTS
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VulkanInstanceManager* m_vulkanInstanceManager = 0;
    void InitializePhysicalDevice();
    void InitializeLogicalDevice();
    void InitializeDebugMessenger();
    // CORE FUNCTIONS

    // HELPER FUNCTIONS
    // Used by createInstance()
    bool CheckValidationLayerSupport();
    //std::vector<const char*> getRequiredExtensions();

    // Used by pickPhysicalDevice()
    bool IsDeviceSuitable(VkPhysicalDevice device);

    // Used by isDeviceSuitable()
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    // Used by populateDebugMessengerCreateInfo()
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData);
    // Used by setupDebugMessenger()
    VkResult CreateDebugUtilsMessengerEXT(  const VkDebugUtilsMessengerCreateInfoEXT*   pCreateInfo,
                                            const VkAllocationCallbacks*                pAllocator,
                                                  VkDebugUtilsMessengerEXT*             pDebugMessenger);

    // Used to destroy debugger
    void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator);
};
