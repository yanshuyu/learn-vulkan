#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include"CoreUtils.h"
#include"core\IMapAccessMemory.h"

class Device;

struct BufferDesc
{
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memFlags; // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : GPU Memory that is not visible by CPU, it's fastest access from GPU (suitble for render targets, sample texures, static vertex/index buffers etc)
                                    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : PUG Memory that might never need to allocated on tile-base renderer for render targets which are never store to (such as msaa/depth targets)
                                    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : GPU Memory that CPU can write to directly (AMD limit up to 256M, suitble for uniform buffers, dynamic vertex/index buffers etc)
                                    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : CPU Memory that GPU can reads (suitble for uniform buffers, dynamic vertex/index buffers, staging buffers etc)
};





class Buffer : public IMapAccessMemory
{
    friend class Device;

private:
    Device* _Device{nullptr};
    BufferDesc _Desc{};
    VkBuffer _Buffer{VK_NULL_HANDLE};
    VkDeviceMemory _BufferMem{VK_NULL_HANDLE};

protected:
    void Reset()
    {
        _Device = nullptr;
        _Desc = {};
        VKHANDLE_SET_NULL(_Buffer);
        VKHANDLE_SET_NULL(_BufferMem);
    }

public:
    Buffer():IMapAccessMemory() {}
    ~Buffer() { assert(!IsValid()); }

    NONE_COPYABLE_NONE_MOVEABLE(Buffer)

    Device* GetDevice() const { return _Device; }
    VkDevice GetDeviceHandle() const override;
    VkDeviceMemory GetMemory() const override { return _BufferMem; }
    uint32_t GetMemorySize() const override { return _Desc.size; }
    VkMemoryPropertyFlags GetMemoryProperties() const override { return _Desc.memFlags; }
    VkBuffer GetHandle() const { return _Buffer; }
    BufferDesc GetDesc() const {return _Desc; }

    bool Create(Device* pDevice, BufferDesc desc);
    bool IsValid() const { return _Device != nullptr && VKHANDLE_IS_NOT_NULL(_Buffer) && VKHANDLE_IS_NOT_NULL(_BufferMem); }
    void Release();

    bool CanMap() const override { return IsValid() && IMapAccessMemory::CanMap(); }

};
