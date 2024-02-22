#include"Buffer.h"
#include"Device.h"
#include<algorithm>



bool Buffer::Create(BufferDesc desc)
{
    if (IsValid())
        return true;

    if (desc.device == nullptr || !desc.device->IsValid())
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
    VkResult result = vkCreateBuffer(desc.device->GetHandle(), &createInfo, nullptr, &createdBuffer);
    if (result != VK_SUCCESS)
    {
       LOGE("Device({}) Create Buffer error: {}", (void*)desc.device, result);
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetBufferMemoryRequirements(desc.device->GetHandle(), createdBuffer, &memReq);

    
    VkDeviceMemory allocedMem = VK_NULL_HANDLE;
    VkMemoryPropertyFlags memProp = desc.memFlags;
    if (!desc.device->AllocMemory(memReq.memoryTypeBits, memProp, memReq.size, &allocedMem))
        return false;

    vkBindBufferMemory(desc.device->GetHandle(), createdBuffer, allocedMem, 0); //TODO: does offet need match to memreq's offset ?

    _Desc = desc;
    _Buffer = createdBuffer;
    _BufferMem = allocedMem;

    return true;
  }


  void Buffer::Release()
  {
    if (!IsValid())
        return;

    vkDestroyBuffer(_Desc.device->GetHandle(), _Buffer, nullptr);
    vkFreeMemory(_Desc.device->GetHandle(), _BufferMem, nullptr);
    _Desc = {};
    VKHANDLE_SET_NULL(_Buffer);
    VKHANDLE_SET_NULL(_BufferMem);

  }

  uint8_t *Buffer::Map()
  {
      if (!CanMap())
          return nullptr;

      if (IsMapped())
          return _MappedData;

      Flush();

      VkResult result = vkMapMemory(_Desc.device->GetHandle(), _BufferMem, 0, VK_WHOLE_SIZE, 0, (void **)(&_MappedData));
      if (result != VK_SUCCESS)
      {
          LOGE("Buffer({}) Map error: {}", (void *)this, result)
          _MappedData = nullptr;
          return nullptr;
      }

      return _MappedData;
  }

  void Buffer::UnMap()
  {
      if (!CanMap() || !IsMapped())
          return;

      Flush();

      vkUnmapMemory(_Desc.device->GetHandle(), _BufferMem);
      _MappedData = nullptr;
  }

  size_t Buffer::SetData(uint8_t *data, size_t dataLen, size_t offset)
  {
     //Map();
    if (!IsMapped())
        return 0;

    size_t writeLen = std::min(dataLen, _Desc.size - offset);
    std::memcpy(_MappedData + offset, data, writeLen);
    //UnMap();

    return writeLen;
  }


  size_t Buffer::GetData(uint8_t* buffer, size_t bufLen, size_t offset)
  {
    Map();
    if (!IsMapped())
        return 0;

    size_t readLen = std::min(bufLen, _Desc.size - offset);
    std::memcpy(buffer, _MappedData + offset, readLen);

    return readLen;
  }

  void Buffer::Flush() const
  {
      if (IsCoherent())
          return;

      VkMappedMemoryRange memMapRange{};
      memMapRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      memMapRange.pNext = nullptr;
      memMapRange.memory = GetMemory();
      memMapRange.offset = 0;
      memMapRange.size = VK_WHOLE_SIZE;
      if (!IsMapped()) // make device writes visible to client
        vkInvalidateMappedMemoryRanges(GetDevice()->GetHandle(), 1, &memMapRange);
      else // make client writes visible to device
        vkFlushMappedMemoryRanges(GetDevice()->GetHandle(), 1, &memMapRange); 
  }

  void Buffer::Reset()
  {
    _Desc = {};
    _Buffer = VK_NULL_HANDLE;
    _BufferMem =VK_NULL_HANDLE;
    _MappedData = nullptr;
  }