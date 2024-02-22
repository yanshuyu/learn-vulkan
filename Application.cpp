#include"Application.h"
#include"core\VulkanInstance.h"
#include"core\Device.h"
#include"core\SwapChain.h"
#include"rendering\Window.h"


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
    
    // Find a physical gpu which support expected operation
    VkPhysicalDevice suitableGpu = m_pVulkanInstance->RequestPhysicalDevice(m_appDesc.enableQueueOperation, window->GetVulkanSurface());
    if (VKHANDLE_IS_NULL(suitableGpu))
    {
        LOGE("-->App Prepare: No avalible device!");
        return false;
    }
    else
    {
        LOGI("-->App Prepare: Avalible device({})", (void*)suitableGpu);
        LOGI("\tqueue family index\t|\tqueue operation\t|\tqueue count\n");
        QueueFamilyIndices queueFamIndices(suitableGpu);
        for (size_t i = 0; i < queueFamIndices.QueueFamilyCount(); i++)
        {
            VkQueueFamilyProperties prop = queueFamIndices.QueueFamilyProperties(i);
            LOGI("\t{}\t{}\t{}", i, prop.queueFlags, prop.queueCount);
        }
    }

    // Create vulkan logical Device
    m_pDevice = std::make_unique<Device>();
    m_pDevice->SetDeviceExtendsionHint(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
    for (size_t i = 0; i < m_appDesc.enabledDeviceExtendsionCount; i++)
    {
        m_pDevice->SetDeviceExtendsionHint(m_appDesc.enabledDeviceExtendsionNames[i], true);
    }
    
    for (size_t j = 0; j < m_appDesc.enabledDeviceFeatureCount; j++)
    {
        m_pDevice->SetDeviceFeatureHint(m_appDesc.enabledDeviceFeatures[j], true);
    }
    

    if (!m_pDevice->Initailze(suitableGpu))
    {
       LOGE("-->App Prepare: Failed to init device!");
       return false; 
    }

    //m_Device.SetActive();

    // Create vulkan platform specific window surface
    if (VKHANDLE_IS_NULL(window->CreateVulkanSurface(m_pVulkanInstance->GetHandle())))
    {
        LOGE("-->App Prepare: Failed to init vulkan surface!");
        return false;
    }
    m_window = window;


    // Create SwapChain ()
    m_pSwapChain = std::make_unique<SwapChain>();


    // concrete application will prepare it's resource here...
    return Setup();
}


void Application::Finish()
{
    // makre sure all cmd are finish
    m_pDevice->WaitIdle();

    // concrete application must relase all device's child resource here ...
    Release();

    m_pSwapChain->Release();

    // it's safer to destory device firstly, becuse device's render cmd will present to window surface
    m_pDevice->Release();

    // make sure destory all instance's child resource before destoy instance
    if (m_window)
        m_window->DestroyVulkanSurface(m_pVulkanInstance->GetHandle());

    // finally destroy instance
    m_pVulkanInstance->Release();

    m_pDevice.reset(nullptr);
    m_pSwapChain.reset(nullptr);
    m_pVulkanInstance.reset(nullptr);
}