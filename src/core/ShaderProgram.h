#pragma once
#include"core\CoreUtils.h"
#include<vector>
#include<map>
#include<string>

class Device;

struct ShaderStageInfo
{
    std::string pEntryName;
    VkShaderModule shaderMoudle;
    VkShaderStageFlagBits stage;
};

class ShaderProgram
{
private:
    std::vector<ShaderStageInfo> _shaders{};
    std::map<size_t, std::vector<VkDescriptorSetLayoutBinding>> _setLayoutBindings{};
    std::map<size_t, size_t> _setIdxToLayoutIdx{};
    std::map<size_t, size_t> _layoutIdxToSetIdx{};

    std::vector<VkDescriptorSetLayout> _setLayouts{};
    VkPipelineLayout _pipelineLayout{VK_NULL_HANDLE};

    Device* _pDevice{nullptr};

private:
    void _release_shaders();
    void _release_layouts();

public:
    ShaderProgram(Device* pDevice);
    ~ShaderProgram();

    NONE_COPYABLE_NONE_MOVEABLE(ShaderProgram)

    bool AddShader(const char* srcFile, const char* entryPoint, VkShaderStageFlagBits shaderStage);
    size_t GetShaderStageCount() const { return _shaders.size(); }
    ShaderStageInfo GetShaderStageInfo(size_t idx) const { return _shaders[idx]; }
    bool HasShaderStage(VkShaderStageFlagBits stage) const;
    bool GetShaderStageInfo(VkShaderStageFlagBits stage, ShaderStageInfo* pResult) const;

    bool AddResourceBinding(size_t setIdx, size_t bindingLocation, VkDescriptorType resourceType, size_t resourceArrayElementCnt, VkShaderStageFlags accessStages);
    bool HasResrouceSetLayout(size_t setIdx) const { return _setIdxToLayoutIdx.find(setIdx) != _setIdxToLayoutIdx.end(); }
    VkDescriptorSetLayout GetResourceSetLayout(size_t setIdx) const { return _setLayouts[_setIdxToLayoutIdx.at(setIdx)]; }
    size_t GetResourceSetLayoutCount() const {return _setLayouts.size(); }
    VkDescriptorSetLayout GetResourceSetLayout(size_t idx) { return _setLayouts[idx]; }
    std::vector<VkDescriptorSetLayout> GetResourceSetLayouts() const { return _setLayouts; }
    std::vector<size_t> GetResourceSetIndices() const;

    VkPipelineLayout GetPipelineLayout() const { return _pipelineLayout; }

    bool Create();
    void Release();
    bool IsValid() const { return VKHANDLE_IS_NOT_NULL(_pipelineLayout); };

};


