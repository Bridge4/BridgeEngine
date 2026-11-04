#pragma once
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif  // !VK_USE_PLATFORM_WIN32_KHR

#include "../ComponentDeclarations.h"
// class WindowHandler;
// class VulkanContext;

namespace backend_vk::devices {

const std::vector<const char*> kValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> kDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// Used by isDeviceSuitable() and createLogicalDevice()
QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device,
                                       VkSurfaceKHR window_surface);

VkPhysicalDevice init_physical_device(VkInstance& instance);

VkDevice init_logical_device();

void init_debug_messenger();

bool check_validation_layer_support();

bool is_device_suitable(VkPhysicalDevice physical_device,
                        VkSurfaceKHR window_surface);

bool check_device_extension_support(VkPhysicalDevice device);

SwapChainSupportDetails query_swap_chain_support(
    VkPhysicalDevice physical_device, VkSurfaceKHR window_surface);

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

void destroy_debug_utils_messenger_ext(const VkAllocationCallbacks* pAllocator);
}  // namespace backend_vk::devices

/*
 *   Provides Physical and Logical Devices for use in Vulkan Context
 *   This class also sets up validation layers and the debug messenger.Will
 * extract that out of the class in the future but for now prioritizing other
 * tasks
 */
class DeviceHandler {
   public:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    DeviceHandler(VulkanGlobalState* VulkanGlobalState) {
        this->m_vulkanGlobalState = VulkanGlobalState;
    }

    void init();

    void Destroy();

    SwapChainSupportDetails QuerySwapChainSupport(
        VkPhysicalDevice physicalDevice);

    // Used by isDeviceSuitable() and createLogicalDevice()
    QueueFamilyIndices find_queue_families(VkPhysicalDevice physicalDevice);

    VkPhysicalDevice m_physical_device = nullptr;
    VkDevice m_logical_device = nullptr;

   private:
    // CONSTANTS
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    VulkanGlobalState* m_vulkanGlobalState = 0;
    void InitializeLogicalDevice();
    void InitializeDebugMessenger();
    // CORE FUNCTIONS

    // HELPER FUNCTIONS
    // Used by createInstance()
    bool CheckValidationLayerSupport();
    // std::vector<const char*> getRequiredExtensions();

    // Used by pickPhysicalDevice()
    bool IsDeviceSuitable(VkPhysicalDevice device);

    // Used by isDeviceSuitable()
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);

    // Used by populateDebugMessengerCreateInfo()
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void* pUserData);
    // Used by setupDebugMessenger()
    VkResult CreateDebugUtilsMessengerEXT(
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger);

    // Used to destroy debugger
    void DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator);
};
