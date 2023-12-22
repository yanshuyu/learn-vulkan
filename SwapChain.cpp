#include"SwapChain.h"
#include<algorithm>
#include<iostream>
#include<numeric>
#include"QueueFamilyIndices.h"



void SwapChain::Init(VkPhysicalDevice phyDevice, VkDevice device, VkSurfaceKHR surface)
{
    if (phyDevice == VK_NULL_HANDLE 
        || device == VK_NULL_HANDLE
        || surface == VK_NULL_HANDLE)
        return;

    m_vkPhyDevice = phyDevice;
    m_vkSurface = surface;
    m_vkDevice = device;
    m_capabilities = {};
    m_SupportedFormats.clear();
    m_SupportedPresentModes.clear();

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phyDevice, surface, &m_capabilities);
    
    uint32_t supportedFmtCnt = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &supportedFmtCnt, nullptr);
    m_SupportedFormats.resize(supportedFmtCnt);
    vkGetPhysicalDeviceSurfaceFormatsKHR(phyDevice, surface, &supportedFmtCnt, m_SupportedFormats.data());

    uint32_t supportedPntModeCnt = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &supportedPntModeCnt, nullptr);
    m_SupportedPresentModes.resize(supportedPntModeCnt);
    vkGetPhysicalDeviceSurfacePresentModesKHR(phyDevice, surface, &supportedPntModeCnt, m_SupportedPresentModes.data());
}


bool SwapChain::Create( VkSurfaceFormatKHR prefferFmt,
                        VkPresentModeKHR prefferPresentMode,
                        int prefferQueuedImgCnt,
                        VkExtent2D prefferImageSz)
{
    if (!IsInited())
        return false;


    if (std::find_if(m_SupportedFormats.begin(), m_SupportedFormats.end(), [&](const VkSurfaceFormatKHR& fmt)
    {
        return fmt.format == prefferFmt.format && fmt.colorSpace == prefferFmt.colorSpace;

    }) == m_SupportedFormats.end())
    {
        std::cout << "-->Swap Chain Preffered Format is not supported!" << std::endl;
        prefferFmt = m_SupportedFormats[0];        
    }

    if (std::find(m_SupportedPresentModes.begin(), m_SupportedPresentModes.end(), prefferPresentMode) == m_SupportedPresentModes.end())
    {
        std::cout << "-->Swap Chain Preffered  Present mode is not supported!" << std::endl;
        prefferPresentMode = m_SupportedPresentModes[0]; 
    }

    //Vulkan tells us to match the resolution of the window by setting the width and height in the currentExtent membe
    //some window managers do allow us to differ here and this is indicated by setting the width and height in currentExtent to a special value: the maximum value of uint32_t
    //in this case, we want to set extent to match the window size in piexs by manully
    VkExtent2D imageExtent = m_capabilities.currentExtent;
    if (imageExtent.width == std::numeric_limits<uint32_t>::max()
        || imageExtent.height == std::numeric_limits<uint32_t>::max())
    {
        imageExtent = prefferImageSz;
    }
    imageExtent.width = std::clamp(imageExtent.width, m_capabilities.minImageExtent.width, m_capabilities.maxImageExtent.width);
    imageExtent.height = std::clamp(imageExtent.height, m_capabilities.minImageExtent.height, m_capabilities.maxImageExtent.height);
    
    prefferQueuedImgCnt = std::clamp((uint32_t)prefferQueuedImgCnt, m_capabilities.minImageCount, m_capabilities.maxImageCount);
    if (prefferQueuedImgCnt <= 1)
    {
        prefferQueuedImgCnt++; // double buffering minimul
        prefferQueuedImgCnt = std::clamp((uint32_t)prefferQueuedImgCnt, m_capabilities.minImageCount, m_capabilities.maxImageCount);
    }

    QueueFamilyIndices queueFamilyIndices;
    queueFamilyIndices.Query(m_vkPhyDevice);

    VkSwapchainCreateInfoKHR swapChainCrateInfo{};
    swapChainCrateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCrateInfo.flags = 0;
    swapChainCrateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapChainCrateInfo.pNext = nullptr;
    swapChainCrateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCrateInfo.clipped = VK_TRUE;
    swapChainCrateInfo.imageFormat = prefferFmt.format;
    swapChainCrateInfo.imageColorSpace = prefferFmt.colorSpace;
    swapChainCrateInfo.imageExtent = imageExtent;
    swapChainCrateInfo.minImageCount = prefferQueuedImgCnt;
    swapChainCrateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCrateInfo.imageArrayLayers = 1;
    if (queueFamilyIndices.GrapicQueueFamilyIndex() != queueFamilyIndices.PresentQueueFamilyIndex(m_vkSurface))
    {
        uint32_t sharingQueueFamilyIndices[2] =  {queueFamilyIndices.GrapicQueueFamilyIndex(), queueFamilyIndices.PresentQueueFamilyIndex(m_vkSurface)};
        swapChainCrateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // share between different queues, only one queue own the swap chain at a time
        swapChainCrateInfo.queueFamilyIndexCount = 2;
        swapChainCrateInfo.pQueueFamilyIndices = sharingQueueFamilyIndices;
    }
    else 
    {
        swapChainCrateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // swap chain owned by one queue only
        swapChainCrateInfo.queueFamilyIndexCount = 0;
        swapChainCrateInfo.pQueueFamilyIndices = nullptr;
    }
    swapChainCrateInfo.presentMode = prefferPresentMode;
    swapChainCrateInfo.surface = m_vkSurface;
    swapChainCrateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    bool ok = vkCreateSwapchainKHR(m_vkDevice, &swapChainCrateInfo, nullptr, &m_vkSwapChain) == VK_SUCCESS;
    
    
    return ok;
}



void SwapChain::Release()
{
    if (!IsCreated())
        return;
    
    vkDestroySwapchainKHR(m_vkDevice, m_vkSwapChain, nullptr);
    m_vkPhyDevice = VK_NULL_HANDLE;
    m_vkDevice = VK_NULL_HANDLE; 
    m_vkSurface = VK_NULL_HANDLE;
    m_vkSwapChain = VK_NULL_HANDLE;
    m_capabilities = {};
    m_SupportedFormats.clear();
    m_SupportedPresentModes.clear();
}