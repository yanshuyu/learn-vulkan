#pragma once
#include"core\CoreUtils.h"
#include"core\VulkanInstance.h"
#include"core\Device.h"
#include"core\SwapChain.h"


class GLFWwindow;



class Application
{
    // GLFW Window
private:
    int m_WndWidth;
    int m_WndHeight;
    std::string m_WndTitle;
    GLFWwindow* m_WndHandle{nullptr};

   // vulkan
private:
    VulkanInstance m_VukanInstance{};
    Device m_Device{};
    SwapChain m_SwapChain{};        
    VkSurfaceKHR m_vkSurface{VK_NULL_HANDLE};
    
public:
    Application(const char* wndTitle, int wndWidth, int wndHeight);
    virtual ~Application() {};

    virtual bool RenderingSetUp();
    virtual void Render() {};
    virtual void RenderingCleanUp();

    virtual bool ApplicationSetUp() { return true; };
    virtual void ApplicationUpdate() {};
    virtual void ApplicationCleanUp() {};

    virtual bool Init() 
    {
        if (!RenderingSetUp())
        {
            LOGE("--->Rendering Set Up Failed!");
            return false;
        }

        if (!ApplicationSetUp())
        {
            LOGE("--->Application Set Up Failed!");
            return false;
        }

        return true;
    }
    virtual void Run();

    virtual void ShutDown()
    {
        ApplicationCleanUp();

        RenderingCleanUp();

    };

};