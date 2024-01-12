#include"core\CommandBuffer.h"
#include<iostream>


CommandBuffer::~CommandBuffer()
{
    if (IsVaild())
    {
        LOGW("Command Buffer({}) destructed in valid state, may have ressource leak!", (void*)this);
    }
}


void CommandBuffer::SetUp(Device* pDevice, VkCommandPool cmdPool, VkQueue exeQueue, VkCommandBuffer cmdBuf, bool isTemp)
{
    if (IsVaild())
    {
        LOGW("Command Buffer({}) is reset up in valid state, may have ressource leak!", (void*)this);
    }

    _pDevice = pDevice;
    _vkCmdPool = cmdPool;
    _vkQueue = exeQueue;
    _vkCmdBuffer = cmdBuf;
    _temprary = isTemp;
    _state = State::Initial;
}

void CommandBuffer::ClenUp()
{
    VKHANDLE_SET_NULL(_pDevice);
    VKHANDLE_SET_NULL(_vkCmdPool);
    VKHANDLE_SET_NULL(_vkQueue);
    VKHANDLE_SET_NULL(_vkCmdBuffer);
    _state = State::Invalid;
}

bool CommandBuffer::IsVaild() const
{
    return _pDevice != nullptr 
            && VKHANDLE_IS_NOT_NULL(_vkCmdPool) 
            && VKHANDLE_IS_NOT_NULL(_vkQueue) 
            && VKHANDLE_IS_NOT_NULL(_vkCmdBuffer);
}

bool CommandBuffer::Begin()
{
    if (!IsVaild() || _state != State::Initial)
        return false;

    VkCommandBufferBeginInfo begInfo{};
    begInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    begInfo.pNext = nullptr;
    begInfo.pInheritanceInfo = nullptr;
    begInfo.flags = _temprary ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : 0;

    VkResult result = vkBeginCommandBuffer(_vkCmdBuffer, &begInfo);
    if (result != VK_SUCCESS)
    {    
        LOGE("Command Buffer({}) Begin Error: {}", (void*)this, result);
    }
    else
    {
        _state = State::Recording;
    }

    return result == VK_SUCCESS;
}


bool CommandBuffer::End()
{
    if (!IsVaild() || _state != State::Recording)
        return false;
    
    VkResult result = vkEndCommandBuffer(_vkCmdBuffer);
    if (result != VK_SUCCESS)
    {    
        LOGE("Command Buffer({}) End Error: {}", (void*)this, result);
    }
    else
    {
        _state = State::Executable;
    }

    return result == VK_SUCCESS;
}

bool CommandBuffer::Execute()
{
    if (!_temprary)
    {
        LOGE("Command Buffer({}) is none-temprary commmand buffer, can't execute directly!");
        return false;
    }

    if (!IsVaild() || _state != State::Executable)
        return false;
    
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext = nullptr;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &_vkCmdBuffer;
    submitInfo.waitSemaphoreCount = 0;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.signalSemaphoreCount = 0;
    submitInfo.pSignalSemaphores = nullptr;
    
    VkResult result = vkQueueSubmit(_vkQueue, 1, &submitInfo, VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        LOGE("Command Buffer({}) Execut Error: {}", (void*)this, result);
        return false;
    }

    vkQueueWaitIdle(_vkQueue);
    return true;
}

bool CommandBuffer::Reset()
{
    if (!_temprary)
        return false;

    VkResult result = vkResetCommandBuffer(_vkCmdBuffer, 0);
    if (result != VK_SUCCESS)
    {
        LOGE("Command Buffer({}) Reset Error: {}", (void*)this, result);
        return false;
    }

    _state = State::Initial;
    return true;
}