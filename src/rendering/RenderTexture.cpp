#include"RenderTexture.h"	
#include<core\Device.h>


RenderTextureDesc::RenderTextureDesc(VkFormat fmt, uint32_t w, uint32_t h, bool genMipmaps)
:RenderTextureDesc()
{
    format = fmt;
    width = w;
    height = h;
    genMipMaps = genMipmaps;
}


RenderTextureDesc::RenderTextureDesc(VkFormat fmt, uint32_t w, uint32_t h, bool genMipmaps, VkFilter filterMode, VkCompareOp cmp)
:RenderTextureDesc(fmt, w, h, genMipmaps)
{
    filter = filterMode;
    cmpEnable = true;
    cmpOp = cmp;
}


RenderTexture::RenderTexture(Device* pdevice, const RenderTextureDesc& desc)
: Image(pdevice)
{
    if (!_create(desc))
        throw std::runtime_error("create render texture error");
    
    _rtDesc = desc;
}


RenderTexture::~RenderTexture()
{
    Release();
}


bool RenderTexture::_create(const RenderTextureDesc& desc)
{
    ImageDesc imgDesc{};
    imgDesc.format = desc.format;
    imgDesc.extents.width = desc.width;
    imgDesc.extents.height = desc.height;
    imgDesc.extents.depth = 1;
    imgDesc.layers = 1;
    imgDesc.sampleCount = desc.sampleCnt;
    imgDesc.mipLeves = desc.genMipMaps ? vkutils_get_mip_level_count_from_extents(imgDesc.extents) : 1;
    imgDesc.linearTiling = false;
    imgDesc.mipLevelFilter = desc.filter;
    imgDesc.usageFlags = vkutils_get_render_texture_usage_flags(desc.format, desc.sampled, desc.persistable);
    if (!Create(imgDesc))
        return false;

    // create sampler if need
    if (desc.sampled)
    {
        VkSamplerCreateInfo samplerCreateInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
        samplerCreateInfo.addressModeU = samplerCreateInfo.addressModeV = samplerCreateInfo.addressModeW = desc.uvMode;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        samplerCreateInfo.anisotropyEnable = false;
        samplerCreateInfo.maxAnisotropy = 0;
        samplerCreateInfo.minFilter = samplerCreateInfo.magFilter = desc.filter;
        samplerCreateInfo.maxLod = desc.genMipMaps ? imgDesc.mipLeves : 0;
        samplerCreateInfo.compareEnable = desc.cmpEnable;
        samplerCreateInfo.compareOp = desc.cmpOp;
        return  vkCreateSampler(_pDevice->GetHandle(), &samplerCreateInfo, nullptr, &_sampler) == VK_SUCCESS;
    }

    return true;
}


void RenderTexture::Release()
{
    if (VKHANDLE_IS_NOT_NULL(_sampler))
    {
        vkDestroySampler(_pDevice->GetHandle(), _sampler, nullptr);
        VKHANDLE_SET_NULL(_sampler);
    }
    Image::Release();
}

