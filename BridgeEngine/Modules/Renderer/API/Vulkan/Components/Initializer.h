#pragma once
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif // !VK_USE_PLATFORM_WIN32_KHR
#include "Window.h"
#include "../VulkanDataStructures.h"


class Initializer {

public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

    //~Initializer();

    void init(Window* window);

    void assign(VkSurfaceKHR* rSurface, VkPhysicalDevice* pDevice, VkDevice* device, VkQueue* gQueue, VkQueue* pQueue);

    VkSurfaceKHR surface() { return r_surface; }

    VkPhysicalDevice physDevice() { return r_physicalDevice; }

    VkDevice logDevice() { return r_device; }

    VkQueue graphicsQueue() { return r_graphicsQueue; }

    VkQueue presentQueue() { return r_presentQueue; }

    void destroy();

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

    // Used by isDeviceSuitable() and createLogicalDevice()
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

private:
    // CONSTANTS
    const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation" };
    const std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    // CLASS ME
    VkInstance m_instance;
    VkDebugUtilsMessengerEXT m_debugMessenger;

    // r_ denotes private member accessed in public function
    VkSurfaceKHR r_surface;
    VkDevice r_device;
    VkPhysicalDevice r_physicalDevice = VK_NULL_HANDLE;
    VkQueue r_graphicsQueue;
    VkQueue r_presentQueue;

    // CORE FUNCTIONS
    
    void createInstance();
    void createDebugMessenger();
    void createSurface(Window window);
    void pickPhysicalDevice();
    void createLogicalDevice();

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
    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    // Used to destroy debugger
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

};