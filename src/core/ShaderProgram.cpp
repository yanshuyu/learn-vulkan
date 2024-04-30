#include"ShaderProgram.h"
#include"core\Device.h"
#include<fstream>
#include<spirv-reflect/spirv_reflect.h>


ShaderProgram::ShaderProgram(Device* pDevice)
: _pDevice(pDevice)
{
    assert(pDevice && pDevice->IsValid());
}

ShaderProgram::~ShaderProgram()
{
    Release();
    _pDevice = nullptr;
}


bool ShaderProgram::AddShader(const ShaderStageInfo* shaderInfo)
{
    if (IsValid())
        return false;
    
    if (HasShaderStage(shaderInfo->stage))
        return false;

    _shaders.push_back(shaderInfo);    
    return true;
}



bool ShaderProgram::HasShaderStage(VkShaderStageFlagBits stage) const
{
    auto pos = std::find_if(_shaders.begin(), _shaders.end(), [=](const ShaderStageInfo* shaderInfo){
        return shaderInfo->stage == stage;
    });

    return pos != _shaders.end();
}


const ShaderStageInfo* ShaderProgram::GetShaderStageInfo(VkShaderStageFlagBits stage) const
{
    auto pos = std::find_if(_shaders.begin(), _shaders.end(), [=](const ShaderStageInfo* shaderInfo){
        return shaderInfo->stage == stage;
    });

    if (pos == _shaders.end())
        return nullptr;
    
    return *pos;
}


bool ShaderProgram::AddAttribute(VertexAttribute attr, size_t loc, VkFormat fmt, const char* name)
{
    auto itr = std::find_if(_attrInfos.begin(), _attrInfos.end(), [=](const VertexAttributeInfo& attrInfo)
    {
        return attrInfo.attrType == attr;
    });

    if (itr == _attrInfos.end())
    {
        VertexAttributeInfo attrInfo{};
        attrInfo.attrType = attr;
        attrInfo.location = loc;
        attrInfo.format = fmt;
        strcpy_s(attrInfo.name, MAX_SHADER_VARIABLE_NAME, name);
        _attrInfos.push_back(std::move(attrInfo));
        return true;
    }
    
    return false;    
}


bool ShaderProgram::AddDescriptorSetBinding(size_t setIdx,
                                       size_t bindingLocation,
                                       VkDescriptorType resourceType,
                                       size_t resourceArrayElementCnt,
                                       VkShaderStageFlags accessStages)
{
    if (IsValid())
        return false;

    auto setItr = _setLayoutBindings.find(setIdx);
    if (setItr == _setLayoutBindings.end())
    {
        auto result = _setLayoutBindings.insert(std::make_pair(setIdx, std::vector<VkDescriptorSetLayoutBinding>()));
        setItr = result.first;
    }

    auto bindingItr = std::find_if(setItr->second.begin(), setItr->second.end(), [=](const VkDescriptorSetLayoutBinding& binding)
    {
        return binding.binding == bindingLocation;
    });

    if (bindingItr == setItr->second.end())
    {
         bindingItr = setItr->second.emplace(setItr->second.end());
    }
    bindingItr->binding = bindingLocation;
    bindingItr->descriptorType = resourceType;
    bindingItr->descriptorCount = resourceArrayElementCnt;
    bindingItr->stageFlags = accessStages;

    return true;

}


const std::vector<VkDescriptorSetLayoutBinding>* ShaderProgram::GetDescriptorSetBindings(size_t setIdx) const
{
    auto result = _setLayoutBindings.find(setIdx);
    if (result == _setLayoutBindings.end())
        return nullptr;
    
    return &result->second;
}



bool ShaderProgram::Apply()
{
    SpvReflectShaderModule refModule{};
    spvReflectCreateShaderModule(_shaders[0]->spvCodes.size(), _shaders[0]->spvCodes.data(), &refModule);
    auto entryInfo = spvReflectGetEntryPoint(&refModule, "main");

    for (size_t i = 0; i < entryInfo->interface_variable_count; i++)
    {
        auto& itf = entryInfo->interface_variables[i];
        LOGI("var name: {} location: {} blitin: {}", itf.name, itf.location, itf.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN);
    }
    

    for (size_t i = 0; i < entryInfo->descriptor_set_count; i++)
    {
        auto set = entryInfo->descriptor_sets[i];
        int x;
    }
    
    
    

    uint32_t attrCnt = 0;
    spvReflectEnumerateInterfaceVariables(&refModule, &attrCnt, nullptr);
    std::vector<SpvReflectInterfaceVariable*> inputAttrs(attrCnt);
    spvReflectEnumerateInterfaceVariables(&refModule, &attrCnt, inputAttrs.data());
    spvReflectDestroyShaderModule(&refModule);

    // create decriptor set layout
    std::vector<VkDescriptorSetLayout> setLayouts(_setLayoutBindings.size(), VK_NULL_HANDLE);
    size_t idx = 0;
    for (auto &&setLayoutBindings : _setLayoutBindings)
    {
        VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        createInfo.bindingCount = setLayoutBindings.second.size();
        createInfo.pBindings = setLayoutBindings.second.data();
        VkDescriptorSetLayout createdSetLayout{VK_NULL_HANDLE};
        if (VKCALL_FAILED(vkCreateDescriptorSetLayout(_pDevice->GetHandle(), &createInfo, nullptr, &createdSetLayout)))
        {
            _release_layouts(setLayouts);
            return false;
        }

        setLayouts[idx] = createdSetLayout;
        idx++;
    }
    
    // create pipeline layout
    VkPipelineLayoutCreateInfo createinfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createinfo.setLayoutCount = setLayouts.size();
    createinfo.pSetLayouts = setLayouts.data();
    if (VKCALL_FAILED(vkCreatePipelineLayout(_pDevice->GetHandle(), &createinfo, nullptr, &_pipelineLayout)))
    {
        _release_layouts(setLayouts);
        return false;
    }

    _release_layouts(setLayouts);
    return true;
}



void ShaderProgram::Release()
{
    if (!IsValid())
        return;
        
    vkDestroyPipelineLayout(_pDevice->GetHandle(), _pipelineLayout, nullptr);
    VKHANDLE_SET_NULL(_pipelineLayout);
}



void ShaderProgram::_release_layouts(const std::vector<VkDescriptorSetLayout>& setLayouts)
{
    for (size_t i = 0; i < setLayouts.size(); i++)
    {
        if (VKHANDLE_IS_NOT_NULL(setLayouts[i]))
            vkDestroyDescriptorSetLayout(_pDevice->GetHandle(), setLayouts[i], nullptr);
    }
}

