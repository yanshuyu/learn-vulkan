#include"AssetsManager.h"
#include"core\Device.h"
#include"core\ShaderProgram.h"
#include"rendering\Texture2D.h"
#include"rendering\DescriptorSetManager.h"
#include<stb\stb_image.h>
#include<spirv-reflect/spirv_reflect.h>
#include<fstream>
#include<regex>

#define PRPGRAM_KEY(vs, fs) std::string

std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> AssetsManager::s_programs{};
std::unordered_map<std::string, ShaderStageInfo> AssetsManager::s_shaderModules{};
Device* AssetsManager::s_pDevice{nullptr};


void AssetsManager::Initailize(Device* pDevice)
{
    assert(pDevice && pDevice->IsValid());
    s_pDevice = pDevice;
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
        vkDestroyShaderModule(s_pDevice->GetHandle(), itr.second.shaderMoudle, nullptr);
    }
    s_shaderModules.clear();
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
    auto vsShader = _load_shader_moudle(vs, vsName, VK_SHADER_STAGE_VERTEX_BIT);
    auto fsShader = _load_shader_moudle(fs, fsName, VK_SHADER_STAGE_FRAGMENT_BIT);
    assert(vsShader);
    assert(fsShader);
    program->AddShader(vsShader);
    program->AddShader(fsShader);
    assert(_parse_shader_reflection(program));
    assert(program->Apply());

    auto materialSetBindings = program->GetDescriptorSetBindings(PerMaterial);
    if (materialSetBindings)
        DescriptorSetManager::RegisterSetLayout(PerMaterial, DescriptorSetManager::DefaultProgramSetHash(program), *materialSetBindings, true, 2);

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


 const ShaderStageInfo* AssetsManager::_load_shader_moudle(const char * srcFile, const char* entryName, VkShaderStageFlagBits stage)
 {
    std::string fullPath(ASSETS_DIR);
    fullPath += srcFile;
     auto itr = s_shaderModules.find(fullPath);
     if (itr == s_shaderModules.end())
     {
         std::ifstream fs(fullPath.c_str(), std::ios_base::ate | std::ios_base::binary);
         if (!fs.is_open())
             return nullptr;

         size_t srcSz = fs.tellg();
         fs.seekg(std::ios_base::beg);
         std::vector<char> spvSrc(srcSz, NULL);
         fs.read(spvSrc.data(), srcSz);
         fs.close();

         VkShaderModule shaderMou{VK_NULL_HANDLE};
         VkShaderModuleCreateInfo shaderMouCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
         shaderMouCreateInfo.pCode = (uint32_t *)spvSrc.data();
         shaderMouCreateInfo.codeSize = srcSz;
         if(VKCALL_FAILED(vkCreateShaderModule(s_pDevice->GetHandle(), &shaderMouCreateInfo, nullptr, &shaderMou)))
            return nullptr;
         
         itr = s_shaderModules.insert(std::make_pair(fullPath, ShaderStageInfo())).first;
         itr->second.srcPath = fullPath;
         itr->second.pEntryName = entryName;
         itr->second.spvCodes = std::move(spvSrc);
         itr->second.stage = stage;
         itr->second.shaderMoudle = shaderMou;
     }

     return &itr->second;
 }

 VertexAttribute AssetsManager::_attr_name_to_type(const char *name)
 {
    if (std::regex_search(name, std::regex(".*pos.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*vertex.*", std::regex::icase)))
        return VertexAttribute::Position;
    
    if (std::regex_search(name, std::regex(".*nor.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*normal.*", std::regex::icase)))
        return VertexAttribute::Normal;

    if (std::regex_search(name, std::regex(".*tan.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*tangent.*", std::regex::icase)))
        return VertexAttribute::Tangent;

    if (std::regex_search(name, std::regex(".*col.*", std::regex::icase)) 
        || std::regex_search(name, std::regex(".*color.*", std::regex::icase)))
        return VertexAttribute::Color;

    if (std::regex_search(name, std::regex(".*uv.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*uv0.*", std::regex::icase)))
        return VertexAttribute::UV0;

    if (std::regex_search(name, std::regex(".*uv1.*", std::regex::icase)))
        return VertexAttribute::UV1;
    
    return VertexAttribute::MaxAttribute;
 }

 bool AssetsManager::_parse_shader_reflection(ShaderProgram *shader)
 {
    for (auto &&shaderStageInfo : shader->GetShaderStageInfos())
    {
        SpvReflectShaderModule refltShaderModule{};
        SpvReflectResult result = spvReflectCreateShaderModule(shaderStageInfo->spvCodes.size(), shaderStageInfo->spvCodes.data(), &refltShaderModule);
        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {   
            LOGE("Failed to create reflect shader module: {}", shaderStageInfo->srcPath.c_str()); 
            return false;
        }
        // parse vertex shader input attributes if current shader module stage is vertex
        if (refltShaderModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
        {
            uint32_t inputVarCnt = 0;
            spvReflectEnumerateInputVariables(&refltShaderModule, &inputVarCnt, nullptr);
            std::vector<SpvReflectInterfaceVariable*> inputVarInfos(inputVarCnt);
            spvReflectEnumerateInputVariables(&refltShaderModule, &inputVarCnt, inputVarInfos.data());
            for (size_t i = 0; i < inputVarCnt; i++)
            {
                const SpvReflectInterfaceVariable& inputVar = *inputVarInfos[i];
                // ignore built in variables
                if (inputVar.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
                    continue;

                shader->AddAttribute(_attr_name_to_type(inputVar.name), inputVar.location, static_cast<VkFormat>(inputVar.format), inputVar.name);
            } 
        }

        // parse descriptor set & bindings
        uint32_t setCnt = 0;
        spvReflectEnumerateDescriptorSets(&refltShaderModule, &setCnt, nullptr);
        std::vector<SpvReflectDescriptorSet*> setInfos(setCnt);
        spvReflectEnumerateDescriptorSets(&refltShaderModule, &setCnt, setInfos.data());
        for (size_t i = 0; i < setCnt; i++)
        {
            for (size_t j = 0; j < setInfos[i]->binding_count; j++)
            {
                const SpvReflectDescriptorBinding& bindingInfo = *setInfos[i]->bindings[j];
                if (bindingInfo.descriptor_type == SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER)
                {
                    int memCnt = bindingInfo.type_description->member_count;
                    for (size_t k = 0; k < bindingInfo.type_description->member_count; k++)
                    {
                        auto& p = bindingInfo.type_description->members[k];
                    }
                    
                }

            }
            
        }
        

        spvReflectDestroyShaderModule(&refltShaderModule);
    }
    
 }
