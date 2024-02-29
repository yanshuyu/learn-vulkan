#pragma once 
#include<vulkan\vulkan.h>
#include<string>
#include<vector>
#include<utility>
#include"core\CoreUtils.h"
#include"rendering\Window.h"
#include"core\QueueFamilyIndices.h"
#include"rendering\ObjectPool.h"
#include"core\CommandBuffer.h"
#include"core\Buffer.h"
#include"core\Image.h"
#include"core\Fence.h"


class Device
{
private:
    VkPhysicalDevice m_vkPhyDevice{VK_NULL_HANDLE};
    QueueFamilyIndices m_DeviceQueueFamilyIndices{};
    VkPhysicalDeviceProperties m_PhyDeviceProps{};
    VkPhysicalDeviceFeatures m_PhyDeviceFeatures{};
    VkPhysicalDeviceMemoryProperties m_PhyDeviceMemProps{};
    
    
    std::vector<std::string> m_DeviceExtendsions{};
    std::vector<DeviceFeatures> m_EnablePhyDeviceFeatures{};

    VkDevice m_vkDevice{VK_NULL_HANDLE};
    VkQueue m_DeviceQueues[QueueFamilyIndices::MAX_INDEX]; 
    VkCommandPool m_DeviceQueueCmdPools[QueueFamilyIndices::MAX_INDEX * 2];
    //VkCommandBuffer m_DeviceQueueCmdBuffers[QUEUE_FAMILY_MAX_COUNT];

private:
    std::vector<Fence*> _Fences{};

    ObjectPool<CommandBuffer> _CmdBufPool{};
    ObjectPool<Buffer> _BufferPool{};
    ObjectPool<Image> _ImagePool{};

public:
    Device();
    ~Device() { Release(); };

    NONE_COPYABLE_NONE_MOVEABLE(Device)
    
    void SetDeviceFeatureHint(DeviceFeatures feature, bool enabled);
    void SetDeviceExtendsionHint(const char* extendsionName, bool enabled) { vkutils_toggle_extendsion_or_layer_name_active(m_DeviceExtendsions, extendsionName, enabled); }
    void ResetAllHints();

    bool Initailze(VkPhysicalDevice phyDevice);
    bool IsValid() const { return m_vkDevice != VK_NULL_HANDLE; }
    void Release();
    void WaitIdle() const;

    VkDevice GetHandle() const { return m_vkDevice; }
    VkPhysicalDevice GetHardwardHandle() const { return m_vkPhyDevice; }

    bool SupportGrapic() const { return VKHANDLE_IS_NOT_NULL(m_DeviceQueues[QueueFamilyIndices::GRAPICS_INDEX]); }
    bool SupportCompute() const { return VKHANDLE_IS_NOT_NULL(m_DeviceQueues[QueueFamilyIndices::COMPUTE_INDEX]); }
    bool SupportTransfer() const { return VKHANDLE_IS_NOT_NULL(m_DeviceQueues[QueueFamilyIndices::GRAPICS_INDEX]) || VKHANDLE_IS_NOT_NULL(m_DeviceQueues[QueueFamilyIndices::TRANSFER_INDEX]); } // grapics queue must support transfer
    bool SupportPrenset(Window* window) const { return VKHANDLE_IS_NOT_NULL(GetPresentQueue(window)); }
    
    VkQueue GetGrapicQueue() const { return m_DeviceQueues[QueueFamilyIndices::GRAPICS_INDEX]; }
    VkQueue GetcomputeQueue() const { return m_DeviceQueues[QueueFamilyIndices::COMPUTE_INDEX]; }
    VkQueue GetTransferQueue() const { return SupportGrapic() ? GetGrapicQueue() : m_DeviceQueues[QueueFamilyIndices::TRANSFER_INDEX]; }
    VkQueue GetPresentQueue(const Window* window) const;

    std::vector<VkSurfaceFormatKHR> GetSupportedPresentFormats(Window* window) const;
    std::vector<VkPresentModeKHR> GetSupportedPresentModes(Window* window) const;
    bool GetSurfaceCapabilities(Window* window, VkSurfaceCapabilitiesKHR* pResult) const;

    CommandBuffer* CreateCommandBuffer(VkQueue queue) { return CreateCommandBufferImp(queue, false); }
    CommandBuffer* CreateTempraryCommandBuffer(VkQueue queue) { return CreateCommandBufferImp(queue, true); }
    bool DestroyCommandBuffer(CommandBuffer* pCmdBuf);

    // Features & Limits
    bool IsFeatureSupport(DeviceFeatures feature) const;
    bool IsFeatureEnabled(DeviceFeatures feature) const;
    uint32_t GetDeviceLimit(DeviceLimits limit) const;

    // Memory Alloc & Free
    bool AllocMemory(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags memPropFlag, VkDeviceMemory* pAllocatedMem);
    void FreeMemory(VkDeviceMemory mem);

    // Resource Create & Destroy
    
    Buffer* CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp);
    void DestroyBuffer(Buffer* pBuffer);

    Image *CreateImage(VkFormat fmt,
                       uint32_t width,
                       uint32_t height,
                       uint32_t depth,
                       VkImageUsageFlags usage,
                       VkMemoryPropertyFlags memProp,
                       bool genMipMaps = false,
                       uint32_t layerCnt = 1,
                       uint32_t sampleCnt = 1,
                       bool linearTiling = false);
    void DestroyImage(Image* pImage);

    Fence* CreateFence(bool signaled);
    bool DestroyFence(Fence* pFence);
    
private:
    bool CreateLogicalDevice(VkPhysicalDevice phyDevice);
    bool CreateCommandPools();
    //bool CreateCommandBuffers();
    void QueryDeviceMemoryProperties();
    //void QueryDeviceSurfaceProperties();
    QueueFamilyIndices::Key GetQueueKey(VkQueue queue) const;   
    CommandBuffer* CreateCommandBufferImp(VkQueue queue, bool temprary);
    bool AllHardWareFeatureSupported(VkPhysicalDevice phyDevice) const;
    VkPhysicalDeviceFeatures HardwareFeaturesToVkPhysicalDeviceFeatures() const;;
};

