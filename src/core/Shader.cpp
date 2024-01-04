#include"core\Shader.h"
#include<iostream>
#include<fstream>
#include<vector>

Shader::Shader()
: m_ShaderType((VkShaderStageFlagBits)0)
, m_ShaderModule(VK_NULL_HANDLE)
, m_FilePath(nullptr)
{
}

Shader::Shader(VkShaderStageFlagBits shaderType, const char* shaderFilePath)
: Shader()
{
    m_ShaderType = shaderType;
    m_FilePath.assign(shaderFilePath);
}


bool Shader::Create(VkDevice device, bool forceCreate)
{
    if (forceCreate)
        Release();
    
    if (IsCreate())
        return true;

    if (m_FilePath.empty())
    {
        std::cout << "--> Loading Shader Moudle Failed: Invalid File Path!" << std::endl;
        return false;
    }

    if (device == VK_NULL_HANDLE)
    {
        std::cout << "--> Loading Shader Module At Path(" << m_FilePath << ")" << "Failed: Null Device!" << std::endl;
        return false;
    }

    std::ifstream fs(m_FilePath.c_str(), std::ios_base::binary | std::ios_base::in | std::ios_base::ate);
    if (!fs.is_open())
    {
        std::cout << "--> Loading Shader Module At Path(" << m_FilePath << ")" << "Failed to Open!" << std::endl;
        return false;
    }

    int byteLen = fs.tellg();
    std::vector<char> shaderSrcBytes(byteLen, NULL);
    fs.seekg(std::ios_base::beg);
    fs.read(shaderSrcBytes.data(), byteLen);
    fs.close();

    VkShaderModuleCreateInfo smCreateInfo{};
    smCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    smCreateInfo.flags = 0;
    smCreateInfo.pNext = nullptr;
    smCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderSrcBytes.data()); // this is confused by vukan implementation
    smCreateInfo.codeSize = byteLen;

    VkShaderModule createdMoudle = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(device, &smCreateInfo, nullptr, &createdMoudle);
    if(result != VK_SUCCESS)
    {
        std::cout << "--> Loading Shader Module At Path(" << m_FilePath << ")" << "Failed: vulkan error(" << result << ")" << std::endl;
        return false;
    }

    m_ShaderModule = createdMoudle;
    m_Device = device;
    
    return true;
}


void Shader::Release()
{
    if (IsCreate())
    {
        vkDestroyShaderModule(m_Device, m_ShaderModule, nullptr);
        m_ShaderModule = VK_NULL_HANDLE;
        m_Device = VK_NULL_HANDLE;
        m_ShaderType = (VkShaderStageFlagBits)0;
        m_FilePath.clear();
    }
}

Shader::~Shader()
{
    if (IsCreate())
        Release();
}
