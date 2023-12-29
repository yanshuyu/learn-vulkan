#include"CommandBuffer.h"
#include<iostream>


CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool cmdPool, VkCommandBuffer cmdBuffer,  bool isTemprary)
: m_vkDevice(device)
, m_vkCmdPool(cmdPool)
, m_vkCmdBuffer(cmdBuffer)
, m_Temprary(isTemprary)
{
}


CommandBuffer::CommandBuffer(CommandBuffer&& other)
{
    if(this == &other)
        return;
    
    *this = std::move(other);
}


CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other)
{
    if (this == &other)
        return *this;
    
    std::swap(m_vkDevice, other.m_vkDevice);
    std::swap(m_vkCmdPool, other.m_vkCmdPool);
    std::swap(m_vkCmdBuffer, other.m_vkCmdBuffer);
    std::swap(m_Temprary, other.m_Temprary);
}


CommandBuffer::~CommandBuffer()
{
    Release();
}


void CommandBuffer::Begin() const
{
    if (!IsVaild())
        return;

    VkCommandBufferBeginInfo begInfo{};
    begInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begInfo.pNext = nullptr;
    begInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(m_vkCmdBuffer, &begInfo);
}


void CommandBuffer::Release()
{
    if (!IsVaild() || !m_Temprary)
        return;

    vkFreeCommandBuffers(m_vkDevice, m_vkCmdPool, 1, &m_vkCmdBuffer);
    m_vkCmdBuffer = VK_NULL_HANDLE;
    m_vkCmdPool = VK_NULL_HANDLE;
    m_vkDevice = VK_NULL_HANDLE;
    m_Temprary = false;
}