#include"ShaderProgram.h"
#include"core\Device.h"
#include<fstream>


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


bool ShaderProgram::AddShader(const ShaderStageInfo& shaderInfo)
{
    if (IsValid())
        return false;
    
    auto pos = std::find_if(_shaders.begin(), _shaders.end(), [&](const ShaderStageInfo& _shaderInfo){
        return _shaderInfo.stage == shaderInfo.stage;
    });
    
    if (pos != _shaders.end())
        return false;

    _shaders.push_back(shaderInfo);    
    return true;
}



bool ShaderProgram::HasShaderStage(VkShaderStageFlagBits stage) const
{
    auto pos = std::find_if(_shaders.begin(), _shaders.end(), [=](const ShaderStageInfo& shaderInfo){
        return shaderInfo.stage == stage;
    });

    return pos != _shaders.end();
}


bool ShaderProgram::GetShaderStageInfo(VkShaderStageFlagBits stage, ShaderStageInfo* pResult) const
{
    auto pos = std::find_if(_shaders.begin(), _shaders.end(), [=](const ShaderStageInfo& shaderInfo){
        return shaderInfo.stage == stage;
    });

    if (pos == _shaders.end())
        return false;
    
    *pResult = *pos;
    return true;
}


bool ShaderProgram::AddAttribute(Attribute attr, size_t location)
{
    auto itr = std::find_if(_attrInfos.begin(), _attrInfos.end(), [=](const VertexAttributeInfo& attrInfo)
    {
        return attrInfo.attrType == attr;
    });

    if (itr == _attrInfos.end())
    {
        VertexAttributeInfo attrInfo{};
        attrInfo.attrType = attr;
        attrInfo.location = location;
        _attrInfos.push_back(attrInfo);
    }
    
    return false;    
}


bool ShaderProgram::AddResourceBinding(size_t setIdx,
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



bool ShaderProgram::Apply()
{
    // create decriptor set layout
    _setLayouts.resize(_setLayoutBindings.size(), VK_NULL_HANDLE);
    _layoutIdxToSetIdx.resize(_setLayoutBindings.size(), -1);
    size_t idx = 0;
    for (auto &&setLayoutBindings : _setLayoutBindings)
    {
        VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        createInfo.bindingCount = setLayoutBindings.second.size();
        createInfo.pBindings = setLayoutBindings.second.data();
        VkDescriptorSetLayout createdSetLayout{VK_NULL_HANDLE};
        if (VKCALL_FAILED(vkCreateDescriptorSetLayout(_pDevice->GetHandle(), &createInfo, nullptr, &createdSetLayout)))
        {
            _release_layouts();
            return false;
        }

        _setLayouts[idx] = createdSetLayout;
        _setIdxToLayoutIdx[setLayoutBindings.first] = idx;
        _layoutIdxToSetIdx[idx] = setLayoutBindings.first;
        idx++;
    }
    
    // create pipeline layout
    VkPipelineLayoutCreateInfo createinfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    createinfo.setLayoutCount = _setLayouts.size();
    createinfo.pSetLayouts = _setLayouts.data();
    if (VKCALL_FAILED(vkCreatePipelineLayout(_pDevice->GetHandle(), &createinfo, nullptr, &_pipelineLayout)))
    {
        _release_layouts();
        return false;
    }

    return true;
}



void ShaderProgram::Release()
{
    ReleaseShaderMoudles();

    if (!IsValid())
        return;
        
    _release_layouts();
    _release_shaders();
}


void ShaderProgram::ReleaseShaderMoudles()
{
    for (size_t i = 0; i < _shaders.size(); i++)
    {
        if (VKHANDLE_IS_NOT_NULL(_shaders[i].shaderMoudle))
        {
            vkDestroyShaderModule(_pDevice->GetHandle(), _shaders[i].shaderMoudle, nullptr);
            VKHANDLE_SET_NULL(_shaders[i].shaderMoudle);
        }
    }
}

void ShaderProgram::_release_layouts()
{
    for (size_t i = 0; i < _setLayouts.size(); i++)
    {
        if (VKHANDLE_IS_NOT_NULL(_setLayouts[i]))
            vkDestroyDescriptorSetLayout(_pDevice->GetHandle(), _setLayouts[i], nullptr);
    }

    _setLayouts.clear();
    _setIdxToLayoutIdx.clear();
    _layoutIdxToSetIdx.clear();

    if (VKHANDLE_IS_NOT_NULL(_pipelineLayout))
    {
        vkDestroyPipelineLayout(_pDevice->GetHandle(), _pipelineLayout, nullptr);
        VKHANDLE_SET_NULL(_pipelineLayout);
    }
}


void ShaderProgram::_release_shaders()
{
    for (size_t i = 0; i < _shaders.size(); i++)
    {
        if (VKHANDLE_IS_NOT_NULL(_shaders[i].shaderMoudle))
            vkDestroyShaderModule(_pDevice->GetHandle(), _shaders[i].shaderMoudle, nullptr);
    }
    _shaders.clear();
}