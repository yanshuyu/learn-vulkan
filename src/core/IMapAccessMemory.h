#pragma once
#include"core\CoreUtils.h"

enum MapAcess
{
    Read,
    Write,
};

class IMapAccessMemory
{
protected:
    uint8_t* _MappedData{nullptr};
    MapAcess _mapAccessMode{Read};
    
    void Flush();

public:
    IMapAccessMemory() {};
    virtual ~IMapAccessMemory() {};

    virtual VkDeviceMemory GetMemory() const = 0;
    virtual uint32_t GetMemorySize() const = 0;
    virtual VkMemoryPropertyFlags GetMemoryProperties() const = 0;
    virtual VkDevice GetDeviceHandle() const = 0;

    virtual bool CanMap() const { return GetMemoryProperties() & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT; }
    bool IsCoherent() const { return GetMemoryProperties() & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT; }
    bool IsMapped() const { return _MappedData != nullptr; }
    uint8_t* Map(MapAcess mode);
    void UnMap();
    size_t SetData(uint8_t* data, size_t dataLen, size_t offset);
    size_t GetData(uint8_t* buffer, size_t bufLen, size_t offset);

};


