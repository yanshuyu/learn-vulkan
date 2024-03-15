#include"AssetsManager.h"
#include"core\Device.h"
#include"core\ShaderProgram.h"
#include<fstream>

#define PRPGRAM_KEY(vs, fs) std::string

std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> AssetsManager::s_programs{};
std::unordered_map<std::string, std::function<void(ShaderProgram*)>> AssetsManager::s_programResourceInitailizer{};
const std::string AssetsManager::s_shaderDir(std::string(ASSETS_DIR) + "/shaders/");
Device* AssetsManager::s_pDevice{nullptr};

static void _vert_color_program_initailizer(ShaderProgram* program);

void AssetsManager::Initailize(Device* pDevice)
{
    assert(pDevice && pDevice->IsValid());
    s_pDevice = pDevice;
    s_programResourceInitailizer.insert(std::make_pair("vertex_color.vert.spv:vertex_color.frag.spv", _vert_color_program_initailizer));
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

    ShaderStageInfo vsi{vsName, VK_NULL_HANDLE, VK_SHADER_STAGE_VERTEX_BIT};
    ShaderStageInfo fsi{fsName, VK_NULL_HANDLE, VK_SHADER_STAGE_FRAGMENT_BIT};
    vsi.shaderMoudle = _load_shader_moudle((s_shaderDir + vs).c_str());
    fsi.shaderMoudle = _load_shader_moudle((s_shaderDir + fs).c_str());
    program->AddShader(vsi);
    program->AddShader(fsi);

    auto resourceInitailizer = s_programResourceInitailizer.find(k_program);
    if (resourceInitailizer == s_programResourceInitailizer.end())
    {
        LOGW("{} program can't find resource initailizer! shader load may be uncompeletly!", k_program);
        throw std::logic_error("program can't find resource initailizer!");
    }
    else
    {
        resourceInitailizer->second.operator()(program);
    }
    
    assert(program->Apply());

    return program;
}


void AssetsManager::Release()
 {
    for (auto &&itr : s_programs)
    {
        itr.second->Release();
    }
    s_programs.clear();
 }


VkShaderModule AssetsManager::_load_shader_moudle(const char* srcPath)
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
    shaderMouCreateInfo.pCode = (uint32_t*) spvSrc.data();
    shaderMouCreateInfo.codeSize = srcSz;
    assert(VKCALL_SUCCESS(vkCreateShaderModule(s_pDevice->GetHandle(), &shaderMouCreateInfo, nullptr, &shaderMou)));

    return shaderMou;
}



static void _vert_color_program_initailizer(ShaderProgram* program)
{
    program->AddResourceBinding(0, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    program->AddAttribute(Position, 0);
    program->AddAttribute(Color, 1);
}
