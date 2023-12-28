#pragma once 
#include<vulkan\vulkan.h>
#include<string>
#include<vector>
#include"QueueFamilyIndices.h"

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

private:
    VkInstance m_vkInstance;
    std::vector<std::string> m_InstanceExtendsions{};
    std::vector<std::string> m_InstanceLayers{};

    VkDebugUtilsMessengerEXT m_vkDebugMsger;
    
    VkSurfaceKHR m_PresentSurface;
    VkQueueFlags m_EnableQueueOperations;

    VkPhysicalDevice m_vkPhyDevice;
    QueueFamilyIndices m_DeviceQueueFamilyIndices{};
    
    std::vector<std::string> m_DeviceExtendsions{};
    std::vector<HardwareFeature> m_EnablePhyDeviceFeatures{};
    VkDevice m_vkDevice;
    VkQueue m_DeviceGraphicQueue;
    VkQueue m_DevicePresentQueue; 

    uint32_t m_ApiVersion;
    bool m_DebugEnabled;
    bool m_OffScreenEnable;
public:
    Device();
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

private:
    bool CreateInstance();
    bool FindPyhsicalDevice();
    bool CreateLogicalDevice();

    void ApplyExtendionOrLayerHint(std::vector<std::string>& arr, const char* name, bool enabled);
    bool AllHardWareFeatureSupported() const;
    VkPhysicalDeviceFeatures HardwareFeaturesToVkPhysicalDeviceFeatures() const;;

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT msgType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData); 
};

