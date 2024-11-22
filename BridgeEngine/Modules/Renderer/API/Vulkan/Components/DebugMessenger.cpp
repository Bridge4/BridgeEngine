//#include "DebugMessenger.h"
//#include "../VulkanBridge.h"
//#include <iostream>
//
//// **** WAS IN THE MIDDLE OF SEPARATING OUT THE DEBUG MESSENGER FROM THE VULKAN BRIDGE 
//void DebugMessenger::Create() {
//    if (!enableValidationLayers) return;
//
//    VkDebugUtilsMessengerCreateInfoEXT createInfo;
//    createInfo = {};
//    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
//    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
//    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
//    createInfo.pfnUserCallback = debugCallback;
//
//    if (CreateDebugUtilsMessengerEXT(&createInfo, nullptr, &vulkanContext->m_debugMessenger) != VK_SUCCESS) {
//        throw std::runtime_error("failed to set up debug messenger!");
//    }
//}
//
//VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessenger::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
//    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
//
//    return VK_FALSE;
//}
//
//// setupDebugMessenger()
//VkResult DebugMessenger::CreateDebugUtilsMessengerEXT(const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
//    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->m_instance, "vkCreateDebugUtilsMessengerEXT");
//    if (func != nullptr) {
//        return func(vulkanContext->m_instance, pCreateInfo, pAllocator, pDebugMessenger);
//    }
//    else {
//        return VK_ERROR_EXTENSION_NOT_PRESENT;
//    }
//}
//
//void DebugMessenger::Destroy(const VkAllocationCallbacks* pAllocator) {
//    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(vulkanContext->m_instance, "vkDestroyDebugUtilsMessengerEXT");
//    if (func != nullptr) {
//        func(vulkanContext->m_instance, vulkanContext->m_debugMessenger, pAllocator);
//    }
//}