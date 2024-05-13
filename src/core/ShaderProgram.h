#pragma once
#include"core\CoreUtils.h"
#include<vector>
#include<map>
#include<string>
#include<optional>


#define MAX_SHADER_VARIABLE_NAME 32

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
    VertexAttribute attrType;
    size_t location;
    VkFormat format;
    char name[MAX_SHADER_VARIABLE_NAME];
};

enum UniformType
{
    UnknowUniformType,
    Boolen,
    Int,
    Float,
    Vector,
    Matrix,
    UserBlock,
    MaxUniformType,
};

enum ImageType
{
    UnKnowImageType,
    Tex1D,
    Tex2D,
    Tex3D,
    TexCube,
    MaxImageType,
};

struct UniformInfo
{
    uint32_t offset{0};
    uint32_t absoluteOffset{0};
    uint32_t size{0};
    uint32_t paddedSize{0};
    uint32_t arrayCnt{0};
    UniformType type{UnknowUniformType};
    char name[MAX_SHADER_VARIABLE_NAME];
};


struct ShaderBlockInfo
{
    uint32_t size{0};
    uint32_t paddedSize{0};
    std::vector<UniformInfo> uniforms{};
};


struct ShaderImageInfo
{
    ImageType type{UnKnowImageType};
    bool layerArrayed{false};
    bool multiSampled{false};
    bool sampled{false};
};



struct SetBindingInfo
{
    int set{-1};
    int binding{-1};
    int descriptorCount{0}; // 1 for none array bingding, 0 for none-boundary array binding, N for boundary array binding
    VkDescriptorType descriptorType{VK_DESCRIPTOR_TYPE_MAX_ENUM};
    VkShaderStageFlags stageFlags{0};
    std::optional<ShaderBlockInfo> blockInfo{};
    std::optional<ShaderImageInfo> imageInfo{};
    bool accessed{false};
    char name[MAX_SHADER_VARIABLE_NAME]{0};
    char structureName[MAX_SHADER_VARIABLE_NAME]{0};
   
};



class ShaderProgram
{
private:
    std::string _name{};
    
    std::vector<const ShaderStageInfo*> _shaders{};
    std::vector<VertexAttributeInfo> _attrInfos{};
    std::map<int, std::vector<SetBindingInfo>> _setbindingInfos{};
    
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

    bool AddShaderStage(const ShaderStageInfo* shaderInfo);
    bool HasShaderStage(VkShaderStageFlagBits stage) const;
    size_t GetShaderStageCount() const { return _shaders.size(); }
    const ShaderStageInfo* GetShaderStageInfo(VkShaderStageFlagBits stage) const;
    const std::vector<const ShaderStageInfo*>& GetShaderStageInfos() const { return _shaders; }

    void AddAttribute(const VertexAttributeInfo& attr) { _attrInfos.push_back(attr); }
    void AddAttribute(VertexAttributeInfo&& attr) { _attrInfos.push_back(std::move(attr)); }
    size_t GetAttributeCount() const { return _attrInfos.size(); }
    const std::vector<VertexAttributeInfo>& GetAttributes() const { return _attrInfos; }

    void AddSetBinding(int set, const std::vector<SetBindingInfo>& bindings) { _setbindingInfos[set] = bindings; }
    void AddSetBinding(int set, std::vector<SetBindingInfo>&& bindings) { _setbindingInfos[set] = std::move(bindings); }
    const std::vector<SetBindingInfo>* GetSetBinding(int set) const;
    int GetSetCount() const { return _setbindingInfos.size(); }
    bool HasSet(int set) const { return _setbindingInfos.find(set) != _setbindingInfos.end(); }

    VkPipelineLayout GetPipelineLayout() const { return _pipelineLayout; }
    Device* GetDevice() const { return _pDevice; }

    bool Apply();
    void Release();
    bool IsValid() const { return VKHANDLE_IS_NOT_NULL(_pipelineLayout); };

};


