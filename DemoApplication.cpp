#include "DemoApplication.h"
#include<iostream>
#include"VulkanUtil.h"


DemoApplication::DemoApplication(const std::string& wndTitle, int wndWidth, int wndHeight)
: m_WndTitle(wndTitle),
m_WndWidth(wndWidth),
m_WndHeight(wndHeight),
m_WndHandle(nullptr),
m_vkInstance(nullptr),
m_vkDebugMsger(nullptr),
m_vkDevice(nullptr),
m_vkDeviceQueueFamilyIndices()
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
    vector<string> deviceEnabledExtendsions;

    std::cout << "-->SystemSetUp..." << std::endl;

    // we load vulkan statically,  canonical desktop loader library exports all Vulkan core and Khronos extension functions, allowing them to be called directly.
    // so we don't need call this function
    /* 
    ok &= static_cast<bool>(glfwVulkanSupported());
    std::cout << "Detect Vulkan Supported: " << ok << std::endl;
    if (!ok)
        goto init_result;
    */

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

    // create vulkan device
    ok &= VulkanUtil::CreateDevice(&m_vkInstance, VK_QUEUE_GRAPHICS_BIT, deviceEnabledExtendsions, &m_vkDevice, &m_vkDeviceQueueFamilyIndices);
    std::cout << "--> Create Vulkan Device: " << ok << std::endl;

    if (!ok)
        goto init_result;
    

init_result:
    std::cout << "-->SystemSetUp Finish " << ok << std::endl;  
    return ok;
}

void DemoApplication::SystemCleanUp()
{
    if (m_WndHandle != nullptr)
    {
        glfwDestroyWindow(m_WndHandle);
        m_WndHandle = nullptr;
        std::cout << "-->SystemCleanUp destory window." << std::endl;
    }

    if (VulkanUtil::DestroyDevice(&m_vkDevice))
        std::cout << "-->SystemCleanUp Destroy Vulkan Device." << std::endl;

    if (VulkanUtil::DestoryInstance(&m_vkInstance, &m_vkDebugMsger))
        std::cout << "-->SystemCleanUp Destroy Vulkan Instance." << std::endl;


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


