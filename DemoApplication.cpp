#include "DemoApplication.h"
#include<iostream>
#include"VulkanUtil.h"


DemoApplication::DemoApplication(const std::string& wndTitle, int wndWidth, int wndHeight)
: m_WndTitle(wndTitle),
m_WndWidth(wndWidth),
m_WndHeight(wndHeight),
m_WndHandle(nullptr),
m_vkInstance(0)
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

    // Init Vulkan
    uint32_t glfwRequireInstanceExtensionCnt = 0;
    const char** glfwRequireInstanceExtensionNames = glfwGetRequiredInstanceExtensions(&glfwRequireInstanceExtensionCnt);
    instanceEnableExtendtionNames.resize(glfwRequireInstanceExtensionCnt);
    for (size_t i = 0; i < glfwRequireInstanceExtensionCnt; i++)
    {
        instanceEnableExtendtionNames[i].assign(*glfwRequireInstanceExtensionNames);
        glfwRequireInstanceExtensionNames++;
    }
    
    ok &= VulkanUtil::CreateInstance(instanceEnableExtendtionNames, instanceEnableLayerNames, &m_vkInstance);
    std::cout << "-->Create Vulkan Instance: " << ok << std::endl;
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


