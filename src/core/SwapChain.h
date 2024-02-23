#pragma once
#include<vulkan\vulkan.h>
#include<vector>

class Device;
class Window;

struct SwapChainDesc
{
    VkExtent2D bufferExtend;
    VkFormat format;
    VkColorSpaceKHR colorSpace;
    bool enableTripleBuffering;
    bool enableVSync;
};



class SwapChain
{
private:
    Device* m_pDevice{nullptr};
    Window* m_Window{nullptr};

    VkSwapchainKHR m_vkSwapChain{VK_NULL_HANDLE};
    VkSurfaceFormatKHR m_SwapChainPxlFmt{};
    VkExtent2D m_SwapChainPxlDimension{};
    VkPresentModeKHR m_SwapChainPresentMode{};

    std::vector<VkImage> m_SwapChainImages{};
    std::vector<VkImageView> m_SwapChainImageViews{};
    

public:
    SwapChain() {};
    ~SwapChain() { Release(); }

    bool Create(Device* device, Window* window, const SwapChainDesc& desc);
    VkSwapchainKHR GetHandle() const { return m_vkSwapChain; }
    size_t GetBufferCount() const { return m_SwapChainImages.size(); }
    VkImageView GetBufferView(int idx) const { return m_SwapChainImageViews[idx]; }
    VkExtent2D GetBufferSize() const { return m_SwapChainPxlDimension; }
    VkSurfaceFormatKHR GetBufferFormat() const { return m_SwapChainPxlFmt; }
    VkPresentModeKHR getPresentMode() const { return m_SwapChainPresentMode; }

    bool AquireNextBuffer(VkSemaphore s, uint32_t* imageIdx, uint64_t timeOut = std::numeric_limits<uint64_t>::max()) const;
    bool AquireNextBuffer(VkFence f, uint32_t* imageIdx, uint64_t timeOut = std::numeric_limits<uint64_t>::max()) const;

    bool IsValid() const { return m_pDevice != nullptr && m_Window != nullptr && m_vkSwapChain != VK_NULL_HANDLE; }
    void Release();
};
