#pragma once 
#include<vulkan\vulkan.h>
#include<string>
#include<vector>
#include<utility>
#include"core\QueueFamilyIndices.h"
#include"core\CoreUtils.h"

class CommandBuffer;

class Device
{
private:
    VkPhysicalDevice m_vkPhyDevice{VK_NULL_HANDLE};
    QueueFamilyIndices m_DeviceQueueFamilyIndices{};
    
    std::vector<std::string> m_DeviceExtendsions{};
    std::vector<HardwareFeature> m_EnablePhyDeviceFeatures{};

    VkDevice m_vkDevice{VK_NULL_HANDLE};
    VkQueue m_DeviceGraphicQueue{VK_NULL_HANDLE};
    VkQueue m_DeviceComputeQueue{VK_NULL_HANDLE};
    VkQueue m_DeviceTransferQueue{VK_NULL_HANDLE};
    VkQueue m_DevicePresentQueue{VK_NULL_HANDLE};
    int m_PresentQueueFamilyIndex{-1};

    VkCommandPool m_DeviceQueueCommandPools[QUEUE_FAMILY_MAX_COUNT * 2];
    VkCommandBuffer m_DeviceQueueCmdBuffers[QUEUE_FAMILY_MAX_COUNT];

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

    CommandBuffer GetCommandBuffer(DeviceJobOperation op);
    CommandBuffer GetTempraryCommandBuffer(DeviceJobOperation op);
private:
    bool CreateLogicalDevice(VkPhysicalDevice phyDevice);
    bool CreateCommandPools();
    bool CreateCommandBuffers();

    int GetOperationQueueFamilyIndex(DeviceJobOperation op);

    bool AllHardWareFeatureSupported(VkPhysicalDevice phyDevice) const;

    VkPhysicalDeviceFeatures HardwareFeaturesToVkPhysicalDeviceFeatures() const;;
};

