#include"TextureCube.h"
#include<core\Device.h>
#include<stb\stb_image.h>

TextureCube::TextureCube(Device* pdevic)
: Image(pdevic)
{
}


bool TextureCube::LoadFromFiles(const char *negX,
                    const char *posX,
                    const char *negY,
                    const char *posY,
                    const char *negZ,
                    const char *posZ,
                    const TextureImportSetting &importSetting,
                    bool forceLoad)
{
    if (IsValid() && forceLoad)
        Release();
    
    if (IsValid() && !forceLoad)
        return false;

    uint8_t* facePixels[6]{nullptr};
    const char* srcFiles[6]{negX, posX, negY, posY, negZ, posZ};

    int w, h, sz;
    VkFormat fmt;
    for (size_t i = 0; i < 6; i++)
    {
        int _w, _h, _sz;
        VkFormat _fmt; 
        facePixels[i] = vkutils_stb_load_texture(_pDevice, srcFiles[i], importSetting.srgbImage, importSetting.readWriteEnable, &_w, &_h, &_fmt, &_sz);
        if ( i > 0)
        {
            if (_w != w || _h != h || _fmt != fmt || _sz != sz)
            {
                for (size_t j = i; j >= 0; j--)
                    if (facePixels[j] != nullptr)
                        stbi_image_free(facePixels[j]);
                
                LOGE("Loading TextureCube in different fmt/sz!");
                return false;
            }
        }
        else
        {
            w = _w;
            h = _h;
            sz = _sz;
            fmt = _fmt;
        }

    }

    // create vulkan image & views
    ImageDesc imgDesc{};
    imgDesc.format = fmt;
    imgDesc.extents.width = w;
    imgDesc.extents.height = h;
    imgDesc.extents.depth = 1;
    imgDesc.layers = 6;
    imgDesc.mipLeves = importSetting.genMipMap ? vkutils_get_mip_level_count_from_extents(imgDesc.extents) : 1;
    imgDesc.sampleCount = 1;
    imgDesc.mipLevelFilter = importSetting.mipLevelFilterMode;
    imgDesc.linearTiling = importSetting.readWriteEnable;
    imgDesc.usageFlags = importSetting.readWriteEnable ? 
                            VK_IMAGE_USAGE_SAMPLED_BIT : VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imgDesc.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    bool success = LoadFromData(facePixels[0], facePixels[1], facePixels[2], facePixels[3], facePixels[4], facePixels[5], sz, imgDesc, importSetting, forceLoad);
    
    for (size_t i = 0; i < 6; i++)
        if (facePixels[i] != nullptr)
            stbi_image_free(facePixels[i]);

    _faceFiles[Face::negX] = negX;
    _faceFiles[Face::posX] = posX;
    _faceFiles[Face::negY] = negY;
    _faceFiles[Face::posY] = posY;
    _faceFiles[Face::negZ] = negZ;
    _faceFiles[Face::posZ] = posZ;

    return success;
}

bool TextureCube::LoadFromFiles(const char *negX,
                    const char *posX,
                    const char *negY,
                    const char *posY,
                    const char *negZ,
                    const char *posZ,
                    bool forceLoad)
{
    TextureImportSetting importSetting{};
    importSetting.filterMode = VK_FILTER_LINEAR;
    importSetting.mipMapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    importSetting.genMipMap = true;
    importSetting.mipLevelFilterMode = VK_FILTER_LINEAR;
    importSetting.mipLodBias = 0;
    importSetting.uAddressMode = importSetting.vAddressMode = importSetting.wAddressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    importSetting.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    importSetting.anisotropyFactor = 0;
    importSetting.srgbImage = true;
    importSetting.readWriteEnable = false;
    return LoadFromFiles(negX, posX, negY, posY, negZ, posZ, importSetting, forceLoad);    
}

bool TextureCube::LoadFromData(const uint8_t *negX,
                    const uint8_t *posX,
                    const uint8_t *negY,
                    const uint8_t *posY,
                    const uint8_t *negZ,
                    const uint8_t *posZ,
                    size_t dataSz,
                    const ImageDesc &desc,
                    const TextureImportSetting &importSetting,
                    bool forceLoad)
{
    if (IsValid() && forceLoad)
        Release();

    if (IsValid() && !forceLoad)
        return false;   
    
    // create vulkan image & view
    if (!Create(desc))
        return false;
    
    SetPixels(negX, dataSz, Face::negX);
    SetPixels(posX, dataSz, Face::posX);
    SetPixels(negY, dataSz, Face::negY);
    SetPixels(posY, dataSz, Face::posY);
    SetPixels(negZ, dataSz, Face::negZ);
    SetPixels(posZ, dataSz, Face::posZ);

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
        LOGE("Failed to create sampler when loading texture cube from data: {}");
        Release();
        return false;
    }

    _importSetting = importSetting;

    return true;
}


void TextureCube::Release()
{
    if (VKHANDLE_IS_NOT_NULL(_sampler))
    {
        vkDestroySampler(_pDevice->GetHandle(), _sampler, nullptr);
        VKHANDLE_SET_NULL(_sampler);
    }

    Image::Release();
}