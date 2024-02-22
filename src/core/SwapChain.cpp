#include"SwapChain.h"
#include<algorithm>
#include<iostream>
#include<numeric>
#include"core\Device.h"
#include"core\CoreUtils.h"
#include"core\QueueFamilyIndices.h"
#include"rendering\Window.h"

bool SwapChain::Create(Device* pDevice, Window* pWindow, const SwapChainDesc& desc)
{
    if (pWindow == nullptr)
    {   
        LOGE("SwapChain Create Error: Window({}) is null!", (void*)pWindow);
        return false;
    }

    if (pDevice == nullptr || !pDevice->IsValid())
    {   
        LOGE("SwapChain Create Error: device({}) is null or invalid!", (void*)pDevice);
        return false;
    }

    if (!pDevice->SupportPrenset(pWindow))
    {   
        LOGE("SwapChain Create Error: device({}) is not able present!", (void*)pDevice);
        return false;
    }

   
    auto supportedFmts = pDevice->GetSupportedPresentFormats(pWindow);
    auto preferredFmt = std::find_if(supportedFmts.begin(), supportedFmts.end(), [&](const VkSurfaceFormatKHR &fmt)
                                     { return fmt.format == desc.format && fmt.colorSpace == desc.colorSpace; });
    VkSurfaceFormatKHR surfaceFmt = preferredFmt ==  supportedFmts.end() ? supportedFmts[0] : *preferredFmt;
    

    VkPresentModeKHR bestPresentModes[] = {VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_FIFO_RELAXED_KHR,VK_PRESENT_MODE_IMMEDIATE_KHR};
    auto supportedPresentModes = pDevice->GetSupportedPresentModes(pWindow);
    VkPresentModeKHR presentMode = supportedPresentModes[0];
    size_t startIndex = desc.enableVSync ? 0 : 2;
    const size_t modeCount = 2;
    bool foundBestMode = false;
    for (size_t i = 0; i < modeCount; i++)
    {
        for (size_t j = 0; j < supportedPresentModes.size(); j++)
        {
            if (supportedPresentModes[j] == bestPresentModes[startIndex + i])
            {
                presentMode = bestPresentModes[startIndex + i];
                foundBestMode = true;
                break;
            }
        }
        
        if (foundBestMode)
            break;
    }

    VkSurfaceCapabilitiesKHR surCaps = pDevice->GetSurfaceCapabilities(pWindow);

    //Vulkan tells us to match the resolution of the window by setting the width and height in the currentExtent membe
    //some window managers do allow us to differ here and this is indicated by setting the width and height in currentExtent to a special value: the maximum value of uint32_t
    //in this case, we want to set extent to match the window size in piexs by manully
    VkExtent2D imageExtent = surCaps.currentExtent;
    if (imageExtent.width == std::numeric_limits<uint32_t>::max()
        || imageExtent.height == std::numeric_limits<uint32_t>::max())
    {
        imageExtent = desc.bufferExtend;
    }
    imageExtent.width = std::clamp(imageExtent.width, surCaps.minImageExtent.width, surCaps.maxImageExtent.width);
    imageExtent.height = std::clamp(imageExtent.height, surCaps.minImageExtent.height, surCaps.maxImageExtent.height);
    
    
    size_t bufCount = std::clamp((uint32_t)desc.bufferCount, surCaps.minImageCount, surCaps.maxImageCount);
    while (bufCount <= 1)
    {
        bufCount++; 
    }
    // double buffering minimul
    bufCount = std::clamp((uint32_t)desc.bufferCount, surCaps.minImageCount, surCaps.maxImageCount);

    VkSwapchainCreateInfoKHR swapChainCrateInfo{};
    swapChainCrateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCrateInfo.flags = 0;
    swapChainCrateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapChainCrateInfo.pNext = nullptr;
    swapChainCrateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapChainCrateInfo.clipped = VK_TRUE;
    swapChainCrateInfo.imageFormat = surfaceFmt.format;
    swapChainCrateInfo.imageColorSpace = surfaceFmt.colorSpace;
    swapChainCrateInfo.imageExtent = imageExtent;
    swapChainCrateInfo.minImageCount = (uint32_t)bufCount;
    swapChainCrateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapChainCrateInfo.imageArrayLayers = 1;

    QueueFamilyIndices deviceQueFamIndices(pDevice->GetHardwardHandle());
    uint32_t queueFamIdxs[2] = { deviceQueFamIndices.GrapicQueueFamilyIndex(), deviceQueFamIndices.PresentQueueFamilyIndex(pWindow->GetVulkanSurface())};
    if ( queueFamIdxs[0] != queueFamIdxs[1])
    {
        swapChainCrateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; // share between different queues, only one queue own the swap chain at a time
        swapChainCrateInfo.queueFamilyIndexCount = 2;
        swapChainCrateInfo.pQueueFamilyIndices = queueFamIdxs;
    }
    else 
    {
        swapChainCrateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE; // swap chain owned by one queue only
        swapChainCrateInfo.queueFamilyIndexCount = 0;
        swapChainCrateInfo.pQueueFamilyIndices = nullptr;
    }
    swapChainCrateInfo.presentMode = presentMode;
    swapChainCrateInfo.surface = pWindow->GetVulkanSurface();
    swapChainCrateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;

    VkSwapchainKHR createdSwapChain{VK_NULL_HANDLE};
    VkResult result = vkCreateSwapchainKHR(pDevice->GetHandle(), &swapChainCrateInfo, nullptr, &createdSwapChain); 
    if (result != VK_SUCCESS)
    {
        LOGE("Device({}) create swapchain error: {}", (void*)pDevice, result);
        return false;
    }

    m_vkSwapChain = createdSwapChain;
    m_SwapChainPxlFmt = surfaceFmt;
    m_SwapChainPxlDimension = imageExtent;
    m_SwapChainPresentMode = presentMode;
    uint32_t imageCnt = 0;
    vkGetSwapchainImagesKHR(pDevice->GetHandle(), m_vkSwapChain, &imageCnt, nullptr);
    m_SwapChainImages.resize(imageCnt);
    vkGetSwapchainImagesKHR(pDevice->GetHandle(), m_vkSwapChain, &imageCnt, m_SwapChainImages.data());

    m_SwapChainImageViews.resize(m_SwapChainImages.size());
    for (size_t i = 0; i < m_SwapChainImages.size(); i++)
    {
        VkImageViewCreateInfo viewCreateInfo{};
        viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewCreateInfo.flags = 0;
        viewCreateInfo.pNext = nullptr;
        viewCreateInfo.image = m_SwapChainImages[i];
        viewCreateInfo.format = m_SwapChainPxlFmt.format;
        viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewCreateInfo.components.r = viewCreateInfo.components.g = viewCreateInfo.components.b = viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewCreateInfo.subresourceRange.baseMipLevel = 0;
        viewCreateInfo.subresourceRange.levelCount = 1;
        viewCreateInfo.subresourceRange.baseArrayLayer = 0;
        viewCreateInfo.subresourceRange.layerCount = 1;
        vkCreateImageView(pDevice->GetHandle(), &viewCreateInfo, nullptr, &m_SwapChainImageViews[i]);
    }
}



void SwapChain::Release()
{
    if (!IsValid())
        return;

    for (auto && imageView : m_SwapChainImageViews)
    {
        if (imageView != VK_NULL_HANDLE)
            vkDestroyImageView(m_pDevice->GetHandle(), imageView, nullptr);
    }
    
    vkDestroySwapchainKHR(m_pDevice->GetHandle(), m_vkSwapChain, nullptr);
    m_pDevice = nullptr;
    m_vkSwapChain = VK_NULL_HANDLE;
    m_SwapChainImages.clear();
    m_SwapChainImageViews.clear();
    m_SwapChainPxlDimension = {0, 0};
    m_SwapChainPxlFmt = {};
}