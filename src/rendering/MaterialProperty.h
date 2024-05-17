#pragma once
#include<vulkan\vulkan.h>
#include<glm\glm.hpp>
#include<vector>
#include"core\CoreUtils.h"
#include"core/ShaderProgram.h"


class Buffer;
class Texture2D;
class TextureCube;

class MaterialProperty
{
public:
    enum Type
    {
        Float,
        FloatArray,
        Int,
        IntArray,
        Bool,
        BoolArray,
        Vector,
        VectorArray,
        Matrix,
        MatrixArray,
        UserBlock,      // struct inside uniform buffer
        UserBlockArray,
        Tex2D, // combine image sampler
        Tex2DArray, // multi layer texture
        TexCube,
        TexCubeArray,
        MaxMaterialPropertyType,
    };
    
public:
    const SetBindingInfo* const setBindingInfo;
    const char* const name;
    const size_t nameHashCode;
    const Type type;

public:
    MaterialProperty(const SetBindingInfo* sbi, const char* name, Type t);
    virtual ~MaterialProperty() {};
    virtual void Update(VkDevice device, VkDescriptorSet set) const = 0;
};

template <typename T>
class MaterialPropertyUniform : public MaterialProperty
{
private:
    T _data{};
    Buffer *_ubo;
    const UniformInfo* const _typeInfo;

public:
    MaterialPropertyUniform(const SetBindingInfo *uboInfo, Buffer *ubo, const UniformInfo *uniform);
    ~MaterialPropertyUniform(){};

    MaterialPropertyUniform(const MaterialPropertyUniform &) = delete;
    MaterialPropertyUniform(MaterialPropertyUniform &&) = delete;
    MaterialPropertyUniform &operator=(const MaterialPropertyUniform &) = delete;
    MaterialPropertyUniform &operator=(MaterialPropertyUniform &&) = delete;

    void Set(const T &val) { _data = val; }
    const T &Get() const { return _data; }
    void Update(VkDevice device, VkDescriptorSet set) const override;
};

typedef MaterialPropertyUniform<float> MaterialPropertyFloat;
typedef MaterialPropertyUniform<int> MaterialPropertyInt;
typedef MaterialPropertyUniform<bool> MaterialPropertyBool;
typedef MaterialPropertyUniform<glm::vec4> MaterialPropertyVector;
typedef MaterialPropertyUniform<glm::mat4> MaterialPropertyMatrix;

template <typename T>
class MaterialPropertyUniformArray : public MaterialProperty
{
private:
    std::vector<T> _data;
    Buffer *_ubo;
    const UniformInfo* const _typeInfo;

public:
    MaterialPropertyUniformArray(const SetBindingInfo *uboInfo, Buffer *ubo, const UniformInfo *uniform);
    ~MaterialPropertyUniformArray(){};

    MaterialPropertyUniformArray(const MaterialPropertyUniformArray &) = delete;
    MaterialPropertyUniformArray(MaterialPropertyUniformArray &&) = delete;
    MaterialPropertyUniformArray &operator=(const MaterialPropertyUniformArray &) = delete;
    MaterialPropertyUniformArray &operator=(MaterialPropertyUniformArray &&) = delete;

    void Set(const T *elements, size_t numElements, size_t offset = 0);
    const T *Get() const { return _data.data(); }
    size_t GetSize() const { return _data.size(); }
    void Update(VkDevice device, VkDescriptorSet set) const override;
};


typedef MaterialPropertyUniformArray<float> MaterialPropertyFloatArray;
typedef MaterialPropertyUniformArray<int> MaterialPropertyIntArray;
//typedef MaterialPropertyUniformArray<bool> MaterialPropertyBoolArray; //std::vector<bool> is a bitset in std implementaion
typedef MaterialPropertyUniformArray<glm::vec4> MaterialPropertyVectorArray;
typedef MaterialPropertyUniformArray<glm::mat4> MaterialPropertyMatrixArray;

class MaterialPropertyBoolArray : public MaterialProperty 
{
private:
    bool* _data;
    Buffer *_ubo;
    const UniformInfo* const _typeInfo;

public:
    MaterialPropertyBoolArray(const SetBindingInfo *uboInfo, Buffer *ubo, const UniformInfo *uniform);
    ~MaterialPropertyBoolArray();

    NONE_COPYABLE_NONE_MOVEABLE(MaterialPropertyBoolArray)

    const bool* Get() const { return _data; }
    void Set(const bool* elements, size_t numElements, size_t offset = 0);
    size_t GetSize() const { return _typeInfo->arrayCnt; }
    void Update(VkDevice device, VkDescriptorSet set) const override;
};

class MaterialPropertyUserBlock: public MaterialProperty
{
private:
    std::vector<uint8_t> _block;
    Buffer *_ubo;
    const UniformInfo *_typeInfo;

public:
    MaterialPropertyUserBlock(const SetBindingInfo *uboInfo, Buffer *ubo, const UniformInfo *uniform);
    ~MaterialPropertyUserBlock(){}

    NONE_COPYABLE_NONE_MOVEABLE(MaterialPropertyUserBlock)

    void Set(const uint8_t* data, size_t dataSz, size_t offet = 0);
    const uint8_t* Get() const { return _block.data(); }
    size_t GetSize() const { return _block.size(); }
    void Update(VkDevice device, VkDescriptorSet set) const override;
};


template<typename T>
class MaterialPropertyImage : MaterialProperty
{
private:
    const T* _tex;

public:
    MaterialPropertyImage(const SetBindingInfo* sbi);
    ~MaterialPropertyImage() {}

    void Set(const T* tex) { _tex = tex; }
    const T* Get() const { return _tex; }

    void Update(VkDevice device, VkDescriptorSet set) const override;
};

typedef MaterialPropertyImage<Texture2D> MaterialPropertyTex2D;
typedef MaterialPropertyImage<TextureCube>  MaterialPropertyTexCube;

