#include "DeviceHandler.h"
#include "../Window/WindowHandler.h"
#include "../VulkanInstanceManager/VulkanInstanceManager.h"
#include <iostream>
#include <array>
#include <set>
#include <cstdint> // Necessary for uint32_t
// CORE FUNCTIONS
//#include "WindowHandler.h"
#include "../../VulkanContext.h"


void DeviceHandler::Initialize() {
    InitializePhysicalDevice();
    InitializeLogicalDevice();
    InitializeDebugMessenger();
}

void DeviceHandler::InitializeDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;

    if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr, &vulkanContext->m_debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("failed to set up debug messenger!");
    }
}

void DeviceHandler::InitializePhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(vulkanContext->m_instance, &deviceCount, nullptr);
    // check if there are no compatible devices (GPUs)
    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support.");
    }

    // Allocate array to hold all VKPhysicalDevice handles
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(vulkanContext->m_instance, &deviceCount, devices.data());


    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            m_vulkanInstanceManager->SetPhysicalDevice(device);
            break;
        }
    }

    if (m_vulkanInstanceManager->GetPhysicalDevice() == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void DeviceHandler::InitializeLogicalDevice() {
    // Specifying queues to be created
    QueueFamilyIndices indices = findQueueFamilies(m_vulkanInstanceManager->GetPhysicalDevice());
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;

    // Queue Priority
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };
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

    // Creating logical device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    // COMPATIBILITY WITH OLDER VULKAN
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(m_vulkanInstanceManager->GetPhysicalDevice(), &createInfo, nullptr, m_vulkanInstanceManager->GetRefLogicalDevice()) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(*m_vulkanInstanceManager->GetRefLogicalDevice(), indices.graphicsFamily.value(), 0, &vulkanContext->m_graphicsQueue);
    vkGetDeviceQueue(*m_vulkanInstanceManager->GetRefLogicalDevice(), indices.presentFamily.value(), 0, &vulkanContext->m_presentQueue);

}

void DeviceHandler::Destroy() {
    vkDestroyDevice(LogicalDevice, nullptr);
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(nullptr);
    }
    vkDestroySurfaceKHR(vulkanContext->m_instance, vulkanContext->m_surface, nullptr);
    vkDestroyInstance(vulkanContext->m_instance, nullptr);
}
// HELPERS

// createInstance()
bool DeviceHandler::checkValidationLayerSupport() {

    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char* layerName : vulkanContext->validationLayers) {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers) {
            if (strcmp(layerName, layerProperties.layerName) == 0) {
                layerFound = true;
                break;
            }
        }

        if (!layerFound) {
            return false;
        }
    }

    return true;
}

// pickPhysicalDevice()
bool DeviceHandler::isDeviceSuitable(VkPhysicalDevice device) {
    // Fetching deviceProperties and deviceFeatures in case we want to check for specifics
    /*
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    */
    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;

    // if the extensions required are supported then we can check for swap chain compatibility
    // otherwise there's no point
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

// isDeviceSuitable()
bool DeviceHandler::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());


    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

SwapChainSupportDetails DeviceHandler::querySwapChainSupport(VkPhysicalDevice physicalDevice) {

    // Fetching the capabilities of the device and surface


    // Getting supported surface formats
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, vulkanContext->m_surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanContext->m_surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, vulkanContext->m_surface, &formatCount, details.formats.data());
    }

    // Getting supported present modes
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkanContext->m_surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, vulkanContext->m_surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

// isDeviceSuitable() + createLogicalDevice()
QueueFamilyIndices DeviceHandler::findQueueFamilies(VkPhysicalDevice physicalDevice) {
    QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, vulkanContext->m_surface, &presentSupport);

        if (presentSupport) {
            indices.presentFamily = i;
        }
        if (indices.isComplete()) {
            break;
        }
        i++;
    }
    return indices;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DeviceHandler::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

// setupDebugMessenger()
VkResult DeviceHandler::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->m_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(vulkanContext->m_instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DeviceHandler::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->m_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(vulkanContext->m_instance, vulkanContext->m_debugMessenger, pAllocator);
    }
}
