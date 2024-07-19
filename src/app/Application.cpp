#include"Application.h"
#include"core\VulkanInstance.h"
#include"core\Device.h"
#include"core\SwapChain.h"
#include"rendering\Window.h"
#include"rendering\AssetsManager.h"
#include"rendering\DescriptorSetManager.h"
#include"rendering\PipelineManager.h"


Application::Application(const AppDesc& appDesc)
: m_appDesc(appDesc)
{

}


bool Application::Prepare(Window* window)
{
    ////////////////////////////////////////////////////////////////////////////////
    // Init Vulkan
    /////////////////////////////////////////////////////////////////////////////////
    //create vulkan instance
    m_pVulkanInstance = std::make_unique<VulkanInstance>();
    uint32_t glfwRequireInstanceExtensionCnt = 0;
    const char** glfwRequireInstanceExtensionNames = window->GetRequireVulkanInstanceExtendsion(&glfwRequireInstanceExtensionCnt);
    for (size_t i = 0; i < glfwRequireInstanceExtensionCnt; i++)
    {
        m_pVulkanInstance->SetInstanceExtendsionHint(glfwRequireInstanceExtensionNames[i], true);
    }

    for (size_t i = 0; i < m_appDesc.enabledInstanceExtendsionCount; i++)
    {   
        m_pVulkanInstance->SetInstanceExtendsionHint( m_appDesc.enabledInstanceExtendsionNames[i], true);
    }
    
    for (size_t i = 0; i < m_appDesc.enabledInstanceLayerCout; i++)
    {
       m_pVulkanInstance->SetInstanceLayerHint(m_appDesc.enabledInstanceLayerNames[i], true);
    }
    

    m_pVulkanInstance->SetDebugEnableHint(m_appDesc.debugEnabled);
     
    m_pVulkanInstance->SetApiVersionHint(m_appDesc.vulkanApiVersion);

    if (!m_pVulkanInstance->Initailize())
    {
        LOGE("-->App Prepare: Failed to init vulkan instance");
        return false;
    }

    //m_VukanInstance.SetActive();

    // Create vulkan platform specific window surface
    if (window)
    {
        if (VKHANDLE_IS_NULL(window->CreateVulkanSurface(m_pVulkanInstance->GetHandle())))
        {
            LOGE("-->App Prepare: Failed to init vulkan surface!");
            return false;
        }
        m_window = window;
    }

    // Find a physical gpu which support expected operation
    VkPhysicalDevice suitableGpu = m_pVulkanInstance->RequestPhysicalDevice(m_appDesc.enabledQueueTypes, m_appDesc.enabledQueueTypeCount, window ? window->GetVulkanSurface() : VK_NULL_HANDLE);
    if (VKHANDLE_IS_NULL(suitableGpu))
    {
        LOGE("-->App Prepare: No avalible device!");
        return false;
    }


    // Create vulkan logical Device
    std::vector<const char*> deviceExtentsions{};
    deviceExtentsions.reserve(m_appDesc.enabledDeviceExtendsionCount + 1);
    for (size_t i = 0; i < m_appDesc.enabledDeviceExtendsionCount; i++)
        deviceExtentsions.push_back(m_appDesc.enabledDeviceExtendsionNames[i]);
    if (window)
       deviceExtentsions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    
    DeviceCreation dc{};
    dc.enableExtendsionCnt = deviceExtentsions.size();
    dc.enableExtendsions = deviceExtentsions.data();
    dc.enableFeatureCnt = m_appDesc.enabledDeviceFeatureCount;
    dc.enableFeatures = m_appDesc.enabledDeviceFeatures;
    dc.enableQueueCnt = m_appDesc.enabledQueueTypeCount;
    dc.enableQueue = m_appDesc.enabledQueueTypes; 

    m_pDevice = std::make_unique<Device>();
    if (!m_pDevice->Create(suitableGpu, m_pVulkanInstance->GetApiVersion(),dc))
    {
       LOGE("-->App Prepare: Failed to init device!");
       return false; 
    }

    //m_Device.SetActive();

    // Create SwapChain ()
    if (window)
    {
        SwapChainDesc desc{};
        desc.format = m_appDesc.swapChainColorFormat;
        desc.colorSpace = m_appDesc.swapChainColorSpace;
        desc.bufferExtend = {(uint32_t)m_window->GetWidth(), (uint32_t)m_window->GetHeight()};
        desc.enableVSync = m_appDesc.vsyncEnabled;
        desc.enableTripleBuffering = m_appDesc.swapChainTrippleBufferEnabled;
        m_pSwapChain = std::make_unique<SwapChain>();
        if ( !m_pSwapChain->Create(m_pDevice.get(), m_window, desc))
        {
            LOGE("-->App Prepare: Failed to init swapchian!");
            return false;
        }
    }

    AssetsManager::Initailize(m_pDevice.get());
    DescriptorSetManager::Initailize(m_pDevice.get());
    PipelineManager::Initailize(m_pDevice.get());

    // concrete application will prepare it's resource here...
    return Setup();
}


void Application::Finish()
{
    // makre sure all cmd are finish
    
    if (m_pDevice)
        m_pDevice->WaitIdle();

    // concrete application must relase all device's child resource here ...
    Release();

    PipelineManager::DeInitailize();
    DescriptorSetManager::DeInitailize();
    AssetsManager::DeInitailize();

    if (m_pSwapChain)
        m_pSwapChain->Release();

    // it's safer to destory device firstly, becuse device's render cmd will present to window surface
    if (m_pDevice)
        m_pDevice->Release();

    // make sure destory all instance's child resource before destoy instance
    if (m_window)
        m_window->DestroyVulkanSurface(m_pVulkanInstance->GetHandle());

    // finally destroy instance
    if(m_pVulkanInstance)
        m_pVulkanInstance->Release();

    m_pDevice.reset(nullptr);
    m_pSwapChain.reset(nullptr);
    m_pVulkanInstance.reset(nullptr);
}