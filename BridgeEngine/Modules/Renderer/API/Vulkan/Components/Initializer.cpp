#include "Initializer.h"
#include "Window.h"
#include <iostream>
#include <array>
#include <set>
#include <cstdint> // Necessary for uint32_t
// CORE FUNCTIONS
#include "Window.h"
#include "../VulkanBridge.h"

void Initializer::init()
{
    //createVulkanInstance();
    //createDebugMessenger();
    /*if (windowRef->createSurface() != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");*/
        /*pickPhysicalDevice();
    createLogicalDevice();*/
}

//void Initializer::assign(VkSurfaceKHR* rSurface, VkPhysicalDevice* pDevice, VkDevice* device, VkQueue* gQueue, VkQueue* pQueue)
//{
//    *rSurface = surface();
//    *pDevice = physDevice();
//    *device = logDevice();
//    *gQueue = graphicsQueue();
//    *pQueue = presentQueue();
//}


void Initializer::createVulkanInstance() {
    // A lot of information is passed through structs rather than function parameters

    //if (enableValidationLayers && !checkValidationLayerSupport()) {
    //    throw std::runtime_error("validation layers requested but not available...");
    //}

    //// optional but helpful information for the driver
    //VkApplicationInfo appInfo{};
    //appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    //appInfo.pApplicationName = "Bridge Engine";
    //appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    //appInfo.pEngineName = "Bridge Engine";
    //appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    //appInfo.apiVersion = VK_API_VERSION_1_3;

    //// MANDATORY for instance creation
    //// Telling Vulkan what global (program) extensions and validations we want to use
    //VkInstanceCreateInfo createInfo{};
    //createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;


    //// ADDING appInfo to createInfo
    //createInfo.pApplicationInfo = &appInfo;

    //
    //// Vulkan is platform agnostic. Here we provide the GLFW extension to interface with the window system
    //std::vector<const char*> extensions = vulkanContext->getRequiredExtensions();
    //createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    //createInfo.ppEnabledExtensionNames = extensions.data();

    //VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    //if (enableValidationLayers) {
    //    createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
    //    createInfo.ppEnabledLayerNames = validationLayers.data();
    //    //populateDebugMessengerCreateInfo(debugCreateInfo);
    //    debugCreateInfo = {};
    //    debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    //    debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    //    debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    //    debugCreateInfo.pfnUserCallback = debugCallback;
    //    debugCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    //}
    //else {
    //    createInfo.enabledLayerCount = 0;
    //    createInfo.pNext = nullptr;
    //}

    //if (vkCreateInstance(&createInfo, nullptr, &vulkanContext->m_instance) != VK_SUCCESS) {
    //    throw std::runtime_error("failed to create instance!");
    //}
}

void Initializer::createDebugMessenger() {
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

void Initializer::createSurface() {
    windowRef->createSurface();
}

void Initializer::pickPhysicalDevice() {
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
            vulkanContext->m_physicalDevice = device;
            break;
        }
    }

    if (vulkanContext->m_physicalDevice == VK_NULL_HANDLE) {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void Initializer::createLogicalDevice(VkPhysicalDevice physicalDevice) {
    // Specifying queues to be created
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
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

    if (vkCreateDevice(vulkanContext->m_physicalDevice, &createInfo, nullptr, &vulkanContext->m_logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    vkGetDeviceQueue(vulkanContext->m_logicalDevice, indices.graphicsFamily.value(), 0, &vulkanContext->m_graphicsQueue);
    vkGetDeviceQueue(vulkanContext->m_logicalDevice, indices.presentFamily.value(), 0, &vulkanContext->m_presentQueue);

}

void Initializer::destroy() {
    vkDestroyDevice(vulkanContext->m_logicalDevice, nullptr);
    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(nullptr);
    }
    vkDestroySurfaceKHR(vulkanContext->m_instance, r_surface, nullptr);
    vkDestroyInstance(vulkanContext->m_instance, nullptr);
}
// HELPERS

// createInstance()
bool Initializer::checkValidationLayerSupport() {

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

//std::vector<const char*> Initializer::getRequiredExtensions() {
//    uint32_t glfwExtensionCount = 0;
//    const char** glfwExtensions;
//    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
//
//    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
//
//    if (enableValidationLayers) {
//        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
//    }
//
//    return extensions;
//}

// pickPhysicalDevice()
bool Initializer::isDeviceSuitable(VkPhysicalDevice device) {
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
bool Initializer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
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

SwapChainSupportDetails Initializer::querySwapChainSupport(VkPhysicalDevice physicalDevice) {

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
QueueFamilyIndices Initializer::findQueueFamilies(VkPhysicalDevice physicalDevice) {
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


VKAPI_ATTR VkBool32 VKAPI_CALL Initializer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}

// setupDebugMessenger()
VkResult Initializer::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->m_instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(vulkanContext->m_instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Initializer::DestroyDebugUtilsMessengerEXT(const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->m_instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(vulkanContext->m_instance, vulkanContext->m_debugMessenger, pAllocator);
    }
}
