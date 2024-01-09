#pragma once
#include<vulkan\vulkan.h>
#include"CoreUtils.h"
#include<vector>


class Device;

class Buffer
{
private:
    Device* _pDevice{nullptr};
    VkBuffer _Buffer{VK_NULL_HANDLE};
    VkDeviceMemory _BufferMem{VK_NULL_HANDLE};

    VkMemoryMapFlags _MemProp{0};
    uint8_t* _MappedAdrr{nullptr};

public:
    Buffer(){};
    ~Buffer(){ assert(!IsValid()); };

    NONE_COPYABLE_NONE_MOVEABLE(Buffer)

    Device* GetDevice() const { return _pDevice; }
    VkBuffer GetHandle() const { return _Buffer; }
    VkDeviceMemory GetMemory() const { return _BufferMem; }
    
    bool Initailize(Device* pDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp);
    bool IsValid() const { return _pDevice != nullptr && VKHANDLE_IS_NOT_NULL(_Buffer) && VKHANDLE_IS_NOT_NULL(_BufferMem); }
    void Release();

    bool CanMap() const { return IsValid() && IsHostVisible(); }
    bool IsMapped() const { return _MappedAdrr != nullptr; }
    uint8_t* Map();
    void UnMap();
    bool Update(uint8_t* data, size_t dataLen, size_t offset);

private:
    bool IsHostVisible() const { return _MemProp & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT > 0; }
    bool IsCoherent() const { return _MemProp & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT > 0; }
};
