#include<stdexcept>
#include"rendering\Window.h"
#include"GLFW\glfw3.h"
#include"input\InputManager.h"



static void GlfwErrorCallBack(int errCode, const char* errMsg)
{
    LOGE("GLFW Error: {}  Msg: {}", errCode, errMsg);
}

static void ResizeCallback(GLFWwindow* window, int width, int height)
{
   Window* pWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
   pWindow->Resize(width, height);
}

static void KeyBoardCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{ 
    InputManager::EnjectKey(key, action);
}


static void CursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
    InputManager::EnJectCursorPos(xpos, ypos);
}

static void MouseBtnCallback(GLFWwindow* window, int button, int action, int mods)
{
    InputManager::EnjectMouse(button, action);
}

static void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    InputManager::EnjectMouseScroll(yoffset);
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
                                        
    if (!m_GlfwWindow)
        throw std::runtime_error("Window creation fatal error: create glfw window failed!");

    glfwSetWindowUserPointer(m_GlfwWindow, this);

    glfwSetWindowSizeCallback(m_GlfwWindow, ResizeCallback);

    InputManager::Reset();
    glfwSetKeyCallback(m_GlfwWindow, KeyBoardCallback);
    glfwSetCursorPosCallback(m_GlfwWindow, CursorPosCallback);
    glfwSetMouseButtonCallback(m_GlfwWindow, MouseBtnCallback);
    glfwSetScrollCallback(m_GlfwWindow, MouseScrollCallback);

    int w, h;
    glfwGetFramebufferSize(m_GlfwWindow, &w, &h);
    m_Width = w;
    m_Height = h;

    m_Desc = wndDesc;

}


Window::~Window()
{
    assert(VKHANDLE_IS_NULL(m_VKSurface));
        
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


 void Window::SetTitle(const char* title)
 {
    glfwSetWindowTitle(m_GlfwWindow, title);
 }


