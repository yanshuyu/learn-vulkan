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


struct VertexAttributeInfo
{
    Attribute attrType;
    size_t location;
};


class ShaderProgram
{
private:
    std::vector<ShaderStageInfo> _shaders{};
    std::map<size_t, std::vector<VkDescriptorSetLayoutBinding>> _setLayoutBindings{};
    std::vector<VkDescriptorSetLayout> _setLayouts{};
    std::map<size_t, size_t> _setIdxToLayoutIdx{};
    std::vector<size_t> _layoutIdxToSetIdx{};

    std::vector<VertexAttributeInfo> _attrInfos{};

    VkPipelineLayout _pipelineLayout{VK_NULL_HANDLE};

    Device* _pDevice{nullptr};

    std::string _name{};

private:
    void _release_shaders();
    void _release_layouts();

public:
    ShaderProgram(Device* pDevice);
    ~ShaderProgram();

    NONE_COPYABLE_NONE_MOVEABLE(ShaderProgram)

    void SetName(const char* name) { _name = name; }
    const char* GetName() const { return _name.c_str(); }

    bool AddShader(const ShaderStageInfo& shaderInfo);
    bool HasShaderStage(VkShaderStageFlagBits stage) const;
    size_t GetShaderStageCount() const { return _shaders.size(); }
    bool ShaderProgram::GetShaderStageInfo(VkShaderStageFlagBits stage, ShaderStageInfo* pResult) const;
    const ShaderStageInfo& GetShaderStageInfo(size_t idx) const { return _shaders[idx]; }
    const std::vector<ShaderStageInfo>& GetShaderStageInfos() const { return _shaders; }

    bool AddAttribute(Attribute attr, size_t location);
    size_t GetAttributeCount() const { return _attrInfos.size(); }
    const std::vector<VertexAttributeInfo>& GetAttributes() const { return _attrInfos; }

    bool AddResourceBinding(size_t setIdx, size_t bindingLocation, VkDescriptorType resourceType, size_t resourceArrayElementCnt, VkShaderStageFlags accessStages);
    bool HasResrouceSetLayout(size_t setIdx) const { return _setIdxToLayoutIdx.find(setIdx) != _setIdxToLayoutIdx.end(); }
    VkDescriptorSetLayout GetResourceSetLayout(size_t setIdx) const { return _setLayouts[_setIdxToLayoutIdx.at(setIdx)]; }
    VkDescriptorSetLayout GetResourceSetLayout(size_t idx) { return _setLayouts[idx]; }
    size_t GetResourceSetLayoutCount() const {return _setLayouts.size(); }
    const std::vector<VkDescriptorSetLayout>& GetResourceSetLayouts() const { return _setLayouts; }
    const std::vector<size_t>& GetResourceSetIndices() const { return _layoutIdxToSetIdx; }
    const std::vector<VkDescriptorSetLayoutBinding>& GetResourceSetLayoutBindins(size_t setIdx) const { return  _setLayoutBindings.at(setIdx); }

    VkPipelineLayout GetPipelineLayout() const { return _pipelineLayout; }

    Device* GetDevice() const { return _pDevice; }

    bool Apply();
    void ReleaseShaderMoudles();
    void Release();
    bool IsValid() const { return VKHANDLE_IS_NOT_NULL(_pipelineLayout); };

};


