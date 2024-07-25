#pragma once
#include<core\Image.h>
#include"TextureCube.h"

class Device;

struct RenderTextureDesc
{
    VkFormat format{VK_FORMAT_MAX_ENUM};
    VkFilter filter{VK_FILTER_LINEAR};
    VkSamplerAddressMode uvMode{VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE}; 
    uint32_t width{0};
    uint32_t height{0};
    uint32_t sampleCnt{1};
    VkCompareOp cmpOp{VK_COMPARE_OP_ALWAYS};
    bool cmpEnable{false}; 
    bool genMipMaps{true};
    bool sampled{true};
    bool persistable{false};

    RenderTextureDesc() = default;
    RenderTextureDesc(VkFormat fmt, uint32_t w, uint32_t h, bool genMipmaps = true);
    RenderTextureDesc(VkFormat fmt, uint32_t w, uint32_t h, bool genMipmaps, VkFilter filterMode, VkCompareOp cmp);
};



class RenderTexture : public Image
{
private:
    bool _create(const RenderTextureDesc& desc);
    RenderTextureDesc _rtDesc;
    VkSampler _sampler{VK_NULL_HANDLE};

public:
    RenderTexture(Device* pdevice, const RenderTextureDesc& rtDesc);
    ~RenderTexture();

    NONE_COPYABLE_NONE_MOVEABLE(RenderTexture)

    void Release() override;
    VkSampler GetSampler() const { return _sampler; }
    uint32_t GetWidth() const { return _rtDesc.width; }
    uint32_t GetHeight() const { return _rtDesc.height; }
    const RenderTextureDesc& GetRenderTextrueDesc() const { return _rtDesc; }
};

