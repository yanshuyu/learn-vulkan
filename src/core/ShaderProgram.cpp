#include"ShaderProgram.h"
#include"core\Device.h"
#include"rendering\DescriptorSetManager.h"
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

void ShaderProgram::GetSets(size_t* sets)
{
    int idx = 0;
    for (auto &&si : _setbindingInfos)
    {
        sets[idx] = si.first;
    }
}


bool ShaderProgram::Apply()
{
    // create decriptor set layout
    std::map<int, std::vector<VkDescriptorSetLayoutBinding>> setLayoutBindings{};
    for (auto &&sbi : _setbindingInfos) 
    {
        std::vector<VkDescriptorSetLayoutBinding> bindings(sbi.second.size());
        for (size_t idx = 0; idx < sbi.second.size(); idx++)
        {
            bindings[idx].binding = sbi.second[idx].binding;
            bindings[idx].descriptorType = sbi.second[idx].descriptorType;
            bindings[idx].descriptorCount = sbi.second[idx].descriptorCount;
            bindings[idx].stageFlags = sbi.second[idx].stageFlags;
        }
        setLayoutBindings.insert(std::make_pair(sbi.first, std::move(bindings)));
    }

    int maxSetIdx = setLayoutBindings.size() > 0 ? setLayoutBindings.rbegin()->first : 0;
    std::vector<VkDescriptorSetLayout> setLayouts{};
    if (maxSetIdx > 0)
    {
        setLayouts.resize(maxSetIdx + 1, VK_NULL_HANDLE);
        for (size_t idx = 0; idx <= maxSetIdx; idx++)
        {
            if (setLayoutBindings.find(idx) == setLayoutBindings.end()) // descritor set gap, insert a dummy set
            {
                setLayouts[idx] = DescriptorSetManager::GetDummySetLayout();
                LOGW("Shader program({}) find missing set({}), will automatically replace a dummy set!", _name.c_str(), idx);
            }
            else
            {
                VkDescriptorSetLayoutCreateInfo createInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
                createInfo.bindingCount = setLayoutBindings[idx].size();
                createInfo.pBindings = setLayoutBindings[idx].data();
                VkDescriptorSetLayout createdSetLayout{VK_NULL_HANDLE};
                if (VKCALL_FAILED(vkCreateDescriptorSetLayout(_pDevice->GetHandle(), &createInfo, nullptr, &createdSetLayout)))
                {
                    LOGE("Shader Program({}) Failed to create set({})'s layout!", _name.c_str(), idx);
                    _release_layouts(setLayouts);
                    return false;
                }
                setLayouts[idx] = createdSetLayout;
            }
        }
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
        if (VKHANDLE_IS_NULL(setLayouts[i]) || setLayouts[i] == DescriptorSetManager::GetDummySetLayout())
            continue;
        
        vkDestroyDescriptorSetLayout(_pDevice->GetHandle(), setLayouts[i], nullptr);
    }
}

