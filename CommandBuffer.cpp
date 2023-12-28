#include"CommandBuffer.h"
#include<iostream>


CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool cmdPool)
: m_vkDevice(device)
, m_vkCmdPool(cmdPool)
, m_vkCmdBuffer(VK_NULL_HANDLE)
{
}

CommandBuffer::~CommandBuffer()
{
    Release();
}


bool CommandBuffer::Create()
{
    if (m_vkDevice == VK_NULL_HANDLE || m_vkCmdPool == VK_NULL_HANDLE)
        return false;
    
    if (IsCreate())
        return;

    VkCommandBufferAllocateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.commandPool = m_vkCmdPool;
    createInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer cmdBuffer = VK_NULL_HANDLE;
    VkResult result = vkAllocateCommandBuffers(m_vkDevice, &createInfo, &cmdBuffer);
    if (result != VK_SUCCESS)
    {
        std::cout << "--> Create Command Buffer Failed! vulkan error: " << result << std::endl;
        return false;
    }
    

    return true;
}


void CommandBuffer::Begin() const
{
    if (!IsCreate())
        return;

    VkCommandBufferBeginInfo begInfo{};
    begInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begInfo.pNext = nullptr;
    begInfo.pInheritanceInfo = nullptr;

    vkBeginCommandBuffer(m_vkCmdBuffer, &begInfo);
}


void CommandBuffer::Release()
{
    if (!Create())
        return;

    vkFreeCommandBuffers(m_vkDevice, m_vkCmdPool, 1, &m_vkCmdBuffer);
    m_vkCmdBuffer = VK_NULL_HANDLE;
    m_vkCmdPool = VK_NULL_HANDLE;
    m_vkDevice = VK_NULL_HANDLE;
}