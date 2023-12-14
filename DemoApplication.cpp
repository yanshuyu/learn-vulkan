#include "DemoApplication.h"
#include<iostream>
#define GLFW_INCLUDE_VULKAN
#include<GLFW\glfw3.h>



DemoApplication::DemoApplication(const std::string& wndTitle, int wndWidth, int wndHeight)
: m_WndTitle(wndTitle),
m_WndWidth(wndWidth),
m_WndHeight(wndHeight),
m_WndHandle(nullptr)
{

}

DemoApplication::~DemoApplication()
{

}

bool DemoApplication::Init()
{
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
}


bool DemoApplication::SystemSetUp()
{
    bool result = true;

    std::cout << "-->SystemSetUp..." << std::endl;

    //if (!glfwVulkanSupported())
        //return false;


    if (!glfwInit())
    {
        result = false;
        goto init_result;
    }

    std::cout << "-->glfw Init Success." << std::endl;

    glfwSetErrorCallback(GlfwErrorCallBack);

    // Create Platform Specify Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // bydefault, glfwCreateWindow will create window and proper opengl(es) context
                                                // by set GLFW_CLIENT_API hint, we tell glfw create window for vulkan not gl
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    m_WndHandle = glfwCreateWindow(m_WndWidth, m_WndHeight, m_WndTitle.data(), nullptr, nullptr);

    if (m_WndHandle == nullptr)
    {
        result = false;
        goto init_result;
    }
    std::cout << "-->glfw Create Window Success." << std::endl;

    // Init Vulkan
        


init_result:
    std::cout << "-->SystemSetUp Finish " << result << std::endl;  
    return result;
}

void DemoApplication::SystemCleanUp()
{
    if (m_WndHandle != nullptr)
    {
        glfwDestroyWindow(m_WndHandle);
        m_WndHandle = nullptr;
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


