#pragma once
#include<memory>
#include"core\CoreUtils.h"


class Window;
class VulkanInstance;
class Device;
class SwapChain;


struct AppDesc
{   
    bool debugEnabled;
    bool vsyncEnabled;
    bool swapChainTrippleBufferEnabled;
    uint32_t vulkanApiVersion;
    size_t enabledInstanceLayerCout;
    const char** enabledInstanceLayerNames;
    size_t enabledInstanceExtendsionCount;
    const char** enabledInstanceExtendsionNames;
    size_t enabledDeviceExtendsionCount;
    const char** enabledDeviceExtendsionNames;
    size_t enabledDeviceFeatureCount;
    const HardwareFeature* enabledDeviceFeatures;
    VkQueueFlags enableQueueOperation;
    VkFormat swapChainColorFormat;
    VkColorSpaceKHR swapChainColorSpace;
    float backBufferClearColor[4];
};


class Application
{
protected:
    std::unique_ptr<VulkanInstance> m_pVulkanInstance;
    std::unique_ptr<Device> m_pDevice;
    std::unique_ptr<SwapChain> m_pSwapChain;    
    Window* m_window{};

    AppDesc m_appDesc{};
    
public:
    Application(const AppDesc& appDesc);
    virtual ~Application() {};

    virtual bool Prepare(Window* window);

    virtual void Step() = 0;

    virtual void Finish();

    virtual bool Setup() { return true; };

    virtual void Release() {};

    virtual void Resize(size_t width, size_t height) {};

};