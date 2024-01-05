#include"Application.h"
#include<GLFW\glfw3.h>



static void GlfwErrorCallBack(int errCode, const char* errMsg)
{
    LOGE("GLFW Error: {}  Msg: {}", errCode, errMsg);
}

Application::Application(const char* wndTitle, int wndWidth, int wndHeight)
: m_WndTitle(wndTitle)
, m_WndWidth(wndWidth)
, m_WndHeight(wndHeight)
{

}



bool Application::RenderingSetUp()
{
    bool ok = true;
    VkSurfaceFormatKHR swapChainPiexlFmt{};
    VkExtent2D swapChainPiexlDimension{};

    LOGI("-->Rendering SetUp...");

    // we load vulkan statically,  canonical desktop loader library exports all Vulkan core and Khronos extension functions, allowing them to be called directly.
    // so we don't need call this function
    /* 
    ok &= static_cast<bool>(glfwVulkanSupported());
    std::cout << "Detect Vulkan Supported: " << ok << std::endl;
    if (!ok)
        goto init_result;
    */
    ////////////////////////////////////////////////////////////
    // Init Window
    ////////////////////////////////////////////////////////////
    ok &= static_cast<bool>(glfwInit());
    LOGI("-->glfw Init: {}", ok);
    if (!ok)
        goto init_result;
    
    glfwSetErrorCallback(GlfwErrorCallBack);

    // Create Platform Specify Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // bydefault, glfwCreateWindow will create window and proper opengl(es) context
                                                // by set GLFW_CLIENT_API hint, we tell glfw create window for vulkan not gl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_WndHandle = glfwCreateWindow(m_WndWidth, m_WndHeight, m_WndTitle.data(), nullptr, nullptr);
    ok &= (m_WndHandle != nullptr);
    LOGI("-->glfw Create Window: {}", ok);
    if (!ok)
        goto init_result;

    ////////////////////////////////////////////////////////////////////////////////
    // Init Vulkan
    /////////////////////////////////////////////////////////////////////////////////
    //create vulkan instance
    uint32_t glfwRequireInstanceExtensionCnt = 0;
    const char** glfwRequireInstanceExtensionNames = glfwGetRequiredInstanceExtensions(&glfwRequireInstanceExtensionCnt);
    for (size_t i = 0; i < glfwRequireInstanceExtensionCnt; i++)
    {
        m_VukanInstance.SetInstanceExtendsionHint(*glfwRequireInstanceExtensionNames, true);
        glfwRequireInstanceExtensionNames++;
    }
    m_VukanInstance.SetDebugEnableHint(true);
     
    ok &= m_VukanInstance.Initailize();
    LOGI("--> Vulkan Instance Create: {}", ok);
    if (!ok)
        goto init_result;

    m_VukanInstance.SetActive();
    
    // Create vulkan platform specific window surface
    ok &= glfwCreateWindowSurface(VulkanInstance::sActive->GetHandle(), m_WndHandle, nullptr, &m_vkSurface) == VK_SUCCESS;
    LOGI("-->Create Vulkan Surface: {}", ok);
    if (!ok)
        goto init_result;

    // Find a physical gpu which support expected operation
    VkPhysicalDevice suitableGpu = VulkanInstance::sActive->RequestPhysicalDevice(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, m_vkSurface);
    ok &= VKHANDLE_IS_NOT_NULL(suitableGpu);
    LOGI("--> Find Suitable Gpu: {}", ok);
    if (!ok)
        goto init_result;

    // Create vulkan logical Device
    m_Device.SetDeviceExtendsionHint(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
    ok &= m_Device.Initailze(suitableGpu, m_vkSurface);
    LOGI("--> Create Vulkan Device: {}", ok);
    if (!ok)
        goto init_result;

    m_Device.SetActive();

    // Create SwapChain ()
    swapChainPiexlFmt.format = VK_FORMAT_B8G8R8A8_UNORM;
    swapChainPiexlFmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    glfwGetFramebufferSize(m_WndHandle, reinterpret_cast<int*>(&swapChainPiexlDimension.width), reinterpret_cast<int*>(&swapChainPiexlDimension.height));
    m_SwapChain.Init(Device::sActive->GetHardwardHandle(), Device::sActive->GetHandle(), m_vkSurface);
    ok &= m_SwapChain.Create(swapChainPiexlFmt, VK_PRESENT_MODE_MAILBOX_KHR, 2, swapChainPiexlDimension);
    LOGI("--> Create SwapChain: {}",ok);
    if (!ok)
        goto init_result;

init_result:
    LOGI("-->Rendering SetUp Finish:  {}", ok);  
    return ok;
}



void Application::RenderingCleanUp()
{
    LOGI("--> Rendering CleanUp...")
    m_Device.WaitIdle();

    m_SwapChain.Release();

    if (m_vkSurface != nullptr)
    {
        vkDestroySurfaceKHR(m_VukanInstance.GetHandle(), m_vkSurface, nullptr);
        m_vkSurface = nullptr;
        LOGI("--> destory Vulkan Surface." );
    }

    m_Device.Release();
    LOGI("--> destory Vulkan Device." );

    m_VukanInstance.Release();
    LOGI("--> destory Vulkan Instance." );

    if (m_WndHandle != nullptr)
    {
        glfwDestroyWindow(m_WndHandle);
        m_WndHandle = nullptr;
        LOGI("--> destory GLFW Window." );
    }

    glfwTerminate();
    
}



void Application::Run()
{
    while (!glfwWindowShouldClose(m_WndHandle))
    {
        ApplicationUpdate();
        Render();
        glfwPollEvents();
    }
}