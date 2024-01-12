#pragma once 
#include<vulkan\vulkan.h>
#include<string>
#include<vector>
#include<utility>
#include"core\QueueFamilyIndices.h"
#include"core\CoreUtils.h"
#include"rendering\ObjectPool.h"


class CommandBuffer;
class Buffer;
class Fence;


class Device
{
private:
    VkPhysicalDevice m_vkPhyDevice{VK_NULL_HANDLE};
    QueueFamilyIndices m_DeviceQueueFamilyIndices{};
    VkPhysicalDeviceMemoryProperties m_PhyDeviceMemProps{};
    
    std::vector<std::string> m_DeviceExtendsions{};
    std::vector<HardwareFeature> m_EnablePhyDeviceFeatures{};

    VkDevice m_vkDevice{VK_NULL_HANDLE};
    VkQueue m_DeviceQueues[QUEUE_FAMILY_MAX_COUNT] {VK_NULL_HANDLE}; 
    VkSurfaceKHR m_PresentSurface {VK_NULL_HANDLE};
    int m_DeviceQueuesPresentIdx{-1};
    

    VkCommandPool m_DeviceQueueCmdPools[QUEUE_FAMILY_MAX_COUNT * 2];
    VkCommandBuffer m_DeviceQueueCmdBuffers[QUEUE_FAMILY_MAX_COUNT];

private:
    std::vector<Buffer*> _BuffersRes{};
    std::vector<Fence*> _Fences{};

    ObjectPool<CommandBuffer> _CmdBufPool;

public:
    static Device* sActive;
    static bool HasActive() { return sActive != nullptr; }
    void SetActive() { sActive = this; }  

public:
    Device();
    ~Device() { Release(); };

    NONE_COPYABLE(Device)
    
    void SetDeviceFeatureHint(HardwareFeature feature, bool enabled);
    void SetDeviceExtendsionHint(const char* extendsionName, bool enabled) { vkutils_toggle_extendsion_or_layer_name_active(m_DeviceExtendsions, extendsionName, enabled); }
    void ResetAllHints();

    bool Initailze(VkPhysicalDevice phyDevice, VkSurfaceKHR presentSurface);
    bool IsValid() const { return m_vkDevice != VK_NULL_HANDLE; }
    void Release();
    void WaitIdle() const;

    VkDevice GetHandle() const { return m_vkDevice; }
    VkPhysicalDevice GetHardwardHandle() const { return m_vkPhyDevice; }

    bool SupportGrapic() const { return VKHANDLE_IS_NOT_NULL(m_DeviceQueues[QUEUE_FAMILY_GRAPICS_INDEX]); }
    bool SupportCompute() const { return VKHANDLE_IS_NOT_NULL(m_DeviceQueues[QUEUE_FAMILY_COMPUTE_INDEX]); }
    bool SupportTransfer() const { return VKHANDLE_IS_NOT_NULL(m_DeviceQueues[QUEUE_FAMILY_GRAPICS_INDEX]); }
    bool SupportPrenset(VkSurfaceKHR surface) const { return surface == m_PresentSurface && m_DeviceQueuesPresentIdx != -1; }

    VkQueue GetGrapicQueue() const { return m_DeviceQueues[QUEUE_FAMILY_GRAPICS_INDEX]; }
    VkQueue GetcomputeQueue() const { return m_DeviceQueues[QUEUE_FAMILY_COMPUTE_INDEX]; }
    VkQueue GetTransferQueue() const { return m_DeviceQueues[QUEUE_FAMILY_GRAPICS_INDEX]; }
    VkQueue GetPresentQueue() const { return m_DeviceQueuesPresentIdx == -1 ? VK_NULL_HANDLE : m_DeviceQueues[m_DeviceQueuesPresentIdx]; }

    CommandBuffer* GetCommandBuffer(DeviceJobOperation op);
    CommandBuffer* GetTempraryCommandBuffer(DeviceJobOperation op);
    bool ReleaseCommandBuffer(CommandBuffer* pCmdBuf);
    bool DestroyCommandBuffer(CommandBuffer* pCmdBuf);

    // Memory Alloc & Free
    bool AllocMemory(uint32_t memTypeBits, VkMemoryPropertyFlags memPropFlag, VkDeviceSize size, VkDeviceMemory* pAllocatedMem);
    void FreeMemory(VkDeviceMemory mem);

    // Resource Create & Destroy
    Buffer* CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp);
    bool DestroyBuffer(Buffer* pBuffer);

    Fence* CreateFence(bool signaled);
    bool DestroyFence(Fence* pFence);
    
private:
    bool CreateLogicalDevice(VkPhysicalDevice phyDevice);
    bool CreateCommandPools();
    bool CreateCommandBuffers();
    void QueryDeviceMemoryProperties();
    int GetOperationQueueFamilyIndex(DeviceJobOperation op);
    bool ReleaseCommandBufferImp(CommandBuffer* pCmdBuf);


    bool AllHardWareFeatureSupported(VkPhysicalDevice phyDevice) const;

    VkPhysicalDeviceFeatures HardwareFeaturesToVkPhysicalDeviceFeatures() const;;
};

