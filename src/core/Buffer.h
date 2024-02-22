#pragma once
#include<vulkan\vulkan.h>
#include"CoreUtils.h"
#include<vector>


class Device;

struct BufferDesc
{
    Device* device;
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memFlags; // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : GPU Memory that is not visible by CPU, it's fastest access from GPU (suitble for render targets, sample texures, static vertex/index buffers etc)
                                    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : PUG Memory that might never need to allocated on tile-base renderer for render targets which are never store to (such as msaa/depth targets)
                                    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : GPU Memory that CPU can write to directly (AMD limit up to 256M, suitble for uniform buffers, dynamic vertex/index buffers etc)
                                    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : CPU Memory that GPU can reads (suitble for uniform buffers, dynamic vertex/index buffers, staging buffers etc)
};





class Buffer
{
    friend Device;

private:
    BufferDesc _Desc{};
    VkBuffer _Buffer{VK_NULL_HANDLE};
    VkDeviceMemory _BufferMem{VK_NULL_HANDLE};
    uint8_t* _MappedData{nullptr};

protected: 
    void Flush() const;
    void Reset();

public:
    Buffer(){};
    ~Buffer(){ assert(!IsValid()); };

    NONE_COPYABLE_NONE_MOVEABLE(Buffer)

    Device* GetDevice() const { return _Desc.device; }
    VkBuffer GetHandle() const { return _Buffer; }
    VkDeviceMemory GetMemory() const { return _BufferMem; }
    
    bool Create(BufferDesc desc);
    bool IsValid() const { return _Desc.device != nullptr && VKHANDLE_IS_NOT_NULL(_Buffer) && VKHANDLE_IS_NOT_NULL(_BufferMem); }
    void Release();

    bool CanMap() const { return IsValid() && (_Desc.memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT != 0 ); }
    bool IsCoherent() const { return CanMap() && (_Desc.memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT != 0); }
    bool IsMapped() const { return _MappedData != nullptr; }
    uint8_t* Map();
    void UnMap();
    size_t SetData(uint8_t* data, size_t dataLen, size_t offset);
    size_t GetData(uint8_t* buffer, size_t bufLen, size_t offset);
};
