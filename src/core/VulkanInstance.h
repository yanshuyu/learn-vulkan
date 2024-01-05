#pragma once
#include<vulkan\vulkan.h>
#include"core\CoreUtils.h"

class VulkanInstance
{
private:
    uint32_t m_ApiVersion{0};
    bool m_DebugEnabled{false};
    std::vector<std::string> m_InstanceExtendsions{};
    std::vector<std::string> m_InstanceLayers{};

    VkDebugUtilsMessengerEXT m_vkDebugMsger{VK_NULL_HANDLE};
    VkInstance m_vkInstance{VK_NULL_HANDLE};

public:
    static VulkanInstance* sActive;
    static bool HasActive() { return sActive != nullptr; }
    void SetActive() { sActive = this; }

public:
    VulkanInstance(){};
    VulkanInstance(VulkanInstance&& other)
    {
        m_ApiVersion = other.m_ApiVersion;
        m_DebugEnabled = other.m_DebugEnabled;
        std::swap(m_InstanceExtendsions, other.m_InstanceExtendsions);
        std::swap(m_InstanceLayers, other.m_InstanceLayers);
        m_vkDebugMsger = other.m_vkDebugMsger;
        m_vkInstance = other.m_vkInstance;
        other.ResetAllHints();
        other.m_vkInstance = VK_NULL_HANDLE;
        other.m_vkDebugMsger = VK_NULL_HANDLE;
    }

    VulkanInstance& operator = (VulkanInstance&& other)
    {
        if (this != &other)
        {
            Release();
            m_ApiVersion = other.m_ApiVersion;
            m_DebugEnabled = other.m_DebugEnabled;
            std::swap(m_InstanceExtendsions, other.m_InstanceExtendsions);
            std::swap(m_InstanceLayers, other.m_InstanceLayers);
            m_vkDebugMsger = other.m_vkDebugMsger;
            m_vkInstance = other.m_vkInstance;
            other.ResetAllHints();
            other.m_vkInstance = VK_NULL_HANDLE;
            other.m_vkDebugMsger = VK_NULL_HANDLE;
        }

        return *this;
    }
    ~VulkanInstance()
    {
        Release();
    };

    NONE_COPYABLE(VulkanInstance)

    void SetApiVersionHint(uint32_t prefferVersion) { m_ApiVersion = prefferVersion; }
    void SetDebugEnableHint(bool enabled) { m_DebugEnabled = enabled; }
    void SetInstanceExtendsionHint(const char* extendsionName, bool enabled) { vkutils_toggle_extendsion_or_layer_name_active(m_InstanceExtendsions, extendsionName, enabled); }
    void SetInstanceLayerHint(const char* layerName, bool enabled) { vkutils_toggle_extendsion_or_layer_name_active(m_InstanceLayers, layerName, enabled); }
    void ResetAllHints();

    bool Initailize();

    bool IsValid() const 
    {
        return VKHANDLE_IS_NOT_NULL(m_vkInstance);
    }

    VkInstance GetHandle() const { return m_vkInstance; }
    
    VkPhysicalDevice RequestPhysicalDevice(VkQueueFlags queueOperation, VkSurfaceKHR presentSurface);

    void Release();
};
