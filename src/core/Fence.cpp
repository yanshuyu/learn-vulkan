#include"core\Fence.h"
#include"core\Device.h"

Fence::Fence(Device* pDevice)
: VKDeviceResource(pDevice)
{
}

 bool Fence::_create(bool signaled)
 {
    VkFenceCreateInfo createInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VkFence createdFence{VK_NULL_HANDLE};
    VkResult result = vkCreateFence(_pDevice->GetHandle(), &createInfo, nullptr, &createdFence);
    if (result != VK_SUCCESS)
    {    
        LOGE("Create Fence Error: {}", result);
        return false;
    }

    _vkFence = createdFence;
    return true;
 }


void Fence::Reset() const
{
    if (IsValid())
        vkResetFences(_pDevice->GetHandle(), 1, &_vkFence);
}


void Fence::Wait(uint64_t timeOut) const
{
    if (IsValid())
        vkWaitForFences(_pDevice->GetHandle(), 1, &_vkFence, true, timeOut);
}


bool Fence::IsSignaled() const
{
    if (!IsValid())
        return false;

    VkResult result = vkGetFenceStatus(_pDevice->GetHandle(), _vkFence);
    return result == VK_SUCCESS;
}


void Fence::Release()
{
    if (IsValid())
    {
        vkDestroyFence(_pDevice->GetHandle(), _vkFence, nullptr);
        VKHANDLE_SET_NULL(_vkFence);
    }
}