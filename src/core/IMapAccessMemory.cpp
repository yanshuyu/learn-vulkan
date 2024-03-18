#include"core\IMapAccessMemory.h"

uint8_t *IMapAccessMemory::Map(MapAcess mode)
{
    if (!CanMap())
        return nullptr;

    if (IsMapped())
    {
        if (_mapAccessMode != mode)
        {
            LOGE("Try to map already maped momery({}) with diffrent access mode!", (void *)this);
            return nullptr;
        }

        return _MappedData;
    }

    VkResult result = vkMapMemory(GetDeviceHandle(), GetMemory(), 0, VK_WHOLE_SIZE, 0, (void **)(&_MappedData));
    if (result != VK_SUCCESS)
    {
        LOGE("Memory({}) Map error: {}", (void *)GetMemory(), result)
        _MappedData = nullptr;
        return nullptr;
    }

    _mapAccessMode = mode;

    if (_mapAccessMode == Read)
        Flush();

    return _MappedData;
}

  void IMapAccessMemory::UnMap()
  {
      if (!CanMap() || !IsMapped())
          return;

      if (_mapAccessMode == Write)   
        Flush();

      vkUnmapMemory(GetDeviceHandle(), GetMemory());
      _MappedData = nullptr;
  }

  size_t IMapAccessMemory::SetData(uint8_t *data, size_t dataLen, size_t offset)
  {
     //Map();
    if (!IsMapped())
        return 0;

    size_t writeLen = std::min(dataLen, GetMemorySize() - offset);
    std::memcpy(_MappedData + offset, data, writeLen);
    //UnMap();

    return writeLen;
  }


  size_t IMapAccessMemory::GetData(uint8_t* buffer, size_t bufLen, size_t offset)
  {
    //Map();
    if (!IsMapped())
        return 0;

    size_t readLen = std::min(bufLen, GetMemorySize() - offset);
    std::memcpy(buffer, _MappedData + offset, readLen);

    return readLen;
  }

  void IMapAccessMemory::Flush()
  {
      if (IsCoherent() || !IsMapped())
          return;

      VkMappedMemoryRange memMapRange{};
      memMapRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
      memMapRange.pNext = nullptr;
      memMapRange.memory = GetMemory();
      memMapRange.offset = 0;
      memMapRange.size = VK_WHOLE_SIZE;
      if (_mapAccessMode == Read) // make device writes visible to client
        vkInvalidateMappedMemoryRanges(GetDeviceHandle(), 1, &memMapRange);
      else // make client writes visible to device
        vkFlushMappedMemoryRanges(GetDeviceHandle(), 1, &memMapRange); 
  }