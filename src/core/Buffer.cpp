#include"Buffer.h"
#include"Device.h"
#include<algorithm>



bool Buffer::Initailize(Device* pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp)
{
    if (IsValid())
        return true;

    if (pDevice == nullptr || !pDevice->IsValid())
    {
        LOGE("Try to create Buffer with an invalid Device instance!");
        return false;
    }

    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.size = size;
    createInfo.usage = usage;
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
    if (!pDevice->AllocMemory(memReq.memoryTypeBits, memProp, memReq.size, &allocedMem))
        return false;

    vkBindBufferMemory(pDevice->GetHandle(), createdBuffer, allocedMem, 0); //TODO: does offet need match to memreq's offset ?

    _pDevice = pDevice;
    _Buffer = createdBuffer;
    _BufferMem = allocedMem;
    _MemProp = memProp;
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
    _pDevice = nullptr;
  }

  uint8_t *Buffer::Map()
  {
      if (!CanMap())
          return nullptr;

      if (IsMapped())
          return _MappedAdrr;

      VkResult result = vkMapMemory(_pDevice->GetHandle(), _BufferMem, 0, VK_WHOLE_SIZE, 0, (void **)(&_MappedAdrr));
      if (result != VK_SUCCESS)
      {
          LOGE("Buffer({}) Map error: {}", (void *)this, result)
          _MappedAdrr = nullptr;
          return nullptr;
      }

      // make sure device's write to the memory visible to host when using none coherent map
      if (!IsCoherent())
      {
          VkMappedMemoryRange memRange{};
          memRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
          memRange.pNext = nullptr;
          memRange.memory = _BufferMem;
          memRange.offset = 0;
          memRange.size = VK_WHOLE_SIZE;
          vkInvalidateMappedMemoryRanges(_pDevice->GetHandle(),
                                         1,
                                         &memRange);
      }

      return _MappedAdrr;
  }

  void Buffer::UnMap()
  {
      if (!CanMap() || !IsMapped())
          return;

      // make sure host's write to the memory visible to device when using coherent map
      if (!IsCoherent())
      {
          VkMappedMemoryRange memRange{};
          memRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
          memRange.pNext = nullptr;
          memRange.memory = _BufferMem;
          memRange.offset = 0;
          memRange.size = VK_WHOLE_SIZE;
          vkFlushMappedMemoryRanges(_pDevice->GetHandle(),
                                    1,
                                    &memRange);
      }

      vkUnmapMemory(_pDevice->GetHandle(), _BufferMem);
      _MappedAdrr = nullptr;
  }

  bool Buffer::Update(uint8_t *data, size_t dataLen, size_t offset)
  {
     Map();
     if (!IsMapped())
        return false;

     std::copy(data, data + dataLen, _MappedAdrr + offset);
     UnMap();
  }