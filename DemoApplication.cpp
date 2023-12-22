#include "DemoApplication.h"
#include<iostream>
#include"VulkanUtil.h"


DemoApplication::DemoApplication(const std::string& wndTitle, int wndWidth, int wndHeight)
: m_WndTitle(wndTitle),
m_WndWidth(wndWidth),
m_WndHeight(wndHeight),
m_WndHandle(nullptr),
m_vkSurface(nullptr),
m_vkInstance(nullptr),
m_vkDebugMsger(nullptr),
m_vkDevice(nullptr),
m_vkDeviceQueueFamilyIndices(),
m_vkDeviceGraphicQueue(nullptr),
m_vkDevicePresentQueue(nullptr),
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
    vector<string> instanceEnableExtendtionNames;
    vector<string> instanceEnableLayerNames;
    vector<string> deviceEnabledExtendsions{VK_KHR_SWAPCHAIN_EXTENSION_NAME};
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
    instanceEnableExtendtionNames.resize(glfwRequireInstanceExtensionCnt);
    for (size_t i = 0; i < glfwRequireInstanceExtensionCnt; i++)
    {
        instanceEnableExtendtionNames[i].assign(*glfwRequireInstanceExtensionNames);
        glfwRequireInstanceExtensionNames++;
    }
    instanceEnableExtendtionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME); // roung validation layer's debug msg to our callback fundtion
    instanceEnableLayerNames.push_back(VulkanUtil::KHRONOS_STANDARD_VALIDATIONLAYER_NAME);

    
    ok &= VulkanUtil::CreateInstance(instanceEnableExtendtionNames, instanceEnableLayerNames, &m_vkInstance, &m_vkDebugMsger);
    std::cout << "-->Create Vulkan Instance: " << ok << std::endl;

    // Create vulkan platform specific window surface
    ok &= glfwCreateWindowSurface(m_vkInstance, m_WndHandle, nullptr, &m_vkSurface) == VK_SUCCESS;
    std::cout << "-->Create Vulkan Surface: " << ok << std::endl;
    if (!ok)
        goto init_result;

    // Pick Suitable Physical Device
    VkPhysicalDevice phyDevice = VK_NULL_HANDLE;
    ok &= VulkanUtil::FindPyhsicalDevice(m_vkInstance, m_vkSurface, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT, &phyDevice);
    std::cout << "-->Found Suitable Physical Device: " << ok << std::endl;
    if (!ok)
        goto init_result;

    // create vulkan device
    ok &= VulkanUtil::CreateDevice(phyDevice, deviceEnabledExtendsions, &m_vkDevice, &m_vkDeviceQueueFamilyIndices);
    std::cout << "-->Create Vulkan Device: " << ok << std::endl;
    if (!ok)
        goto init_result;
    
    vkGetDeviceQueue(m_vkDevice, m_vkDeviceQueueFamilyIndices.GrapicQueueFamilyIndex(), 0, &m_vkDeviceGraphicQueue);
    std::cout << "-->Vulkan Device Graphic Queue Family Index: " << m_vkDeviceQueueFamilyIndices.GrapicQueueFamilyIndex() << std::endl;
    if (m_vkDeviceGraphicQueue == nullptr)
    {
        std::cout <<"-->Failed to Get Graphices Queue." << std::endl;
        ok = false;
        goto init_result;
    }

    vkGetDeviceQueue(m_vkDevice, m_vkDeviceQueueFamilyIndices.PresentQueueFamilyIndex(m_vkSurface), 0, &m_vkDevicePresentQueue);
     std::cout << "-->Vulkan Device Present Queue Family Index: " <<m_vkDeviceQueueFamilyIndices.PresentQueueFamilyIndex(m_vkSurface) << std::endl;
    if (m_vkDevicePresentQueue == nullptr)
    {
        std::cout <<"-->Failed to Get Graphices Queue." << std::endl;
        ok = false;
        goto init_result;
    }

    // Create SwapChain ()
    swapChainPiexlFmt.format = VK_FORMAT_R8G8B8_UNORM;
    swapChainPiexlFmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    glfwGetFramebufferSize(m_WndHandle, reinterpret_cast<int*>(&swapChainPiexlDimension.width), reinterpret_cast<int*>(&swapChainPiexlDimension.height));
    m_SwapChain.Init(phyDevice, m_vkDevice, m_vkSurface);
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
        vkDestroySurfaceKHR(m_vkInstance, m_vkSurface, nullptr);
        m_vkSurface = nullptr;
        std::cout << "-->SystemCleanUp destory Vulkan Surface." << std::endl;
    }

    if (VulkanUtil::DestroyDevice(&m_vkDevice))
    std::cout << "-->SystemCleanUp Destroy Vulkan Device." << std::endl;


    if (VulkanUtil::DestoryInstance(&m_vkInstance, &m_vkDebugMsger))
        std::cout << "-->SystemCleanUp Destroy Vulkan Instance." << std::endl;

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


