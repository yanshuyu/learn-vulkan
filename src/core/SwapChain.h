#pragma once
#include<vulkan\vulkan.h>
#include<vector>

using std::vector;

struct SwapChain
{
private:
    VkSurfaceCapabilitiesKHR m_capabilities{};
    vector<VkSurfaceFormatKHR> m_SupportedFormats{};
    vector<VkPresentModeKHR> m_SupportedPresentModes{};
    VkSurfaceKHR m_vkSurface{VK_NULL_HANDLE};
    VkPhysicalDevice m_vkPhyDevice{VK_NULL_HANDLE};
    VkDevice m_vkDevice{VK_NULL_HANDLE};
    VkSwapchainKHR m_vkSwapChain{VK_NULL_HANDLE};

    vector<VkImage> m_SwapChainImages{};
    vector<VkImageView> m_SwapChainImageViews{};
    VkSurfaceFormatKHR m_SwapChainPxlFmt{};
    VkExtent2D m_SwapChainPxlDimension{};
    

public:
    void Init(VkPhysicalDevice phyDevice, VkDevice device, VkSurfaceKHR surface);
    bool Create(VkSurfaceFormatKHR prefferFmt,
                VkPresentModeKHR prefferPresentMode,
                int prefferQueuedImgCnt,
                VkExtent2D prefferImageSz);
    void Release();

    bool IsInited() const
    {
        return m_vkSurface != VK_NULL_HANDLE
                && m_vkPhyDevice != VK_NULL_HANDLE 
                && m_SupportedFormats.size() > 0 
                && m_SupportedPresentModes.size() > 0;
    }

    bool IsCreated() const 
    {
        return m_vkSwapChain != VK_NULL_HANDLE;
    }
};
