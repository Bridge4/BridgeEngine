#pragma once
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/RendererTypes.h"
#ifndef VK_USE_PLATFORM_WIN32_KHR
#define VK_USE_PLATFORM_WIN32_KHR
#endif  // !VK_USE_PLATFORM_WIN32_KHR

#include "../ComponentDeclarations.h"
// class WindowHandler;
// class VulkanContext;

namespace backend_vk::devices {

#ifdef NDEBUG
const bool kEnableValidationLayers = false;
#else
const bool kEnableValidationLayers = true;
#endif
const std::vector<const char*> kValidationLayers = {
    "VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> kDeviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// Used by isDeviceSuitable() and createLogicalDevice()
QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device,
                                       VkSurfaceKHR window_surface);

VkPhysicalDevice init_physical_device(VkInstance& instance,
                                      VkSurfaceKHR window_surface);

VkDevice init_logical_device(VkPhysicalDevice& physical_device,
                             VkSurfaceKHR window_surface,
                             ERenderingMode rendering_mode);

std::optional<VkDebugUtilsMessengerEXT> init_debug_messenger(
    VkInstance instance);

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

VkResult create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger);

void destroy_debug_utils_messenger_ext(
    VkInstance& instance, VkDebugUtilsMessengerEXT& debug_messenger,
    const VkAllocationCallbacks* pAllocator);

VkQueue get_device_queue(VkDevice& logical_device, uint32_t queue_family_index);

void destroy_devices(VkPhysicalDevice& physical_device,
                     VkDevice& logical_device, VkSurfaceKHR window_surface,
                     VkInstance& instance,
                     VkDebugUtilsMessengerEXT& debug_messenger);
}  // namespace backend_vk::devices

/*
 *   Provides Physical and Logical Devices for use in Vulkan Context
 *   This class also sets up validation layers and the debug messenger.Will
 * extract that out of the class in the future but for now prioritizing other
 * tasks
 */
