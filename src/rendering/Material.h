#pragma once
#include<core\CoreUtils.h>
#include"MaterialProperty.h"
#include<unordered_map>
#include<unordered_set>

class ShaderProgram;



class Material
{
private:
    ShaderProgram* _shaderProgram{ nullptr };
    std::unordered_map<size_t, MaterialProperty*> _properties{};
    std::unordered_map<size_t, Buffer*> _buffers{};
    VkDescriptorSet _vkSet{VK_NULL_HANDLE};
    
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

    static void Update();
};


