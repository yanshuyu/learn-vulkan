#pragma once 
#include<vulkan\vulkan.h>
#include<string>
#include<vector>
#include<utility>
#include"core\QueueFamilyIndices.h"
#include"core\CommandBuffer.h"

class Device
{

public:
    enum HardwareFeature
    {
        none,
        geometryShader,
        tessellationShader,
        samplerAnisotropy,
        textureCompressionETC2,
        // will add more
    };

    enum JobOperation
    {
        grapic,
        compute,
        transfer,
        present,
    };

private:
    uint32_t m_ApiVersion;
    bool m_DebugEnabled;
    VkInstance m_vkInstance;
    std::vector<std::string> m_InstanceExtendsions{};
    std::vector<std::string> m_InstanceLayers{};

    VkDebugUtilsMessengerEXT m_vkDebugMsger;
    
    bool m_OffScreenEnable;
    VkSurfaceKHR m_PresentSurface;
    VkQueueFlags m_EnableQueueOperations;

    VkPhysicalDevice m_vkPhyDevice;
    QueueFamilyIndices m_DeviceQueueFamilyIndices{};
    
    std::vector<std::string> m_DeviceExtendsions{};
    std::vector<HardwareFeature> m_EnablePhyDeviceFeatures{};
    VkDevice m_vkDevice;
    VkQueue m_DeviceGraphicQueue;
    VkQueue m_DeviceComputeQueue;
    VkQueue m_DeviceTransferQueue;
    VkQueue m_DevicePresentQueue;
    int m_PresentQueueFamilyIndex;

    VkCommandPool m_DeviceQueueCommandPools[QUEUE_FAMILY_MAX_COUNT * 2];
    VkCommandBuffer m_DeviceQueueCmdBuffers[QUEUE_FAMILY_MAX_COUNT];
public:
    Device();
    Device(const Device& other) = delete;
    Device& operator = (const Device& other) = delete;
    ~Device();

    void SetApiVersionHint(uint32_t prefferVersion) { m_ApiVersion = prefferVersion; }
    void SetDebugEnableHint(bool enabled) { m_DebugEnabled = enabled; }
    void SetInstanceExtendsionHint(const char* extendsionName, bool enabled) { ApplyExtendionOrLayerHint(m_InstanceExtendsions, extendsionName, enabled); }
    void SetInstanceLayerHint(const char* layerName, bool enabled) { ApplyExtendionOrLayerHint(m_InstanceLayers, layerName, enabled); }
    void SetEnableQueueOperationHint(VkQueueFlags queueOperation) { m_EnableQueueOperations = queueOperation; }
    void SetOffScreenRenderingHint(bool offScreenRendering) { m_OffScreenEnable = offScreenRendering; }
    void SetPresentSurfaceHint(VkSurfaceKHR surface) { m_PresentSurface = surface; }
    
    void SetDeviceFeatureHint(HardwareFeature feature, bool enabled);
    void SetDeviceExtendsionHint(const char* extendsionName, bool enabled) { ApplyExtendionOrLayerHint(m_DeviceExtendsions, extendsionName, enabled); }
    
    void ResetAllHints();

    bool Initialize();
    bool Create();
    bool IsInittailize() const { return m_vkPhyDevice != VK_NULL_HANDLE; }
    bool IsCreate() const { return m_vkDevice != VK_NULL_HANDLE; }
    void Release();

    VkInstance GetRawInstance() const { return m_vkInstance; }
    VkPhysicalDevice GetRawPhysicalDevice() const { return m_vkPhyDevice; }
    VkDevice GetRawDevice() const { return m_vkDevice; }

    CommandBuffer GetCommandBuffer(JobOperation op);
    CommandBuffer GetTempraryCommandBuffer(JobOperation op);
private:
    bool CreateInstance();
    bool FindPyhsicalDevice();
    bool CreateLogicalDevice();
    bool CreateCommandPools();
    bool CreateCommandBuffers();

    int GetOperationQueueFamilyIndex(JobOperation op);

    void ApplyExtendionOrLayerHint(std::vector<std::string>& arr, const char* name, bool enabled);
    bool AllHardWareFeatureSupported() const;
    VkPhysicalDeviceFeatures HardwareFeaturesToVkPhysicalDeviceFeatures() const;;

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT msgType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData); 
};

