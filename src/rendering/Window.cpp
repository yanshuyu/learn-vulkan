#include<stdexcept>
#include"rendering\Window.h"
#include"GLFW\glfw3.h"



static void GlfwErrorCallBack(int errCode, const char* errMsg)
{
    LOGE("GLFW Error: {}  Msg: {}", errCode, errMsg);
}

static void ResizeCallback(GLFWwindow* window, int width, int height)
{
   Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
   pWindow->Resize(width, height);
}


Window::Window(const WindowDesc& wndDesc)
{
   
    // we load vulkan statically,  canonical desktop loader library exports all Vulkan core and Khronos extension functions, allowing them to be called directly.
    // so we don't need call this function
    /* 
    ok &= static_cast<bool>(glfwVulkanSupported());
    std::cout << "Detect Vulkan Supported: " << ok << std::endl;
    if (!ok)
        return false;
    */
    ////////////////////////////////////////////////////////////
    // Init Window
    ////////////////////////////////////////////////////////////
    if (!glfwInit())
        throw std::runtime_error("Window creation fatal error:  glfw init failed!");
    
    glfwSetErrorCallback(GlfwErrorCallBack);

    // Create Platform Specify Window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // bydefault, glfwCreateWindow will create window and proper opengl(es) context
                                                // by set GLFW_CLIENT_API hint, we tell glfw create window for vulkan not gl
    glfwWindowHint(GLFW_RESIZABLE, wndDesc.resizeble);


    m_GlfwWindow = glfwCreateWindow(wndDesc.windowWidth, 
                                    wndDesc.windowHeight,
                                    wndDesc.name, 
                                    wndDesc.fullScreenMode ? glfwGetPrimaryMonitor() : nullptr, 
                                    nullptr);

    glfwSetWindowUserPointer(m_GlfwWindow, this);

    glfwSetWindowSizeCallback(m_GlfwWindow, ResizeCallback);

    
    if (!m_GlfwWindow)
        throw std::runtime_error("Window creation fatal error: create glfw window failed!");
}


Window::~Window()
{
    if (VKHANDLE_IS_NOT_NULL(m_VKSurface))
        throw std::runtime_error("Try to destory window when has unrelease surface!");
    
    if (m_GlfwWindow)   
    {  
        glfwDestroyWindow(m_GlfwWindow);
        m_GlfwWindow = nullptr;
        glfwTerminate();
    }
}


VkSurfaceKHR Window::CreateVulkanSurface(VkInstance vkInstance)
{
    glfwCreateWindowSurface(vkInstance, m_GlfwWindow, nullptr, &m_VKSurface);    
    return m_VKSurface;
}


bool Window::DestroyVulkanSurface(VkInstance instance)
{
    if (VKHANDLE_IS_NOT_NULL(m_VKSurface))
    {   
        vkDestroySurfaceKHR(instance, m_VKSurface, nullptr);
        VKHANDLE_SET_NULL(m_VKSurface);
        return true;
    }

    return false;
}


bool Window::ShouldClose() const
{
    return glfwWindowShouldClose(m_GlfwWindow);
}

void Window::Close()
{
    glfwSetWindowShouldClose(m_GlfwWindow, 1);
}

void Window::ProcessEvent() const
{
    glfwPollEvents();
}

void Window::Resize(size_t width, size_t height)
{
    m_Width = width;
    m_Height = height;
    if (m_ResizeCallback)
        m_ResizeCallback(width, height);
}


 const char** Window::GetRequireVulkanInstanceExtendsion(uint32_t* extendsionCnt)
 {
    return glfwGetRequiredInstanceExtensions(extendsionCnt);
 }


