#include"core\Image.h"
#include"core\Device.h"

 Image::Image(Device* pDevice): VKDeviceResource(pDevice), IMapAccessMemory()
{

}

bool Image::_create(const ImageDesc &desc)
{
    bool canMap = desc.linearTiling && (desc.memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) > 0; // only linear tiling image can access by host directly
    VkImageCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    createInfo.format = desc.format;
    createInfo.imageType = vkutils_get_image_type_form_extents(desc.extents);
    createInfo.extent = desc.extents;
    createInfo.arrayLayers = desc.layers;
    createInfo.samples = vkutils_get_sample_count_flag_bit(desc.sampleCount);
    createInfo.mipLevels = desc.generalMipMaps ? vkutils_get_mip_level_count_from_extents(desc.extents) : 1;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.initialLayout = canMap ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.tiling = desc.linearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    createInfo.usage = desc.usageFlags;
    VkImage createdImage{VK_NULL_HANDLE};
    VkResult result = vkCreateImage(_pDevice->GetHandle(), &createInfo, nullptr, &createdImage);
    if (VKCALL_FAILED(result))
    {
        LOGE("Device({}) create image error({})!", (void *)_pDevice, result);
        return false;
    }

    VkMemoryRequirements memReq{};
    vkGetImageMemoryRequirements(_pDevice->GetHandle(), createdImage, &memReq);
    VkDeviceMemory allocedMem{VK_NULL_HANDLE};
    if (!_pDevice->AllocMemory(memReq, desc.memFlags, &allocedMem))
    {
        vkDestroyImage(_pDevice->GetHandle(), createdImage, nullptr);
        return false;
    }

    result = vkBindImageMemory(_pDevice->GetHandle(), createdImage, allocedMem, 0);
    if (VKCALL_FAILED(result))
    {
        vkDestroyImage(_pDevice->GetHandle(), createdImage, nullptr);
        _pDevice->FreeMemory(allocedMem);
        LOGE("Image({}) memory({}) bind error({})!", (void *)createdImage, (void *)allocedMem, result);
        return false;
    }

    m_vkImage = createdImage;
    m_ImageMem = allocedMem;
    m_Desc = desc;
    m_MemSz = memReq.size;
    m_MipLevelCount = createInfo.mipLevels;

    return true;
}


void Image::Release()
{
    if (!IsValid())
        return;

    // release views
    if (m_views.size() > 0)
    {
        LOGW("Release Image({}) with unrelease views!", (void*)this);
        for (auto &&view : m_views)
        {
            DestroyView(view);
        }
    }

    vkDestroyImage(_pDevice->GetHandle(), m_vkImage, nullptr);
    _pDevice->FreeMemory(m_ImageMem);
    VKHANDLE_SET_NULL(m_vkImage);
    VKHANDLE_SET_NULL(m_ImageMem);
    m_MemSz = 0;
    m_Desc = {};
    m_views.clear();
}

VkImageView Image::CreateView(VkImageViewType viewType, VkImageAspectFlags viewAspect, uint32_t baseArrayLayer, uint32_t layerCnt, uint32_t baseMipLevel, uint32_t levelCnt )
{
    if (!IsValid())
        return VK_NULL_HANDLE;

    VkImageViewCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    createInfo.format = m_Desc.format;
    createInfo.image = m_vkImage;
    createInfo.viewType = viewType;
    createInfo.subresourceRange.aspectMask = viewAspect;
    createInfo.subresourceRange.baseArrayLayer = baseArrayLayer;
    createInfo.subresourceRange.layerCount = layerCnt;
    createInfo.subresourceRange.baseMipLevel = baseMipLevel;
    createInfo.subresourceRange.levelCount = levelCnt;
    
    VkImageView createView{VK_NULL_HANDLE};
    if (VKCALL_FAILED(vkCreateImageView(_pDevice->GetHandle(), &createInfo, nullptr, &createView)))
    {
        LOGE("Image({}) create view error!", (void*)this);
        return VK_NULL_HANDLE;
    }

    m_views.push_back(createView);

    return createView;
}

bool Image::DestroyView(VkImageView view)
{
    if (!IsValid())
        return false;

    auto pos = std::find(m_views.begin(), m_views.end(), view);
    if (pos == m_views.end())
    {
        LOGW("Try to relase a view not create by image({})!", (void*)this);
        return false;
    }

    vkDestroyImageView(_pDevice->GetHandle(), view, nullptr);
    m_views.erase(pos);
    return true;
}

VkDevice Image::GetDeviceHandle() const
{
    return _pDevice->GetHandle();
}