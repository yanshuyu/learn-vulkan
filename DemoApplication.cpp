#include "DemoApplication.h"
#include<iostream>


#define KHRONOS_STANDARD_VALIDATIONLAYER_NAME "VK_LAYER_KHRONOS_validation"

DemoApplication::DemoApplication(const std::string& wndTitle, int wndWidth, int wndHeight)
: m_WndTitle(wndTitle),
m_WndWidth(wndWidth),
m_WndHeight(wndHeight),
m_WndHandle(nullptr),
m_vkSurface(nullptr),
m_SwapChain()
{

}

DemoApplication::~DemoApplication()
{

}

bool DemoApplication::Init()
{
    std::cout << "Application Boots Up." << std::endl;
    bool ok = SystemSetUp();
    if (ok)
        ok &= Start();
    
    if (!ok)
        ShutDown();

    return ok;
}


void DemoApplication::Run()
{
    while (!glfwWindowShouldClose(m_WndHandle))
    {


        glfwPollEvents();
    }
}


void DemoApplication::ShutDown()
{
    Release();
    SystemCleanUp();
    std::cout << "Application Shut Down." << std::endl;
}


bool DemoApplication::SystemSetUp()
{
    bool ok = true;
    VkSurfaceFormatKHR swapChainPiexlFmt{};
    VkExtent2D swapChainPiexlDimension{};

    std::cout << "-->SystemSetUp..." << std::endl;

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
    std::cout << "-->glfw Init: " << ok << std::endl;
    if (!ok)
        goto init_result;
    
    glfwSetErrorCallback(GlfwErrorCallBack);

    // Create Platform Specify Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // bydefault, glfwCreateWindow will create window and proper opengl(es) context
                                                // by set GLFW_CLIENT_API hint, we tell glfw create window for vulkan not gl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_WndHandle = glfwCreateWindow(m_WndWidth, m_WndHeight, m_WndTitle.data(), nullptr, nullptr);
    ok &= (m_WndHandle != nullptr);
    std::cout << "-->glfw Create Window: " << ok << std::endl;
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
        m_Device.SetInstanceExtendsionHint(*glfwRequireInstanceExtensionNames, true);
        glfwRequireInstanceExtensionNames++;
    }
    m_Device.SetInstanceExtendsionHint(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true); // roung validation layer's debug msg to our callback fundtion
    m_Device.SetInstanceLayerHint(KHRONOS_STANDARD_VALIDATIONLAYER_NAME, true);
    m_Device.SetDebugEnableHint(true);
    m_Device.SetDeviceExtendsionHint(VK_KHR_SWAPCHAIN_EXTENSION_NAME, true);
    
    
    ok &= m_Device.Initialize();
    if (!ok)
        goto init_result;
    
    // Create vulkan platform specific window surface
    ok &= glfwCreateWindowSurface(m_Device.GetRawInstance(), m_WndHandle, nullptr, &m_vkSurface) == VK_SUCCESS;
    std::cout << "-->Create Vulkan Surface: " << ok << std::endl;
    if (!ok)
        goto init_result;

    // Pick Suitable Physical Device
   
    ok &= m_Device.Create();
    if (!ok)
        goto init_result;

    // Create SwapChain ()
    swapChainPiexlFmt.format = VK_FORMAT_R8G8B8_UNORM;
    swapChainPiexlFmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    glfwGetFramebufferSize(m_WndHandle, reinterpret_cast<int*>(&swapChainPiexlDimension.width), reinterpret_cast<int*>(&swapChainPiexlDimension.height));
    m_SwapChain.Init(m_Device.GetRawPhysicalDevice(), m_Device.GetRawDevice(), m_vkSurface);
    ok &= m_SwapChain.Create(swapChainPiexlFmt, VK_PRESENT_MODE_MAILBOX_KHR, 2, swapChainPiexlDimension);
    std::cout << "--> Create SwapChain: " << ok << std::endl;
    if (!ok)
        goto init_result;

init_result:
    std::cout << "-->SystemSetUp Finish " << ok << std::endl;  
    return ok;
}

void DemoApplication::SystemCleanUp()
{

    m_SwapChain.Release();

    if (m_vkSurface != nullptr)
    {
        vkDestroySurfaceKHR(m_Device.GetRawInstance(), m_vkSurface, nullptr);
        m_vkSurface = nullptr;
        std::cout << "-->SystemCleanUp destory Vulkan Surface." << std::endl;
    }

    m_Device.Release();

    if (m_WndHandle != nullptr)
    {
        glfwDestroyWindow(m_WndHandle);
        m_WndHandle = nullptr;
        std::cout << "-->SystemCleanUp destory window." << std::endl;
    }

    glfwTerminate();
}

bool DemoApplication::Start()
{
    return true;
}

void DemoApplication::Release()
{

}


void DemoApplication::GlfwErrorCallBack(int errCode, const char* errMsg)
{
    std::cerr << "GLFW Error Code: " << errCode << std::endl << "Error Msg: " << errMsg << std::endl;
}


