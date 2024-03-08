#pragma once
#include"core\CoreUtils.h"
#include"core\IMapAccessMemory.h"

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


class Image : public IMapAccessMemory
{
    friend class Device;

private:
    Device* m_pDevice{nullptr};
    VkImage m_vkImage{VK_NULL_HANDLE};
    VkDeviceMemory m_ImageMem{VK_NULL_HANDLE};
    ImageDesc m_Desc{};
    uint32_t m_MemSz{0};
    uint32_t m_MipLevelCount{1};
    std::vector<VkImageView> m_views{};

protected:
    void Reset()
    {
        m_pDevice = nullptr;
        VKHANDLE_SET_NULL(m_vkImage);
        VKHANDLE_SET_NULL(m_ImageMem);
        m_MemSz = 0;
        m_Desc = {};
        m_views.clear();
    }

public:
    Image(): IMapAccessMemory() {}
    ~Image() { assert(!IsValid()); }

    NONE_COPYABLE_NONE_MOVEABLE(Image)

    bool Create(Device *pDevice, const ImageDesc &desc);
    void Release();
    VkImageView CreateView(VkImageViewType viewType, VkImageAspectFlags viewAspect, uint32_t baseArrayLayer, uint32_t layerCnt, uint32_t baseMipLevel, uint32_t levelCnt );
    bool DestroyView(VkImageView view);
    bool IsValid() const { return m_pDevice != nullptr && VKHANDLE_IS_NOT_NULL(m_vkImage) && VKHANDLE_IS_NOT_NULL(m_ImageMem); }

    bool CanMap() const override { return IsValid() && IMapAccessMemory::CanMap() && m_Desc.linearTiling; }
    Device* GetDevice() const { return m_pDevice; }
    VkDevice GetDeviceHandle() const override;
    VkImage GetHandle() const { return m_vkImage; }
    VkDeviceMemory GetMemory() const override { return m_ImageMem; }
    VkMemoryPropertyFlags GetMemoryProperties() const override { return m_Desc.memFlags; }
    uint32_t GetMemorySize() const override { return m_MemSz; }
    uint32_t GetMipMapCount() const { return m_MipLevelCount; }
    ImageDesc GetDesc() const { return m_Desc; }
};


