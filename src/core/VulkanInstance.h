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
    VulkanInstance() = default;
    ~VulkanInstance()
    {
        Release();
    };

    NONE_COPYABLE_NONE_MOVEABLE(VulkanInstance)

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
