#pragma once
#include"core\CoreUtils.h"
#include"core\IMapAccessMemory.h"
#include"core\VKDeviceResource.h"

class Device;

struct ImageDesc
{
    VkFormat format;
    VkExtent3D extents;
    uint32_t layers;
    uint32_t sampleCount;
    VkImageUsageFlags usageFlags;
    VkMemoryPropertyFlags memFlags;
    bool generalMipMaps;
    bool linearTiling;
};


class Image : public IMapAccessMemory, VKDeviceResource
{
    friend class Device;

private:
    VkImage m_vkImage{VK_NULL_HANDLE};
    VkDeviceMemory m_ImageMem{VK_NULL_HANDLE};
    ImageDesc m_Desc{};
    uint32_t m_MemSz{0};
    uint32_t m_MipLevelCount{1};
    std::vector<VkImageView> m_views{};

protected:
    bool _create(const ImageDesc &desc);
    void Release() override;

public:
    Image(Device* pDevice = nullptr);
    ~Image() { assert(!IsValid()); }

    NONE_COPYABLE_NONE_MOVEABLE(Image)

    VkImageView CreateView(VkImageViewType viewType, VkImageAspectFlags viewAspect, uint32_t baseArrayLayer, uint32_t layerCnt, uint32_t baseMipLevel, uint32_t levelCnt );
    bool DestroyView(VkImageView view);
    bool IsValid() const override { return VKHANDLE_IS_NOT_NULL(m_vkImage) && VKHANDLE_IS_NOT_NULL(m_ImageMem); }
    bool CanMap() const override { return IsValid() && IMapAccessMemory::CanMap() && m_Desc.linearTiling; }
    VkImage GetHandle() const { return m_vkImage; }
    VkDevice GetDeviceHandle() const override;
    VkDeviceMemory GetMemory() const override { return m_ImageMem; }
    VkMemoryPropertyFlags GetMemoryProperties() const override { return m_Desc.memFlags; }
    uint32_t GetMemorySize() const override { return m_MemSz; }
    uint32_t GetMipMapCount() const { return m_MipLevelCount; }
    ImageDesc GetDesc() const { return m_Desc; }
};


