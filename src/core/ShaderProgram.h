#pragma once
#include"core\CoreUtils.h"
#include<vector>
#include<map>
#include<string>

class Device;

struct ShaderStageInfo
{   
    std::string pEntryName;
    std::string srcPath;
    std::vector<char> spvCodes;
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
    std::string _name{};
    
    std::vector<const ShaderStageInfo*> _shaders{};
    std::vector<VertexAttributeInfo> _attrInfos{};
    std::map<size_t, std::vector<VkDescriptorSetLayoutBinding>> _setLayoutBindings{};
    
    VkPipelineLayout _pipelineLayout{VK_NULL_HANDLE};
    Device* _pDevice{nullptr};

private:
    void _release_layouts(const std::vector<VkDescriptorSetLayout>& setLayouts);

public:
    ShaderProgram(Device* pDevice);
    ~ShaderProgram();

    NONE_COPYABLE_NONE_MOVEABLE(ShaderProgram)

    void SetName(const char* name) { _name = name; }
    const std::string& GetName() const { return _name; }

    bool AddShader(const ShaderStageInfo* shaderInfo);
    bool HasShaderStage(VkShaderStageFlagBits stage) const;
    size_t GetShaderStageCount() const { return _shaders.size(); }
    const ShaderStageInfo* GetShaderStageInfo(VkShaderStageFlagBits stage) const;
    const std::vector<const ShaderStageInfo*>& GetShaderStageInfos() const { return _shaders; }

    bool AddAttribute(Attribute attr, size_t location);
    size_t GetAttributeCount() const { return _attrInfos.size(); }
    const std::vector<VertexAttributeInfo>& GetAttributes() const { return _attrInfos; }

    bool AddDescriptorSetBinding(size_t setIdx, size_t bindingLocation, VkDescriptorType resourceType, size_t resourceArrayElementCnt, VkShaderStageFlags accessStages);
    const std::vector<VkDescriptorSetLayoutBinding>* GetDescriptorSetBindings(size_t setIdx) const;
    size_t GetDescriptorSetCount() const { return _setLayoutBindings.size(); }

    VkPipelineLayout GetPipelineLayout() const { return _pipelineLayout; }
    Device* GetDevice() const { return _pDevice; }

    bool Apply();
    void Release();
    bool IsValid() const { return VKHANDLE_IS_NOT_NULL(_pipelineLayout); };

};


