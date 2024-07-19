#pragma once 
#include<vulkan\vulkan.h>
#include<string>
#include<vector>
#include<utility>
#include"core\CoreUtils.h"
#include"rendering\Window.h"
#include"rendering\ObjectPool.h"
#include"core\CommandBuffer.h"
#include"core\Buffer.h"
#include"core\Image.h"
#include"core\Fence.h"


struct DeviceCreation
{
    const char** enableExtendsions;
    size_t enableExtendsionCnt;
    const DeviceFeatures* enableFeatures;
    size_t enableFeatureCnt;
    const QueueType* enableQueue;
    size_t enableQueueCnt;
};



class Device
{
private:
    uint32_t m_vkApiVersion{0};
    VkPhysicalDevice m_vkPhyDevice{VK_NULL_HANDLE};
    VkPhysicalDeviceProperties m_PhyDeviceProps{};
    VkPhysicalDeviceFeatures m_PhyDeviceFeatures{};
    VkPhysicalDeviceMemoryProperties m_PhyDeviceMemProps{};
    
    std::vector<std::string> m_DeviceExtendsions{};
    std::vector<DeviceFeatures> m_DeviceFeatures{};

    VkDevice m_vkDevice{VK_NULL_HANDLE};
    VkQueue m_DeviceQueues[QueueType::MaxQueueType];
    int m_DeviceQueueFamilyIndices[QueueType::MaxQueueType];
    VkCommandPool m_DeviceQueueCmdPools[QueueType::MaxQueueType * 2];


private:
    ObjectPool<CommandBuffer> _CmdBufPool;
    ObjectPool<Buffer> _BufferPool;
    ObjectPool<Fence> _FencePool;

public:
    Device();
    ~Device() { Release(); };

    NONE_COPYABLE_NONE_MOVEABLE(Device)
    
    // void SetDeviceFeatureHint(DeviceFeatures feature, bool enabled);
    // void SetDeviceExtendsionHint(const char* extendsionName, bool enabled) { vkutils_toggle_extendsion_or_layer_name_active(m_DeviceExtendsions, extendsionName, enabled); }
    // void ResetAllHints();

    bool Create(VkPhysicalDevice phyDevice, uint32_t driverVersion, const DeviceCreation& desc);
    bool IsValid() const { return m_vkDevice != VK_NULL_HANDLE; }
    void Release();
    void WaitIdle() const;

    VkDevice GetHandle() const { return m_vkDevice; }
    VkPhysicalDevice GetHardwardHandle() const { return m_vkPhyDevice; }

  
    bool SupportPrenset(Window* window) const;
    VkQueue GetMainQueue() const { return m_DeviceQueues[QueueType::Main]; }
    //VkQueue GetComputeQueue() const { return m_DeviceQueues[QueueType::Compute]; }
    //VkQueue GetTransferQueue() const { return m_DeviceQueues[QueueType::Transfer]; }
    VkQueue GetQueue(QueueType queue) const { if (queue == QueueType::MaxQueueType) return VK_NULL_HANDLE; return m_DeviceQueues[queue]; }


    std::vector<VkSurfaceFormatKHR> GetSupportedPresentFormats(Window* window) const;
    std::vector<VkPresentModeKHR> GetSupportedPresentModes(Window* window) const;
    bool GetSurfaceCapabilities(Window* window, VkSurfaceCapabilitiesKHR* pResult) const;

    // Features & Limits
    bool IsFeatureSupport(DeviceFeatures feature) const;
    bool IsFeatureEnabled(DeviceFeatures feature) const;
    bool IsFormatFeatureSupport(VkFormat fmt, VkFormatFeatureFlagBits fmtFeature, bool linearTiling) const;
    uint32_t GetDeviceLimit(DeviceLimits limit) const;

    CommandBuffer* CreateCommandBuffer(VkQueue queue) { return _create_command_buffer(queue, false); }
    CommandBuffer* CreateTempraryCommandBuffer(VkQueue queue) { return _create_command_buffer(queue, true); }
    bool DestroyCommandBuffer(CommandBuffer* pCmdBuf);
    
    // Memory Alloc & Free
    bool AllocMemory(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags memPropFlag, VkDeviceMemory* pAllocatedMem);
    void FreeMemory(VkDeviceMemory mem);

    // Resource Create & Destroy
    
    Buffer* CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp, const char* name = nullptr);
    bool DestroyBuffer(Buffer* pBuffer);


    Fence* CreateFence(bool signaled = false);
    bool DestroyFence(Fence* pFence);

    // shader compiling
    bool CompileShader(const char* path, VkShaderStageFlagBits stage, std::vector<char>& spvCodes) const;

    uint32_t GetDriverVersion() const { return m_vkApiVersion; }
private:

    VkResult _create_command_pools();
    void _release_command_pools();
    CommandBuffer* _create_command_buffer(VkQueue queue, bool temprary);
    QueueType _get_queue_type(VkQueue queue) const;

};

