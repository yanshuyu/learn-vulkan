#include"Texture2D.h"
#include<core\Device.h>
#include<stb\stb_image.h>


Texture2D::Texture2D(Device* pdevice)
: Image(pdevice)
{
}


bool Texture2D::LoadFromFile(const char* srcFile, bool forceLoad)
{
    if (IsValid() && !forceLoad)
        return false;

    TextureImportSetting importSetting{};
    importSetting.filterMode = VK_FILTER_LINEAR;
    importSetting.genMipMap = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    importSetting.uAddressMode = importSetting.vAddressMode = importSetting.wAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    importSetting.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    importSetting.readWriteEnable = false;
    importSetting.genMipMap = true;
    importSetting.mipLevelFilterMode = VK_FILTER_LINEAR;
    importSetting.srgbImage = true;
    return LoadFromFile(srcFile, importSetting, forceLoad);
}

bool Texture2D::LoadFromFile(const char* srcFile, const TextureImportSetting& importSetting, bool forceLoad)
{
    if (IsValid() && !forceLoad)
        return false;
    
    if (IsValid() && forceLoad)
        Release();

    int w, h, sz;
    VkFormat fmt;
    uint8_t* pdata = vkutils_stb_load_texture(_pDevice, srcFile, importSetting.srgbImage, importSetting.readWriteEnable, &w, &h, &fmt, &sz);
    ImageDesc imgDesc;
    imgDesc.format = fmt;
    imgDesc.extents.width = w;
    imgDesc.extents.height = h;
    imgDesc.extents.depth = 1;
    imgDesc.layers = 1;
    imgDesc.mipLeves = importSetting.genMipMap ? vkutils_get_mip_level_count_from_extents(imgDesc.extents): 1;
    imgDesc.sampleCount = 1;
    imgDesc.mipLevelFilter = importSetting.mipLevelFilterMode;
    imgDesc.linearTiling = importSetting.readWriteEnable;
    imgDesc.usageFlags = importSetting.readWriteEnable ? 
                            VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;    
    bool success = LoadFromtData(pdata, sz, imgDesc, importSetting);
  
    if (success)
        _srcFile = srcFile;
    
    stbi_image_free(pdata);

    return success;
}

bool Texture2D::LoadFromtData(const uint8_t* data, size_t dataSz, const ImageDesc& desc, const TextureImportSetting& importSetting, bool forceLoad)
{
    if (IsValid() && !forceLoad)
        return false;

    if (IsValid() && forceLoad)
        Release();
    
    // create vulkan image & view
    if(!Create(desc))
    {
        LOGE("Failed to create vkimage for texture data: {}", (void*)data);
        return false;
    }

    SetPixels(data, dataSz); 

    // create sampler
    VkSamplerCreateInfo samplerInfo{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
    samplerInfo.addressModeU = importSetting.uAddressMode;
    samplerInfo.addressModeV = importSetting.vAddressMode;
    samplerInfo.addressModeW = importSetting.wAddressMode;
    samplerInfo.borderColor = importSetting.borderColor;
    samplerInfo.magFilter = importSetting.filterMode;
    samplerInfo.minFilter = importSetting.filterMode;
    samplerInfo.mipmapMode = importSetting.mipMapMode;
    samplerInfo.anisotropyEnable = importSetting.anisotropyFactor > 0;
    samplerInfo.maxAnisotropy =  vkutils_remap_anisotropy_level(importSetting.anisotropyFactor, _pDevice);
    samplerInfo.compareEnable = false;
    samplerInfo.minLod = 0;
    samplerInfo.maxLod = m_Desc.mipLeves - 1;
    samplerInfo.mipLodBias = std::min(importSetting.mipLodBias, (float)_pDevice->GetDeviceLimit(DeviceLimits::maxMipLodBias));
    if (VKCALL_FAILED(vkCreateSampler(_pDevice->GetHandle(), &samplerInfo, nullptr, &_sampler)))
    {
        LOGE("Failed to create sampler when loading texture from data: {}", (void*)data);
        Release();
        return false;
    }
    _importSetting = importSetting;
    _srcFile.clear();

    return true;
}


void Texture2D::Release()
{
    if (VKHANDLE_IS_NOT_NULL(_sampler))
    {
        vkDestroySampler(_pDevice->GetHandle(), _sampler, nullptr);
        VKHANDLE_SET_NULL(_sampler);
    }
    Image::Release();
}
