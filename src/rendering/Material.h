#pragma once
#include<core\CoreUtils.h>
#include<core\PipelineState.h>
#include"MaterialProperty.h"
#include"PipelineManager.h"
#include<unordered_map>
#include<unordered_set>


class Material
{
private:
    ShaderProgram* _shaderProgram{ nullptr };
    std::unordered_map<size_t, MaterialProperty*> _properties{};
    std::unordered_map<size_t, Buffer*> _buffers{};
    VkDescriptorSet _vkSet{VK_NULL_HANDLE};

    RasterizationState _rasterizationState{};
    DepthStencilState _depthStencilState{};
    BlendMode _colorBlendMode{BlendMode::None};
    BlendMode _alphaBlendMode{BlendMode::None};
    ColorMask _colorMask{ColorMask::RGBA};
    
    std::unordered_set<MaterialProperty*> _dirtyProperties{};
    static std::unordered_set<Material*> s_DirtyMaterials;

    void _instantiate_set_binding_properties(const SetBindingInfo* setBinding);
    MaterialProperty* _instantitate_uniform_property(const SetBindingInfo* binding, const UniformInfo* uniform, Buffer* buffer);
    MaterialProperty* _instantitate_image_property(const SetBindingInfo* binding);
    void _destroy_property(MaterialProperty* prop);
    void _mark_property_dirty(MaterialProperty* p);

    MaterialProperty* _get_property(const char* name, MaterialProperty::Type t) const;
    
    template<typename T, typename P,  MaterialProperty::Type pt>
    T _get_property_value(const char* name) const 
    {
        MaterialProperty* p = _get_property(name, pt);
        if (p == nullptr)
            return T();
        
        return reinterpret_cast<P*>(p)->Get();
    }

    template<typename T, typename P,  MaterialProperty::Type pt>
    std::pair<T, size_t> _get_property_array_value(const char* name) const 
    {
        MaterialProperty* p = _get_property(name, pt);
        if (p == nullptr)
            return std::make_pair<T, size_t>(T(), 0);
        
        P* _p = reinterpret_cast<P*>(p);
        return std::make_pair<T, size_t>(_p->Get(), _p->GetSize());
    }

    template<typename T, typename P, MaterialProperty::Type pt>
    void _set_property_value(const char* name, T& val)
    {
        MaterialProperty* p = _get_property(name, pt);
        if (p == nullptr)
            return;
        
        reinterpret_cast<P*>(p)->Set(val);
        _mark_property_dirty(p);
    }

    template<typename T, typename P, MaterialProperty::Type pt>
    void _set_property_array_value(const char* name, const T* data, size_t cnt, size_t offset)
    {
        MaterialProperty* p = _get_property(name, pt);
        if (p == nullptr)
            return;
        reinterpret_cast<P*>(p)->Set(data, cnt, offset);
        _mark_property_dirty(p);
    }

    void _reset();
    void _update();


    friend PipelineState PipelineManager::MakePipelineState(const Mesh* mesh, const Material* material, const RenderPass* renderPass, size_t subPassIdx);

public:
    Material(ShaderProgram* shader);
    ~Material();

    NONE_COPYABLE_NONE_MOVEABLE(Material)

    const ShaderProgram* GetShaderProgram() const { return _shaderProgram; }
    void SetShaderProgram(ShaderProgram* program);
    VkPipelineLayout GetPipelineLayout() const { return _shaderProgram ? _shaderProgram->GetPipelineLayout() : VK_NULL_HANDLE; }
    void Bind(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;
    bool IsValid() const { return _shaderProgram != nullptr; }

    bool HasProperties() const { return _properties.size() > 0; }
    bool HasBool(const char* name) const { return _get_property(name, MaterialProperty::Bool) != nullptr; }
    bool HasInt(const char* name) const { return _get_property(name, MaterialProperty::Int) != nullptr; }
    bool HasFloat(const char* name) const { return _get_property(name, MaterialProperty::Float) != nullptr; }
    bool HasVector(const char* name) const { return _get_property(name, MaterialProperty::Vector) != nullptr; }
    bool HasMatrix(const char* name) const { return _get_property(name, MaterialProperty::Matrix) != nullptr; }
    bool HasCustomBlock(const char* name) const { return _get_property(name, MaterialProperty::UserBlock) != nullptr; }
    bool HasTexture(const char* name) const { return _get_property(name, MaterialProperty::Tex2D) != nullptr; }
    bool HasTextureCube(const char* name) const { return _get_property(name, MaterialProperty::TexCube) != nullptr; }
    bool HasBoolArray(const char* name) const { return _get_property(name, MaterialProperty::BoolArray) != nullptr; }
    bool HasIntArray(const char* name) const { return _get_property(name, MaterialProperty::IntArray) != nullptr; }
    bool HasFloatArray(const char* name) const { return _get_property(name, MaterialProperty::FloatArray) != nullptr; }
    bool HasVectorArray(const char* name) const { return _get_property(name, MaterialProperty::VectorArray) != nullptr; }
    bool HasMatrixArray(const char* name) const { return _get_property(name, MaterialProperty::MatrixArray) != nullptr; }

    bool GetBool(const char* name) const { return _get_property_value<bool, MaterialPropertyBool, MaterialProperty::Type::Bool>(name); }
    int GetInt(const char* name) const { return _get_property_value<int, MaterialPropertyInt, MaterialProperty::Type::Int>(name); }
    float GetFloat(const char* name) const { return _get_property_value<float, MaterialPropertyFloat, MaterialProperty::Type::Float>(name); }
    glm::vec4 GetVector(const char* name) const { return _get_property_value<glm::vec4, MaterialPropertyVector, MaterialProperty::Type::Vector>(name); }
    glm::mat4 GetMatrix(const char* name) const { return _get_property_value<glm::mat4, MaterialPropertyMatrix, MaterialProperty::Type::Matrix>(name); }
    const uint8_t* GetCustomBlock(const char* name) const { return _get_property_value<const uint8_t*, MaterialPropertyUserBlock, MaterialProperty::Type::UserBlock>(name); }
    Texture2D* GetTexture(const char* name) const { return _get_property_value<Texture2D*, MaterialPropertyTex2D, MaterialProperty::Type::Tex2D>(name); };
    TextureCube* GetTextureCube(const char* name) const { return _get_property_value<TextureCube*, MaterialPropertyTexCube, MaterialProperty::Type::TexCube>(name); };

    std::pair<const bool*, size_t> GetBoolArray(const char* name) const { return _get_property_array_value<const bool*, MaterialPropertyBoolArray, MaterialProperty::Type::BoolArray>(name); }
    std::pair<const int*, size_t> GetIntArray(const char* name) const { return _get_property_array_value<const int*, MaterialPropertyIntArray, MaterialProperty::Type::IntArray>(name); }
    std::pair<const float*, size_t> GetFloatArray(const char* name) const { return _get_property_array_value<const float*, MaterialPropertyFloatArray, MaterialProperty::Type::FloatArray>(name); }
    std::pair<const glm::vec4*, size_t> GetVectorArray(const char* name) const { return _get_property_array_value<const glm::vec4*, MaterialPropertyVectorArray, MaterialProperty::Type::VectorArray>(name); }
    std::pair<const glm::mat4*, size_t> GetMatrixArray(const char* name) const { return _get_property_array_value<const glm::mat4*, MaterialPropertyMatrixArray, MaterialProperty::Type::MatrixArray>(name); }

    void SetBool(const char* name, bool val) { _set_property_value<bool, MaterialPropertyBool, MaterialProperty::Type::Bool>(name, val); }
    void SetInt(const char* name, int val) { _set_property_value<int, MaterialPropertyInt, MaterialProperty::Type::Int>(name, val); }
    void SetFloat(const char* name, float val) { _set_property_value<float, MaterialPropertyFloat, MaterialProperty::Type::Float>(name, val); }
    void SetVector(const char* name,  glm::vec4& val) { _set_property_value<glm::vec4, MaterialPropertyVector, MaterialProperty::Type::Vector>(name, val); }
    void SetMatrix(const char* name,  glm::mat4& val) { _set_property_value<glm::mat4, MaterialPropertyMatrix, MaterialProperty::Type::Matrix>(name, val); }
    void SetCustomBlock(const char* name, const uint8_t* val, size_t len, size_t offset = 0);
    void SetTexture(const char* name, Texture2D* val) { _set_property_value<Texture2D*, MaterialPropertyTex2D, MaterialProperty::Type::Tex2D>(name, val); }
    void SetTextureCube(const char* name, TextureCube* val) { _set_property_value<TextureCube*, MaterialPropertyTexCube, MaterialProperty::Type::TexCube>(name, val); }

    void SetBoolArray(const char* name, const bool* data, size_t cnt, size_t offset = 0) { _set_property_array_value<bool, MaterialPropertyBoolArray, MaterialProperty::Type::BoolArray>(name, data, cnt, offset); }
    void SetIntArray(const char* name, const int* data, size_t cnt, size_t offset = 0) { _set_property_array_value<int, MaterialPropertyIntArray, MaterialProperty::Type::IntArray>(name, data, cnt, offset); }
    void SetFloatArray(const char* name, const float* data, size_t cnt, size_t offset = 0) { _set_property_array_value<float, MaterialPropertyFloatArray, MaterialProperty::Type::FloatArray>(name, data, cnt, offset); }
    void SetVectorArray(const char* name, const glm::vec4* data, size_t cnt, size_t offset = 0) { _set_property_array_value<glm::vec4, MaterialPropertyVectorArray, MaterialProperty::Type::VectorArray>(name, data, cnt, offset); }
    void SetMatrixArray(const char* name, const glm::mat4* data, size_t cnt, size_t offset = 0) { _set_property_array_value<glm::mat4, MaterialPropertyMatrixArray, MaterialProperty::Type::MatrixArray>(name, data, cnt, offset); }

    // rasterization state
    void SetCullFace(VkCullModeFlagBits cullMode) { _rasterizationState.cullMode = cullMode; }
    VkCullModeFlags GetCullFace() const { return _rasterizationState.cullMode; }
    void SetFrontFaceOrder(VkFrontFace order) { _rasterizationState.frontFace = order; }
    VkFrontFace GetFrontFaceOrder() const { return _rasterizationState.frontFace; }
    void SetFillFace(VkPolygonMode fillMode) { _rasterizationState.polygonMode = fillMode; }
    VkPolygonMode GetFillFace() const { return _rasterizationState.polygonMode; }
    void SetFillLineWidth(float w) { _rasterizationState.lineWidth = w; }
    float GetFillLineWidth() const { return _rasterizationState.lineWidth; }
    void EnableDepthBias() { _rasterizationState.depthBiasEnable = true; }
    void DisableDepthBias() { _rasterizationState.depthBiasEnable = false; }
    bool IsDepthBiasEnable() const { return _rasterizationState.depthBiasEnable; }
    void SetDepthBiasFactors(float constantFactor, float slotFactor, float clampFactor) { _rasterizationState.depthBiasConstantFactor = constantFactor; _rasterizationState.depthBiasSlopeFactor =  slotFactor; _rasterizationState.depthBiasClamp = clampFactor; }
    void GetDepthBiasFactors(float& constantFactor, float& slotFactor, float& clampFactor) const { constantFactor = _rasterizationState.depthBiasConstantFactor; slotFactor = _rasterizationState.depthBiasSlopeFactor; clampFactor = _rasterizationState.depthBiasClamp; }

    // depth stencil state
    void EnableZTest() { _depthStencilState.depthTestEnable = true; }
    void DisableZTest() { _depthStencilState.depthTestEnable = false; }
    bool IsZTestEnable() const { return _depthStencilState.depthTestEnable; }
    void EnableZWrite() { _depthStencilState.depthWriteEnable = true; }
    void DisableZWrite() { _depthStencilState.depthWriteEnable = false; }
    bool IsZWriteEnable() const { return _depthStencilState.depthWriteEnable; }
    void SetZTestOp(VkCompareOp op) { _depthStencilState.depthCompareOp; }
    void EnableStencilTest() { _depthStencilState.stencilTestEnable = true; }
    void DisableStencilTest() {_depthStencilState.stencilTestEnable = false; }
    bool IsStencilTestEnable() const { return _depthStencilState.stencilTestEnable; }
    void SetStencilOp(uint8_t ref, VkCompareOp cmp, uint8_t readMask = 255, uint8_t writeMask = 255, VkStencilOp passOp = VK_STENCIL_OP_REPLACE, VkStencilOp failOp = VK_STENCIL_OP_KEEP, VkStencilOp zFailOp = VK_STENCIL_OP_KEEP)
    {
        VkStencilOpState ss{};
        ss.reference = ref;
        ss.compareMask = readMask;
        ss.compareOp = cmp;
        ss.passOp = passOp;
        ss.writeMask = writeMask;
        ss.failOp = failOp;
        ss.depthFailOp = zFailOp;
        _depthStencilState.front = _depthStencilState.back = ss;
    }
    void GetStencilOp(uint8_t& ref, VkCompareOp& op, uint8_t& readMask, uint8_t& writeMask, VkStencilOp& passOp, VkStencilOp& failOp, VkStencilOp& zFailOp)
    {
        ref = _depthStencilState.front.reference;
        op = _depthStencilState.front.compareOp;
        readMask = _depthStencilState.front.compareMask;
        writeMask = _depthStencilState.front.writeMask;
        passOp = _depthStencilState.front.passOp;
        failOp = _depthStencilState.front.failOp;
        zFailOp = _depthStencilState.front.depthFailOp;
    }

    // blend
    void SetColorBlendMode(BlendMode mode) { _colorBlendMode = mode; }
    BlendMode GetColorBlendMode() const { return _colorBlendMode; }
    void SetAlphaBlendMode(BlendMode mode) { _alphaBlendMode = mode; }
    BlendMode GetAlphaBlendMode() const { return _alphaBlendMode; }
    void SetColorMask(ColorMask mask) { _colorMask = mask; }
    ColorMask GetColorMask() const { return _colorMask; }
    static void Update();
};


