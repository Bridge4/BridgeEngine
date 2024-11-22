#pragma once
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif // !VK_USE_PLATFORM_WIN32_KHR

#include "../VulkanDataStructures.h"

class Window;
class VulkanBridge;
class Initializer {

public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    Initializer(VulkanBridge* vulkanBridge, Window* window) {
        vulkanContext = vulkanBridge;
        windowRef = window;
    }

    void init();
    void createVulkanInstance();
    void createDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice(VkPhysicalDevice physicalDevice);
    //void assign(VkSurfaceKHR* rSurface, VkPhysicalDevice* pDevice, VkDevice* device, VkQueue* gQueue, VkQueue* pQueue);


    void destroy();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice physicalDevice);

    // Used by isDeviceSuitable() and createLogicalDevice()
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice);

    Window* windowRef = 0;
    VulkanBridge* vulkanContext = 0;
private:
    // CONSTANTS
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkSurfaceKHR r_surface;

    // CORE FUNCTIONS
    
    
    

    // HELPER FUNCTIONS
    // Used by createInstance()
    bool checkValidationLayerSupport();
    std::vector<const char*> getRequiredExtensions();

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