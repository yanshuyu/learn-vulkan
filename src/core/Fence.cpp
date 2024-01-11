#include"core\Fence.h"
#include"core\Device.h"

Fence::Fence(Device* pDevice, bool signaled)
: _pDevice(pDevice)
{
    VkFenceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
    VkResult result = vkCreateFence(_pDevice->GetHandle(), &createInfo, nullptr, &_vkFence);
    if (result != VK_SUCCESS)
        LOGE("Create Fence Error: {}", result);
}

Fence::Fence(Fence&& rval)
{
    _pDevice = rval._pDevice;
    _vkFence = rval._vkFence;
    rval._pDevice = nullptr;
    VKHANDLE_SET_NULL(rval._vkFence);
}


Fence& Fence::operator = (Fence&& rval)
{
   if (this != &rval)
   {
        std::swap(_pDevice, rval._pDevice);
        std::swap(_vkFence, rval._vkFence);
   }

   return *this;
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
        _pDevice = nullptr;
        VKHANDLE_SET_NULL(_vkFence);
    }
}