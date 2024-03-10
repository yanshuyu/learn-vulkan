#include<iostream>
#include"core\CommandBuffer.h"
#include"core\Buffer.h"
#include"core\Device.h"
#include"core\Fence.h"

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

bool CommandBuffer::Execute(Fence* fence)
{
    if (!_temprary)
    {
        LOGE("Command Buffer({}) is none-temprary commmand buffer, which is only execute by a render context, can't execute directly!", (void*)this);
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
    
    VkResult result = vkQueueSubmit(_vkQueue, 1, &submitInfo, fence ? fence->GetHandle() : VK_NULL_HANDLE);
    if (result != VK_SUCCESS)
    {
        LOGE("Command Buffer({}) Execut Error: {}", (void*)this, result);
        return false;
    }

    if (!fence)
        vkQueueWaitIdle(_vkQueue);
    
    return true;
}

bool CommandBuffer::Reset()
{
    if (_temprary)
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

bool CommandBuffer::CopyBuffer(const Buffer* src,
                      size_t srcOffset,
                      Buffer* dst,
                      size_t dstOffset,
                      size_t dataSz,
                      VkPipelineStageFlags waitStageMask,
                      VkAccessFlags waitAccessMask,
                      VkPipelineStageFlags signalStageMask,
                      VkAccessFlags signalAccessMask)
{
    if (_state != State::Recording)
        return false;

    // insert a buffer memory barrier to ensure transffer write wait for prev read finish
    VkBufferMemoryBarrier mb{VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
    mb.buffer = dst->GetHandle();
    mb.offset = dstOffset;
    mb.size = dataSz;
    mb.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mb.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    mb.srcAccessMask = waitAccessMask;
    mb.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    vkCmdPipelineBarrier(GetHandle(), waitStageMask, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &mb, 0, nullptr);

    // transffer staging buffer to dst buffer
    VkBufferCopy copyArea{};
    copyArea.srcOffset = srcOffset;
    copyArea.dstOffset = 0;
    copyArea.size = dataSz;
    vkCmdCopyBuffer(GetHandle(), src->GetHandle(), dst->GetHandle(), 1, &copyArea);

    // insert a buffer memory barrier to ensure next read wait for transffer write finish
    mb.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    mb.dstAccessMask = signalAccessMask;
    vkCmdPipelineBarrier(GetHandle(), VK_PIPELINE_STAGE_TRANSFER_BIT, signalStageMask, 0, 0, nullptr, 1, &mb, 0, nullptr);

    return true;
}