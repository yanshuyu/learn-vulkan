#pragma once
#include"core\CoreUtils.h"
#include"core\IMapAccessMemory.h"
#include"core\VKDeviceResource.h"

class Device;

struct ImageDesc
{
    VkFormat format{VK_FORMAT_MAX_ENUM};
    VkExtent3D extents{0,0,0};
    uint32_t layers{1};
    uint32_t mipLeves{1};
    uint32_t sampleCount{1};
    VkFilter mipLevelFilter{VK_FILTER_LINEAR};
    VkImageUsageFlags usageFlags{0};
    VkImageCreateFlags flags{0};
    bool linearTiling{false};
};


class Image : public IMapAccessMemory, public VKDeviceResource
{
    friend class Device;

protected:
    VkImage m_vkImage{VK_NULL_HANDLE};
    VkDeviceMemory m_ImageMem{VK_NULL_HANDLE};
    VkMemoryPropertyFlags m_MemProps{0};
    ImageDesc m_Desc{};
    uint32_t m_MemSz{0};
    VkImageView m_View{VK_NULL_HANDLE};
    VkImageLayout m_Layout{VK_IMAGE_LAYOUT_UNDEFINED};

protected:
    virtual VkImageView CreateView(const ImageDesc& desc);

public:
    Image(Device* pDevice = nullptr);
    virtual ~Image() { Release(); }

    NONE_COPYABLE_NONE_MOVEABLE(Image)

    bool Create(const ImageDesc &desc);
    virtual void SetPixels(const uint8_t* rawData, size_t dataSz, size_t layer = 0);
    virtual void Release() override;
    
    bool IsCreate() const { return VKHANDLE_IS_NOT_NULL(m_vkImage) && VKHANDLE_IS_NOT_NULL(m_ImageMem); }
    virtual bool IsValid() const override { return VKHANDLE_IS_NOT_NULL(m_vkImage) && VKHANDLE_IS_NOT_NULL(m_ImageMem) && VKHANDLE_IS_NOT_NULL(m_View); }
    bool CanMap() const override { return IsValid() && IMapAccessMemory::CanMap() && m_Desc.linearTiling; }
    VkImage GetHandle() const { return m_vkImage; }
    VkDevice GetDeviceHandle() const override;
    VkDeviceMemory GetMemory() const override { return m_ImageMem; }
    VkMemoryPropertyFlags GetMemoryProperties() const override { return m_MemProps; }
    uint32_t GetMemorySize() const override { return m_MemSz; }
    const ImageDesc& GetDesc() const { return m_Desc; }
    VkImageView GetView() const { return m_View; }
    VkImageLayout GetLayout() const {return m_Layout; }
    VkExtent3D GetExtent() const { return m_Desc.extents; }
    VkFormat GetFormat() const { return m_Desc.format; }
    VkImageType GetType() const { return vkutils_get_image_type_form_extents(m_Desc.extents); }
    size_t GetMipLevelsCount() const { return m_Desc.mipLeves; }
};


