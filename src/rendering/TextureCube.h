#pragma once
#include<core\Image.h>
#include<string>

class Device;

class TextureCube : public Image
{
public:
    enum Face 
    {
        negZ,
        posZ,
        posY,
        negY,
        posX,
        negX,
    };

private:
    VkSampler _sampler{VK_NULL_HANDLE};
    TextureImportSetting _importSetting{};
    std::string _faceFiles[6] {};
    
public:
    TextureCube(Device* pdevice);
    ~TextureCube() = default;

    NONE_COPYABLE_NONE_MOVEABLE(TextureCube)

    bool LoadFromFiles(const char *negX,
                       const char *posX,
                       const char *negY,
                       const char *posY,
                       const char *negZ,
                       const char *posZ,
                       const TextureImportSetting &importSetting,
                       bool forceLoad = false);

    bool LoadFromFiles(const char *negX,
                       const char *posX,
                       const char *negY,
                       const char *posY,
                       const char *negZ,
                       const char *posZ,
                       bool forceLoad = false);

    bool LoadFromData(const uint8_t *negX,
                      const uint8_t *posX,
                      const uint8_t *negY,
                      const uint8_t *posY,
                      const uint8_t *negZ,
                      const uint8_t *posZ,
                      size_t dataSz,
                      const ImageDesc &desc,
                      const TextureImportSetting &importSetting,
                      bool forceLoad = false);
    
    void SetPixels(const uint8_t* pdata, size_t dataSz, Face face)
    {
        Image::SetPixels(pdata, dataSz, (size_t)face);
    }

    bool IsValid() const override { return Image::IsValid() && VKHANDLE_IS_NOT_NULL(_sampler); }
    void Release() override;

    const char* GetFailePath(Face face) const { return _faceFiles[face].c_str(); }
    const TextureImportSetting& GetImportSetting() const { return _importSetting; }
    VkSampler GetSampler() const { return _sampler; }
    VkFormat GetFormat() const { return m_Desc.format; }
    size_t GetMipLevelCount() const { return m_Desc.mipLeves; }    

};
