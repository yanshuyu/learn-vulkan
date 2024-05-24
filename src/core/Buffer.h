#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include"CoreUtils.h"
#include"core\IMapAccessMemory.h"
#include"VKDeviceResource.h"


struct BufferDesc
{
    VkDeviceSize size;
    VkBufferUsageFlags usage;
    VkMemoryPropertyFlags memFlags; // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT : GPU Memory that is not visible by CPU, it's fastest access from GPU (suitble for render targets, sample texures, static vertex/index buffers etc)
                                    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT : PUG Memory that might never need to allocated on tile-base renderer for render targets which are never store to (such as msaa/depth targets)
                                    // VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT : GPU Memory that CPU can write to directly (AMD limit up to 256M, suitble for uniform buffers, dynamic vertex/index buffers etc)
                                    // VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : CPU Memory that GPU can reads (suitble for uniform buffers, dynamic vertex/index buffers, staging buffers etc)
};





class Buffer : public IMapAccessMemory, VKDeviceResource
{
    friend class Device;

private:
    BufferDesc _Desc{};
    VkBuffer _Buffer{VK_NULL_HANDLE};
    VkDeviceMemory _BufferMem{VK_NULL_HANDLE};
    std::string _name{};

protected:
    bool _create(BufferDesc desc, const char* name = nullptr);
    void Release() override;

public:
    Buffer(Device* pDevice = nullptr);
    ~Buffer() { assert(!IsValid()); }

    NONE_COPYABLE_NONE_MOVEABLE(Buffer)

    VkDevice GetDeviceHandle() const override;
    VkDeviceMemory GetMemory() const override { return _BufferMem; }
    uint32_t GetMemorySize() const override { return _Desc.size; }
    VkMemoryPropertyFlags GetMemoryProperties() const override { return _Desc.memFlags; }
    VkBuffer GetHandle() const { return _Buffer; }
    BufferDesc GetDesc() const {return _Desc; }
    const char* GetName() const { return _name.c_str(); }

    bool IsValid() const override { return  VKHANDLE_IS_NOT_NULL(_Buffer) && VKHANDLE_IS_NOT_NULL(_BufferMem); }
    bool CanMap() const override { return IsValid() && IMapAccessMemory::CanMap(); }
};
