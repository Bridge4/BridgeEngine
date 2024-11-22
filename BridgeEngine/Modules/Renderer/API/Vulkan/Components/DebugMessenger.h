//#pragma once
//#ifndef VK_USE_PLATFORM_WIN32_KHR
//#define VK_USE_PLATFORM_WIN32_KHR
//#endif // !VK_USE_PLATFORM_WIN32_KHR
//
//#include "../VulkanDataStructures.h"
//
//class Window;
//class VulkanBridge;
//class DebugMessenger {
//
//public:
//#ifdef NDEBUG
//    const bool enableValidationLayers = false;
//#else
//    const bool enableValidationLayers = true;
//#endif
//
//    DebugMessenger(VulkanBridge* vulkanBridge) {
//        vulkanContext = vulkanBridge;
//    }
//
//    void Create();
//
//    void Destroy(const VkAllocationCallbacks* pAllocator);
//
//    VulkanBridge* vulkanContext = 0;
//private:
//    
//
//    // CORE FUNCTIONS
//
//    // Used by populateDebugMessengerCreateInfo()
//    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
//        VkDebugUtilsMessageTypeFlagsEXT messageType,
//        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
//        void* pUserData);
//    // Used by setupDebugMessenger()
//    VkResult CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
//        const VkAllocationCallbacks* pAllocator,
//        VkDebugUtilsMessengerEXT* pDebugMessenger);
//
//    // Used to destroy debugger
//    void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator);
//
//};