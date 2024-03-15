#include"Buffer.h"
#include"Device.h"
#include<algorithm>


Buffer::Buffer(Device* pDevice): VKDeviceResource(pDevice), IMapAccessMemory()
{

}

VkDevice Buffer::GetDeviceHandle() const 
{ 
    return _pDevice->GetHandle(); 
}

bool Buffer::_create(BufferDesc desc)
{
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = desc.size;
    createInfo.usage = desc.usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices = nullptr;
    
    VkBuffer createdBuffer = VK_NULL_HANDLE;
    VkResult result = vkCreateBuffer(_pDevice->GetHandle(), &createInfo, nullptr, &createdBuffer);
    if (result != VK_SUCCESS)
    {
        LOGE("Device({}) Create Buffer error: {}", (void*)_pDevice, result);
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(_pDevice->GetHandle(), createdBuffer, &memReq);
    VkDeviceMemory allocedMem = VK_NULL_HANDLE;
    VkMemoryPropertyFlags memProp = desc.memFlags;
    if (!_pDevice->AllocMemory(memReq, memProp, &allocedMem))
    {
        vkDestroyBuffer(_pDevice->GetHandle(), createdBuffer, nullptr);
        LOGE("Device({}) alloc memory error!", (void*)_pDevice);
        return false;
    }

    if(VKCALL_FAILED(vkBindBufferMemory(_pDevice->GetHandle(), createdBuffer, allocedMem, 0)))//TODO: does offet need match to memreq's offset ?
    {
        vkDestroyBuffer(_pDevice->GetHandle(), createdBuffer, nullptr);
        _pDevice->FreeMemory(allocedMem);
        LOGE("Buffer({}) memory({}) bind error!", (void*)createdBuffer, (void*)allocedMem);
        return false;
    }

    _Desc = desc;
    _Buffer = createdBuffer;
    _BufferMem = allocedMem;

    return true;
  }


  void Buffer::Release()
  {
    if (!IsValid())
        return;

    vkDestroyBuffer(_pDevice->GetHandle(), _Buffer, nullptr);
    vkFreeMemory(_pDevice->GetHandle(), _BufferMem, nullptr);
    VKHANDLE_SET_NULL(_Buffer);
    VKHANDLE_SET_NULL(_BufferMem);
  }
