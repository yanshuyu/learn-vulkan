#include"Buffer.h"
#include"Device.h"
#include<algorithm>


VkDevice Buffer::GetDeviceHandle() const 
{ 
    return _Device->GetHandle(); 
}

bool Buffer::Create(Device* pDevice, BufferDesc desc)
{
    if (IsValid())
        return false;

    if (pDevice == nullptr || !pDevice->IsValid())
    {
        LOGE("Try to create Buffer with an invalid Device instance!");
        return false;
    }

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
    VkResult result = vkCreateBuffer(pDevice->GetHandle(), &createInfo, nullptr, &createdBuffer);
    if (result != VK_SUCCESS)
    {
       LOGE("Device({}) Create Buffer error: {}", (void*)pDevice, result);
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(pDevice->GetHandle(), createdBuffer, &memReq);
    VkDeviceMemory allocedMem = VK_NULL_HANDLE;
    VkMemoryPropertyFlags memProp = desc.memFlags;
    if (!pDevice->AllocMemory(memReq, memProp, &allocedMem))
    {
        vkDestroyBuffer(pDevice->GetHandle(), createdBuffer, nullptr);
        LOGE("Device({}) alloc memory error!", (void*)pDevice);
        return false;
    }

    if(VKCALL_FAILED(vkBindBufferMemory(pDevice->GetHandle(), createdBuffer, allocedMem, 0)))//TODO: does offet need match to memreq's offset ?
    {
        vkDestroyBuffer(pDevice->GetHandle(), createdBuffer, nullptr);
        pDevice->FreeMemory(allocedMem);
        LOGE("Buffer({}) memory({}) bind error!", (void*)createdBuffer, (void*)allocedMem);
        return false;
    }

    _Device = pDevice;
    _Desc = desc;
    _Buffer = createdBuffer;
    _BufferMem = allocedMem;

    return true;
  }


  void Buffer::Release()
  {
    if (!IsValid())
        return;

    vkDestroyBuffer(_Device->GetHandle(), _Buffer, nullptr);
    vkFreeMemory(_Device->GetHandle(), _BufferMem, nullptr);
    Reset();
  }
