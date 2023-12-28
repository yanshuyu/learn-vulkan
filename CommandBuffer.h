#pragma once
#include<vulkan\vulkan.h>


class CommandBuffer
{
private:
    VkDevice m_vkDevice;
    VkCommandPool m_vkCmdPool;
    VkCommandBuffer m_vkCmdBuffer;

public:
    CommandBuffer(VkDevice device, VkCommandPool cmdPool);
    ~CommandBuffer();

    bool Create();
    bool IsCreate() const { return m_vkCmdBuffer != VK_NULL_HANDLE; }
    void Release();

    void Begin() const;
    void End() const { if(IsCreate()) vkEndCommandBuffer(m_vkCmdBuffer); }
    void Reset() const { if(IsCreate()) vkResetCommandBuffer(m_vkCmdBuffer, 0); }

};


