#pragma once
#include<vulkan\vulkan.h>
#include"CoreUtils.h"
#include<vector>


class Device;

struct BufferDescription
{
    Device* device;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    bool readWriteEnable;
};



class Buffer
{
private:
    BufferDescription _Desc{};
    VkBuffer _Buffer{VK_NULL_HANDLE};
    VkDeviceMemory _BufferMem{VK_NULL_HANDLE};
    uint8_t* _MappedData{nullptr};

public:
    Buffer(){};
    ~Buffer(){ assert(!IsValid()); };

    NONE_COPYABLE_NONE_MOVEABLE(Buffer)

    Device* GetDevice() const { return _Desc.device; }
    VkBuffer GetHandle() const { return _Buffer; }
    VkDeviceMemory GetMemory() const { return _BufferMem; }
    
    bool Create(BufferDescription desc);
    bool IsValid() const { return _Desc.device != nullptr && VKHANDLE_IS_NOT_NULL(_Buffer) && VKHANDLE_IS_NOT_NULL(_BufferMem); }
    void Release();

    bool CanMap() const { return IsValid() && _Desc.readWriteEnable; }
    bool IsMapped() const { return _MappedData != nullptr; }
    uint8_t* Map();
    void UnMap();
    size_t SetData(uint8_t* data, size_t dataLen, size_t offset);
    size_t GetData(uint8_t* buffer, size_t bufLen, size_t offset);
};
