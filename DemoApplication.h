#pragma once
#include<string>
#include<vulkan\vulkan.h>
#include<GLFW\glfw3.h>
#include"QueueFamilyIndices.h"
#include"SwapChain.h"


class DemoApplication
{
    // GLFW Window
private:
    int m_WndWidth;
    int m_WndHeight;
    std::string m_WndTitle;
    GLFWwindow* m_WndHandle;

    // Vulkan Device
private:
    VkSurfaceKHR m_vkSurface;
    VkInstance m_vkInstance;
    VkDebugUtilsMessengerEXT m_vkDebugMsger;
    VkDevice m_vkDevice;
    QueueFamilyIndices m_vkDeviceQueueFamilyIndices;
    VkQueue m_vkDeviceGraphicQueue;
    VkQueue m_vkDevicePresentQueue;
    SwapChain m_SwapChain;

public:
    DemoApplication(const std::string& wndTitle, int wndWidth, int wndHeight);
    ~DemoApplication();

    DemoApplication(const DemoApplication&) = delete;
    DemoApplication& operator = (const DemoApplication&) = delete;

private:
    static void GlfwErrorCallBack(int errCode, const char* errMsg);
    bool SystemSetUp(); // window/vulkan initailtion
    void SystemCleanUp(); // window/vulkan clean up
    bool Start(); // client initailtion
    void Update(double dt); // client logic update
    void Render(); // render loop
    void Release(); // client clean up

public:
 
    bool Init();
    void Run();
    void ShutDown();

};

