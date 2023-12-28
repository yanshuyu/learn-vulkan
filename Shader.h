#pragma once
#include<vulkan\vulkan.h>
#include<string>

class Shader
{
private:
    VkShaderModule m_ShaderModule;
    VkShaderStageFlagBits m_ShaderType;
    VkDevice m_Device;
    std::string m_FilePath;

public:
    Shader();
    Shader(VkShaderStageFlagBits shaderType, const char* shaderFilePath);
    ~Shader();

    void SetFilePathHint(const char* filePath) { m_FilePath.assign(filePath); }
    const char* GetFilePath() const { return m_FilePath.c_str(); }
    
    void SetShaderStageHint(VkShaderStageFlagBits shaderType) { m_ShaderType = shaderType; }
    VkShaderStageFlagBits GetShaderStage() const { return m_ShaderType; }

    VkDevice GetOwnerDevice() const { return m_Device; }
    VkShaderModule GetRawMoudle() const { return m_ShaderModule; }

    bool IsCreate() const { return m_ShaderModule != VK_NULL_HANDLE; }
    bool Create(VkDevice device, bool forceCreate = false);
    void Release();
};

