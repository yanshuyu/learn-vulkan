#pragma once
#include<core\Image.h>
#include<string>


class Texture2D : public Image
{
private:
    std::string _srcFile{};
    VkSampler _sampler{VK_NULL_HANDLE};
    TextureImportSetting _importSetting{};
public:
    Texture2D(Device* pdevice);
    ~Texture2D() = default;

    NONE_COPYABLE_NONE_MOVEABLE(Texture2D)

    bool LoadFromFile(const char* srcFile, bool forceLoad = false);
    bool LoadFromFile(const char* srcFile, const TextureImportSetting& importSetting, bool forceLoad = false);
    bool LoadFromtData(const uint8_t* data, size_t dataSz, const ImageDesc& desc, const TextureImportSetting& importSetting, bool forceLoad = false);
    void Release() override;
    bool IsValid() const override { return Image::IsValid() && VKHANDLE_IS_NOT_NULL(_sampler); }

    const TextureImportSetting& GetImportSetting() const { return _importSetting; }
    VkFormat GetFormat() const { return m_Desc.format; }
    size_t GetMipMapLevelCount() const { return m_Desc.mipLeves; }
    bool IsReadWriteEnable() const { return m_Desc.linearTiling; }
    const char* GetFilePath() const { return _srcFile.c_str(); } 
};

