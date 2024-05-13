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


bool ShaderProgram::AddShaderStage(const ShaderStageInfo* shaderInfo)
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


const std::vector<SetBindingInfo>* ShaderProgram::GetSetBinding(int set) const
{
    auto pos = _setbindingInfos.find(set);
    if (pos == _setbindingInfos.end())
        return nullptr;
    
    return &pos->second;
}


bool ShaderProgram::Apply()
{
    // create decriptor set layout
  
    std::vector<std::vector<VkDescriptorSetLayoutBinding>> setLayoutBindings(_setbindingInfos.size());
    int setIdx = 0;
    for (auto &&sbi : _setbindingInfos) 
    {
        setLayoutBindings[setIdx].reserve(sbi.second.size());
        for (size_t bindingIdx = 0; bindingIdx < sbi.second.size(); bindingIdx++)
        {
            VkDescriptorSetLayoutBinding aBinding{};
            const SetBindingInfo& aBindingInfo = sbi.second[bindingIdx];
            aBinding.binding = aBindingInfo.binding;
            aBinding.descriptorType = aBindingInfo.descriptorType;
            aBinding.descriptorCount = aBindingInfo.descriptorCount;
            aBinding.stageFlags = aBindingInfo.stageFlags;
            setLayoutBindings[setIdx].push_back(aBinding);
        }
        setIdx++;
    }

    std::vector<VkDescriptorSetLayout> setLayouts{};
    setLayouts.reserve(_setbindingInfos.size());
    for (setIdx = 0; setIdx < setLayoutBindings.size(); setIdx++)
    {
        VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        createInfo.bindingCount = setLayoutBindings[setIdx].size();
        createInfo.pBindings = setLayoutBindings[setIdx].data();
        VkDescriptorSetLayout createdSetLayout{VK_NULL_HANDLE};
        if (VKCALL_FAILED(vkCreateDescriptorSetLayout(_pDevice->GetHandle(), &createInfo, nullptr, &createdSetLayout)))
        {
            LOGE("Shader Program({}) Failed to create set({})'s layout!", _name.c_str(), setIdx);
            _release_layouts(setLayouts);
            return false;
        }

        setLayouts.push_back(createdSetLayout);
    }
    

    
    // create pipeline layout
    VkPipelineLayoutCreateInfo createinfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createinfo.setLayoutCount = setLayouts.size();
    createinfo.pSetLayouts = setLayouts.data();
    if (VKCALL_FAILED(vkCreatePipelineLayout(_pDevice->GetHandle(), &createinfo, nullptr, &_pipelineLayout)))
    {
        LOGE("Shader Program({}) failed to create pipeline layout!", _name.c_str());
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

