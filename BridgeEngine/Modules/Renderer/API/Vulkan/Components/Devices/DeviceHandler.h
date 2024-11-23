#pragma once
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif // !VK_USE_PLATFORM_WIN32_KHR

#include "../../VulkanDataStructures.h"

class WindowHandler;
class VulkanContext;

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
    DeviceHandler(VulkanContext* apiVulkan, WindowHandler* window) {
        vulkanContext = apiVulkan;
        windowRef = window;
    }

    void Initialize();

    void Destroy();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);

    // Used by isDeviceSuitable() and createLogicalDevice()
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);

    
    VkPhysicalDevice PhysicalDevice = nullptr;
    VkDevice LogicalDevice = nullptr;
private:
    // CONSTANTS
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    WindowHandler* windowRef = 0;
    VulkanContext* vulkanContext = 0;

    void InitializePhysicalDevice();
    void InitializeLogicalDevice();
    void InitializeDebugMessenger();
    // CORE FUNCTIONS

    // HELPER FUNCTIONS
    // Used by createInstance()
    bool checkValidationLayerSupport();
    //std::vector<const char*> getRequiredExtensions();

    // Used by pickPhysicalDevice()
    bool isDeviceSuitable(VkPhysicalDevice device);

    // Used by isDeviceSuitable()
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    // Used by populateDebugMessengerCreateInfo()
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
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