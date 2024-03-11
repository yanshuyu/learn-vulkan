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


bool ShaderProgram::AddShader(const char* srcFile, const char* entryPoint, VkShaderStageFlagBits shaderStage)
{
    if (IsValid())
        return false;
    
    std::ifstream fs(srcFile, std::ios_base::ate);
    if (!fs.is_open())
        return false;
    
    size_t srcSz = fs.tellg();
    fs.seekg(std::ios_base::beg);
    std::vector<char> spvSrc(srcSz);
    fs.read(spvSrc.data(), srcSz);
    fs.close();

    VkShaderModule shaderMou{VK_NULL_HANDLE};
    VkShaderModuleCreateInfo shaderMouCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    shaderMouCreateInfo.pCode = (uint32_t*) spvSrc.data();
    shaderMouCreateInfo.codeSize = srcSz;
    if (VKCALL_FAILED(vkCreateShaderModule(_pDevice->GetHandle(), &shaderMouCreateInfo, nullptr, &shaderMou)))
    {
        _release_shaders();
        return false;
    }

    ShaderStageInfo shaderInfo{};
    shaderInfo.shaderMoudle = shaderMou;
    shaderInfo.pEntryName = entryPoint;
    shaderInfo.stage = shaderStage;
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
        _setLayoutBindings.insert(std::make_pair(setIdx, std::vector<VkDescriptorSetLayoutBinding>()));
        setItr = _setLayoutBindings.find(setIdx);
    }

    auto bindingItr = std::find_if(setItr->second.begin(), setItr->second.end(), [=](const VkDescriptorSetLayoutBinding& binding)
    {
        return binding.binding == bindingLocation;
    });

    if (bindingItr == setItr->second.end())
    {
        setItr->second.push_back({});
        bindingItr = setItr->second.end();
        bindingItr--;
    }

    bindingItr->binding = bindingLocation;
    bindingItr->descriptorType = resourceType;
    bindingItr->descriptorCount = resourceArrayElementCnt;
    bindingItr->stageFlags = accessStages;

    return true;

}



bool ShaderProgram::Create()
{
    // create decriptor set layout
    _setLayouts.resize(_setLayoutBindings.size(), VK_NULL_HANDLE);
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


    
 std::vector<size_t> ShaderProgram::GetResourceSetIndices() const
 {
    std::vector<size_t> setIndices{};
    setIndices.reserve(_layoutIdxToSetIdx.size());
    for (auto &&layoutIdxToSetIdx : _layoutIdxToSetIdx)
        setIndices.push_back(layoutIdxToSetIdx.second);
    
    return std::move(setIndices);
 }


void ShaderProgram::Release()
{
    if (!IsValid())
        return;

    _release_layouts();
    _release_shaders();
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