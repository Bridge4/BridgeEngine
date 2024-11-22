#pragma once
#include "ImageView.h"
#include "Initializer.h"

// Including Initializer for the SwapChainSupportDetails struct
class SwapChain
{
public: 
    void create(Initializer init, Window window);

    void createImageViews(VkDevice device, ImageView imgV);

    void assign(VkSwapchainKHR* swapChain, VkFormat* scFormat, VkExtent2D* scExtent, std::vector<VkImageView>* scImageViews);

    VkSwapchainKHR get() { return m_swapChain; }

    VkFormat getImageFormat() { return m_swapChainImageFormat; }
    
    VkExtent2D getExtent() { return m_swapChainExtent; }

    std::vector<VkImageView> getImageViews() { return m_swapChainImageViews; }

private:
    VkSwapchainKHR m_swapChain;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;

    // HELPERS
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

    VkExtent2D chooseSwapExtent(Window window, const VkSurfaceCapabilitiesKHR& capabilities);
};

