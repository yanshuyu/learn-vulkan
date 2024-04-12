#include"ShaderReflection.h"
#include<core\ShaderProgram.h>
#include"DescriptorSetManager.h"

#define DEC_FN_PARSER(FnName) bool FnName(ShaderProgram*);
DEC_FN_PARSER(_vertex_color_parser)


std::unordered_map<std::string, std::function<bool(ShaderProgram*)>> ShaderReflection::s_ShaderParser{};


void ShaderReflection::RegisterParser(const char* hash, Fn_Parser parser)
{
    if (s_ShaderParser.find(hash) != s_ShaderParser.end())
    {
        LOGW("Try to register multiple instance shader parser of program: {}", hash);
        return;
    }

    s_ShaderParser[hash] = parser;
}


void ShaderReflection::Initailze()
{
    RegisterParser("vertex_color.vert.spv:vertex_color.frag.spv", _vertex_color_parser);
}


void ShaderReflection::DeInitailize()
{
    s_ShaderParser.clear();
}

void ShaderReflection::Parse(ShaderProgram* program)
{
    auto parser = s_ShaderParser.find(program->GetName());
    if (parser == s_ShaderParser.end())
    {
        LOGE("Can't find shader parser for program: {}", program->GetName());
        throw std::runtime_error("shader parse error");
    }
    assert(parser->second(program));
    _parse_common(program);
}


void ShaderReflection::_parse_common(ShaderProgram* program)
{
    // add per frame bindings
    auto bindings = DescriptorSetManager::GetSetLayoutBindings(PerFrame, 0);
    for (auto &&binding : *bindings)
    {
        program->AddDescriptorSetBinding(PerFrame, binding.binding, binding.descriptorType, binding.descriptorCount, binding.stageFlags);
    }

    // add per camera bindings
    bindings = DescriptorSetManager::GetSetLayoutBindings(PerCamera, 0);
    for (auto &&binding : *bindings)
    {
        program->AddDescriptorSetBinding(PerCamera, binding.binding, binding.descriptorType, binding.descriptorCount, binding.stageFlags);
    }

    // add per object bindings
    bindings = DescriptorSetManager::GetSetLayoutBindings(PerObject, 0);
    for (auto &&binding : *bindings)
    {
        program->AddDescriptorSetBinding(PerObject, binding.binding, binding.descriptorType, binding.descriptorCount, binding.stageFlags);
    }
    
}


static bool _vertex_color_parser(ShaderProgram* program)
{
    program->AddAttribute(Position, 0);
    program->AddAttribute(Color, 1);
    program->AddAttribute(UV0, 2);

    program->AddDescriptorSetBinding(PerMaterial, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, VK_SHADER_STAGE_FRAGMENT_BIT);
    
    return true;
}



