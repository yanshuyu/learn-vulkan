#include<iostream>
#include"core\CommandBuffer.h"
#include"core\Buffer.h"
#include"core\Device.h"
#include"core\Fence.h"

CommandBuffer::CommandBuffer(Device* pDevice)
: VKDeviceResource(pDevice)
{

}


bool CommandBuffer::_create(VkCommandPool cmdPool, VkQueue exeQueue, bool isTemp)
{

    // alloc one resetable command buffer
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = cmdPool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer cmdBuffer{VK_NULL_HANDLE};
    VkResult result = vkAllocateCommandBuffers(_pDevice->GetHandle(), &allocInfo, &cmdBuffer);
    if (result != VK_SUCCESS)
    {
        LOGE("Alloc command buffer error: {}", result);
        return false;
    }
    _vkCmdPool = cmdPool;
    _vkQueue = exeQueue;
    _vkCmdBuffer = cmdBuffer;
    _temprary = isTemp;
    _state = State::Initial;
    return true;
}


bool CommandBuffer::IsValid() const
{
    return VKHANDLE_IS_NOT_NULL(_vkCmdPool) 
            && VKHANDLE_IS_NOT_NULL(_vkQueue) 
            && VKHANDLE_IS_NOT_NULL(_vkCmdBuffer)
            && _state != State::Invalid;
}

bool CommandBuffer::Begin()
{
    if (!IsValid() || _state != State::Initial)
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
    if (!IsValid() || _state != State::Recording)
        return false;
    
    VkResult result = vkEndCommandBuffer(_vkCmdBuffer);
    if (result != VK_SUCCESS)
    {   
        _state = State::Invalid; 
        LOGE("Command Buffer({}) End Error: {}", (void*)this, result);
    }
    else
    {
        _state = State::Executable;
    }

    return result == VK_SUCCESS;
}

bool CommandBuffer::_execute(Fence* fence)
{
    if (!_temprary)
    {
        LOGE("Command Buffer({}) is none-temprary commmand buffer, which is only execute by a render context, can't execute directly!", (void*)this);
        return false;
    }

    if (!IsValid() || _state != State::Executable)
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
    {
        _state = State::Invalid;
        return false;
    }

    VkResult result = vkResetCommandBuffer(_vkCmdBuffer, 0);
    if (result != VK_SUCCESS)
    {
        LOGE("Command Buffer({}) Reset Error: {}", (void*)this, result);
        return false;
    }

    _state = State::Initial;
    return true;
}

void CommandBuffer::Release()
{
    if (!IsValid())
        return;

    vkFreeCommandBuffers(_pDevice->GetHandle(), _vkCmdPool, 1, &_vkCmdBuffer);
    VKHANDLE_SET_NULL(_vkCmdPool);
    VKHANDLE_SET_NULL(_vkQueue);
    VKHANDLE_SET_NULL(_vkCmdBuffer);
    _state = State::Invalid;
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

void CommandBuffer::TransitionLayout(VkImage img,
                                     VkImageSubresourceRange imgSubRange,
                                     VkImageLayout oldLayout,
                                     VkImageLayout newLayout,
                                     VkPipelineStageFlags srcStageMask,
                                     VkPipelineStageFlags dstStageMas)
{
    if (oldLayout == newLayout)
        return;

    VkImageMemoryBarrier imgBarrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    imgBarrier.image = img;
    imgBarrier.subresourceRange = imgSubRange;
    imgBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    imgBarrier.oldLayout = oldLayout;
    imgBarrier.newLayout = newLayout;
    
    // Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
    switch (oldLayout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        imgBarrier.srcAccessMask = VK_ACCESS_NONE;
        break;
    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        imgBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        imgBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        imgBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
        imgBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        imgBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        imgBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;
    default:
        break;
    }

    // Destination access mask controls the dependency for the new image layout
    switch (newLayout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        imgBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL: // undefine -> shader read
        if (imgBarrier.srcAccessMask == VK_ACCESS_NONE)
            imgBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        imgBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    case VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL:
        imgBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        imgBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;
    case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        imgBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    default:
        break;
    }

    vkCmdPipelineBarrier(_vkCmdBuffer, srcStageMask, dstStageMas, 0, 0, nullptr, 0, nullptr, 1, &imgBarrier);

}



void CommandBuffer::TransitionLayout( CommandBuffer* cmd,
                        VkImage img,
                        VkImageSubresourceRange imgSubRange,
                        VkImageLayout oldLayout,
                        VkImageLayout newLayout,
                        VkPipelineStageFlags srcStageMask,
                        VkPipelineStageFlags dstStageMask)
{
    cmd->Begin();
    cmd->TransitionLayout(img, imgSubRange, oldLayout, newLayout, srcStageMask, dstStageMask);
    cmd->End();
    cmd->ExecuteSync();   
}