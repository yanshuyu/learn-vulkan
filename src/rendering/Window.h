#pragma once
#include"vulkan\vulkan.h"
#include"core\CoreUtils.h"
#include<functional>


class GLFWwindow;

struct WindowDesc
{
    const char* name;
    bool fullScreenMode;
    bool resizeble;
    size_t windowWidth;
    size_t windowHeight;
};

class Window
{
private:
    GLFWwindow* m_GlfwWindow{nullptr};
    VkSurfaceKHR m_VKSurface{VK_NULL_HANDLE};

    size_t m_Width;
    size_t m_Height;

    std::function<void(size_t, size_t)> m_ResizeCallback;

public:
    Window(const WindowDesc& desc);
    ~Window();

    NONE_COPYABLE_NONE_MOVEABLE(Window)

    VkSurfaceKHR CreateVulkanSurface(VkInstance instance);
    
    bool DestroyVulkanSurface(VkInstance instance);

    VkSurfaceKHR GetVulkanSurface() const { return m_VKSurface; }
    
    const char** GetRequireVulkanInstanceExtendsion(uint32_t* extendsionCnt);

    bool ShouldClose() const;

    void Close();

    void ProcessEvent() const;

    void Resize(size_t width, size_t height);

    void SetResizeCallback(std::function<void(size_t, size_t)> callback) { m_ResizeCallback = callback; }
 
    size_t GetWidth() const { return m_Width; }
    
    size_t GetHeight() const { return m_Height; }

    GLFWwindow* GetHandle() const { return m_GlfwWindow; }

};
