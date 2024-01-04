#pragma once
#include<vulkan\vulkan.h>


class CommandBuffer
{
private:
    VkDevice m_vkDevice;
    VkCommandPool m_vkCmdPool;
    VkCommandBuffer m_vkCmdBuffer;
    bool m_Temprary;
public:
    CommandBuffer() = delete;
    CommandBuffer(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer, bool isTemprary);
    CommandBuffer(const CommandBuffer& other) = delete;
    CommandBuffer& operator = (const CommandBuffer& other) = delete;
    CommandBuffer(CommandBuffer&& other);
    CommandBuffer& operator = (CommandBuffer&& other);
    ~CommandBuffer();

    bool IsTemprary() const { return m_Temprary; }
    bool IsVaild() const { return m_vkCmdBuffer != VK_NULL_HANDLE; }
    void Begin() const;
    void End() const { if(IsVaild()) vkEndCommandBuffer(m_vkCmdBuffer); }
    void Reset() const { if(IsVaild() && !m_Temprary) vkResetCommandBuffer(m_vkCmdBuffer, 0); }
    void Release();

};


