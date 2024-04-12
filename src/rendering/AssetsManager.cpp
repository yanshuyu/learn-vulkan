#include"AssetsManager.h"
#include"core\Device.h"
#include"core\ShaderProgram.h"
#include"rendering\ShaderReflection.h"
#include"rendering\Texture2D.h"
#include"rendering\DescriptorSetManager.h"
#include<stb\stb_image.h>
#include<fstream>

#define PRPGRAM_KEY(vs, fs) std::string

std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> AssetsManager::s_programs{};
std::unordered_map<std::string, VkShaderModule> AssetsManager::s_shaderModules{};
const std::string AssetsManager::s_shaderDir(std::string(ASSETS_DIR) + "/shaders/");
Device* AssetsManager::s_pDevice{nullptr};


void AssetsManager::Initailize(Device* pDevice)
{
    assert(pDevice && pDevice->IsValid());
    s_pDevice = pDevice;
    ShaderReflection::Initailze();
}



void AssetsManager::DeInitailize()
 {
    for (auto &&itr : s_programs)
    {
        itr.second->Release();
    }
    s_programs.clear();

    for (auto &&itr : s_shaderModules)
    {
        vkDestroyShaderModule(s_pDevice->GetHandle(), itr.second, nullptr);
    }
    s_shaderModules.clear();

    ShaderReflection::DeInitailize();
 }




ShaderProgram* AssetsManager::LoadProgram(const char* vs, const char* vsName, const char* fs, const char* fsName)
{
    std::string k_program(vs);
    k_program += ":";
    k_program += fs;
    auto pos = s_programs.find(k_program);
    if (pos != s_programs.end())
        return pos->second.get();

    auto result = s_programs.emplace(std::make_pair(k_program, new ShaderProgram(s_pDevice)));
    ShaderProgram* program = result.first->second.get();
    program->SetName(k_program.c_str());
    ShaderStageInfo vsi{vsName, VK_NULL_HANDLE, VK_SHADER_STAGE_VERTEX_BIT};
    ShaderStageInfo fsi{fsName, VK_NULL_HANDLE, VK_SHADER_STAGE_FRAGMENT_BIT};
    vsi.shaderMoudle = _load_shader_moudle((s_shaderDir + vs).c_str());
    fsi.shaderMoudle = _load_shader_moudle((s_shaderDir + fs).c_str());
    program->AddShader(vsi);
    program->AddShader(fsi);
    ShaderReflection::Parse(program);
    assert(program->Apply());

    auto materialSetBindings = program->GetDescriptorSetBindings(PerMaterial);
    if (materialSetBindings)
        DescriptorSetManager::RegisterSetLayout(PerMaterial, 0, *materialSetBindings, true, 2);

    return program;
}


void AssetsManager::UnloadProgram(ShaderProgram* program)
{
    auto itr = s_programs.find(program->GetName());
    if (itr != s_programs.end())
    {
        itr->second->Release();
        s_programs.erase(itr);
    }
}


 VkShaderModule AssetsManager::_load_shader_moudle(const char *srcPath)
 {
     auto itr = s_shaderModules.find(srcPath);
     if (itr == s_shaderModules.end())
     {
         std::ifstream fs(srcPath, std::ios_base::ate | std::ios_base::binary);
         if (!fs.is_open())
             return VK_NULL_HANDLE;

         size_t srcSz = fs.tellg();
         fs.seekg(std::ios_base::beg);
         std::vector<char> spvSrc(srcSz, NULL);
         fs.read(spvSrc.data(), srcSz);
         fs.close();

         VkShaderModule shaderMou{VK_NULL_HANDLE};
         VkShaderModuleCreateInfo shaderMouCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
         shaderMouCreateInfo.pCode = (uint32_t *)spvSrc.data();
         shaderMouCreateInfo.codeSize = srcSz;
         assert(VKCALL_SUCCESS(vkCreateShaderModule(s_pDevice->GetHandle(), &shaderMouCreateInfo, nullptr, &shaderMou)));
         itr = s_shaderModules.insert(std::make_pair(srcPath, shaderMou)).first;
     }

     return itr->second;
 }
