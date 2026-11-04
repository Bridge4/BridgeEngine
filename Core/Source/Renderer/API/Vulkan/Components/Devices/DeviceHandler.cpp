#include "DeviceHandler.h"

#include <vulkan/vulkan_core.h>

#include <cstdint>  // Necessary for uint32_t
#include <iostream>
#include <optional>
#include <set>

#include "../VulkanGlobalState/VulkanGlobalState.h"
// m_vulkan_global_state FUNCTIONS
#include "../../VulkanContext.h"
#include "Source/Renderer/API/Vulkan/VkTypes.h"
#include "Source/Renderer/RendererTypes.h"

namespace backend_vk::devices {

VkPhysicalDevice init_physical_device(VkInstance& instance,
                                      VkSurfaceKHR window_surface) {
    VkPhysicalDevice physical_device;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    // check if there are no compatible devices (GPUs)
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support.");
    }

    // Allocate array to hold all VKPhysicalDevice handles
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    // check if there are no compatible devices (GPUs), &deviceCount,
    // devices.data());

    for (const auto& device : devices) {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(device, &properties);
        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            physical_device = device;

        } else {
            if (is_device_suitable(device, window_surface)) {
                physical_device = device;
                break;
            }
        }
    }

    if (physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    return physical_device;
}

bool is_device_suitable(VkPhysicalDevice physical_device,
                        VkSurfaceKHR window_surface) {
    // Fetching deviceProperties and deviceFeatures in case we want to check for
    // specifics
    /*
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    */
    QueueFamilyIndices indices =
        find_queue_families(physical_device, window_surface);

    bool extensionsSupported = check_device_extension_support(physical_device);

    bool swapChainAdequate = false;

    // if the extensions required are supported then we can check for swap chain
    // compatibility otherwise there's no point
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport =
            query_swap_chain_support(physical_device, window_surface);
        swapChainAdequate = !swapChainSupport.formats.empty() &&
                            !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(physical_device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate &&
           supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device,
                                       VkSurfaceKHR window_surface) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount,
                                             nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queueFamilyCount,
                                             queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphics_family = i;
        }

        if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) {
            indices.compute_family = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, window_surface,
                                             &presentSupport);

        if (presentSupport) {
            indices.present_family = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

bool check_device_extension_support(VkPhysicalDevice physical_device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr,
                                         &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(
        physical_device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(kDeviceExtensions.begin(),
                                             kDeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails query_swap_chain_support(
    VkPhysicalDevice physical_device, VkSurfaceKHR window_surface) {
    // Fetching the capabilities of the device and surface

    // Getting supported surface formats
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, window_surface,
                                              &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, window_surface,
                                         &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, window_surface,
                                             &formatCount,
                                             details.formats.data());
    }

    // Getting supported present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, window_surface,
                                              &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(
            physical_device, window_surface, &presentModeCount,
            details.presentModes.data());
    }

    return details;
}

VkDevice init_logical_device(VkPhysicalDevice& physical_device,
                             VkSurfaceKHR window_surface,
                             ERenderingMode rendering_mode) {
    VkDevice logical_device;
    // Specifying queues to be created
    QueueFamilyIndices indices =
        find_queue_families(physical_device, window_surface);
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphics_family.value();
    queueCreateInfo.queueCount = 1;

    // Queue Priority
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphics_family.value(),
                                              indices.present_family.value()};
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }
    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    VkPhysicalDeviceVulkan12Features vulkan12Features{
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES};
    if (rendering_mode == ERenderingMode::Bindless) {
        vulkan12Features.descriptorIndexing = VK_TRUE;
        vulkan12Features.descriptorIndexing = VK_TRUE;
        vulkan12Features.runtimeDescriptorArray = VK_TRUE;
        vulkan12Features.shaderSampledImageArrayNonUniformIndexing = VK_TRUE;
        vulkan12Features.descriptorBindingPartiallyBound = VK_TRUE;
        vulkan12Features.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE;
        vulkan12Features.descriptorBindingUpdateUnusedWhilePending = VK_TRUE;
    }

    // Creating logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.pNext = &vulkan12Features;
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount =
        static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    // COMPATIBILITY WITH OLDER VULKAN
    createInfo.enabledExtensionCount =
        static_cast<uint32_t>(kDeviceExtensions.size());
    createInfo.ppEnabledExtensionNames = kDeviceExtensions.data();
    if (kEnableValidationLayers) {
        createInfo.enabledLayerCount =
            static_cast<uint32_t>(kValidationLayers.size());
        createInfo.ppEnabledLayerNames = kValidationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physical_device, &createInfo, nullptr,
                       &logical_device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }
    return logical_device;
}

VkQueue get_device_queue(VkDevice& logical_device,
                         uint32_t queue_family_index) {
    VkQueue queue;
    vkGetDeviceQueue(logical_device, queue_family_index, 0, &queue);
    return queue;
}

std::optional<VkDebugUtilsMessengerEXT> init_debug_messenger(
    VkInstance instance) {
    if (!kEnableValidationLayers) return nullptr;
    VkDebugUtilsMessengerEXT debug_messenger;
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debug_callback;

    if (create_debug_utils_messenger_ext(instance, &createInfo, nullptr,
                                         &debug_messenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
    return debug_messenger;
}

VkResult create_debug_utils_messenger_ext(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* p_create_info,
    const VkAllocationCallbacks* p_allocator,
    VkDebugUtilsMessengerEXT* p_debug_messenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, p_create_info, p_allocator, p_debug_messenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
               VkDebugUtilsMessageTypeFlagsEXT messageType,
               const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
               void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

void destroy_devices(VkPhysicalDevice& physical_device,
                     VkDevice& logical_device, VkSurfaceKHR window_surface,
                     VkInstance& instance,
                     VkDebugUtilsMessengerEXT& debug_messenger) {
    vkDestroyDevice(logical_device, nullptr);
    if (kEnableValidationLayers) {
        destroy_debug_utils_messenger_ext(instance, debug_messenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, window_surface, nullptr);
    vkDestroyInstance(instance, nullptr);
}

void destroy_debug_utils_messenger_ext(
    VkInstance& instance, VkDebugUtilsMessengerEXT& debug_messenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
        instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debug_messenger, pAllocator);
    }
}

}  // namespace backend_vk::devices
