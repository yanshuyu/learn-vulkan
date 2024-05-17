#include"MaterialProperty.h"
#include"core/Buffer.h"
#include"rendering/Texture2D.h"
#include"rendering/TextureCube.h"


static MaterialProperty::Type _uniform_type_to_property_type(UniformType ut, bool isArray)
{
    switch (ut)
    {
    case UniformType::Boolen:      
        return isArray ? MaterialProperty::Type::BoolArray : MaterialProperty::Type::Bool;
    case UniformType::Int:
        return isArray ? MaterialProperty::Type::IntArray : MaterialProperty::Type::Int;
    case UniformType::Float:
        return isArray ? MaterialProperty::FloatArray : MaterialProperty::Float;
    case UniformType::Vector:
        return isArray ? MaterialProperty::Type::VectorArray : MaterialProperty::Type::Vector;
    case UniformType::Matrix:
        return isArray ? MaterialProperty::Type::MatrixArray : MaterialProperty::Type::Matrix;
    case UniformType::UserBlock:
        return isArray ? MaterialProperty::Type::UserBlockArray : MaterialProperty::UserBlock;
    }   
}

static MaterialProperty::Type _image_type_to_property_type(ImageType it, bool isArray)
{
    switch (it)
    {
    case ImageType::Tex2D:
        return isArray ? MaterialProperty::Type::Tex2DArray : MaterialProperty::Type::Tex2D;
    case ImageType::TexCube:
        return isArray ? MaterialProperty::Type::TexCubeArray : MaterialProperty::Type::TexCube;    
    default:
        return MaterialProperty::Type::MaxMaterialPropertyType;
    }
}

MaterialProperty::MaterialProperty(const SetBindingInfo* sbi, const char* propName, Type t)
: setBindingInfo(sbi)
, name(propName)
, nameHashCode(std::hash<std::string>()(name))
, type(t)
{
}


template<typename T>
MaterialPropertyUniform<T>::MaterialPropertyUniform(const SetBindingInfo* uboInfo,  Buffer* ubo, const UniformInfo* uniform)
: MaterialProperty(uboInfo, uniform->name, _uniform_type_to_property_type(uniform->type, false))
, _typeInfo(uniform)
, _ubo(ubo)
{
    assert(uboInfo->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(_typeInfo->arrayCnt == 1);
}

template<typename T>
void MaterialPropertyUniform<T>::Update(VkDevice device, VkDescriptorSet set) const
{
    //_ubo->Map();
    _ubo->SetData((uint8_t*)&_data, _typeInfo->size, _typeInfo->offset);
    //_ubo->UnMap();
}


template<typename T>
MaterialPropertyUniformArray<T>::MaterialPropertyUniformArray(const SetBindingInfo* uboInfo,  Buffer* ubo, const UniformInfo* uniform)
: MaterialProperty(uboInfo, uniform->name, _uniform_type_to_property_type(uniform->type, true))
, _typeInfo(uniform)
, _ubo(ubo)
{
    assert(uboInfo->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(_typeInfo->arrayCnt > 1);
    _data.resize(uniform->arrayCnt);

}


template<typename T>
void MaterialPropertyUniformArray<T>::Set(const T *elements, size_t numElements, size_t offset)
{
    for (size_t i=offset, j=0; i < _data.size() && j < numElements; i++, j++)
    {
        _data[i] = elements[j];
    }
    
}

template<typename T>
void MaterialPropertyUniformArray<T>::Update(VkDevice device, VkDescriptorSet set) const
{
    _ubo->SetData((uint8_t*)_data.data(), _typeInfo->paddedSize, _typeInfo->offset);
}

MaterialPropertyBoolArray::MaterialPropertyBoolArray(const SetBindingInfo *uboInfo, Buffer *ubo, const UniformInfo *uniform)
: MaterialProperty(uboInfo, uniform->name, MaterialProperty::Type::BoolArray)
, _typeInfo(uniform)
, _ubo(ubo)
{
    assert(uboInfo->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(_typeInfo->arrayCnt > 1);
    assert(_typeInfo->type == UniformType::Boolen);
    _data = new bool[_typeInfo->arrayCnt];
    memset(_data, 0, sizeof(bool) * _typeInfo->arrayCnt);
}

MaterialPropertyBoolArray::~MaterialPropertyBoolArray()
{
    if (_data != nullptr)
    {
        delete _data;
        _data = nullptr;
    }
}

void MaterialPropertyBoolArray::Set(const bool* elements, size_t numElements, size_t offset)
{
    for (int i = offset, j=0; i < _typeInfo->arrayCnt, j < numElements; i++, j++)
    {
        _data[i] = elements[j];
    }
}

void MaterialPropertyBoolArray::Update(VkDevice device, VkDescriptorSet set) const
{
    _ubo->SetData((uint8_t*)_data, _typeInfo->size, _typeInfo->offset);
}


MaterialPropertyUserBlock::MaterialPropertyUserBlock(const SetBindingInfo *uboInfo, Buffer *ubo, const UniformInfo *uniform)
: MaterialProperty(uboInfo, uniform->name, MaterialProperty::Type::UserBlock)
, _typeInfo(uniform)
, _ubo(ubo)
{
    assert(uboInfo->descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
    assert(_typeInfo->type == UniformType::UserBlock);
    _block.resize(uniform->paddedSize);

}


void MaterialPropertyUserBlock::Set(const uint8_t* data, size_t dataSz, size_t offset)
{
    for (size_t i = offset, j=0; i < _block.size(), j < dataSz; i++, j++)
    {
        _block[i] = data[j];
    }
}

void MaterialPropertyUserBlock::Update(VkDevice device, VkDescriptorSet set) const
{
    _ubo->SetData(_block.data(), _typeInfo->size, _typeInfo->offset);
}


template<typename T>
MaterialPropertyImage<T>::MaterialPropertyImage(const SetBindingInfo* sbi)
: MaterialProperty(sbi, sbi->name, _image_type_to_property_type(sbi->imageInfo->type, false))
{
    assert(sbi->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER);
    //assert(sbi->imageInfo->layerArrayed == false);
}

template<typename T>
void MaterialPropertyImage<T>::Update(VkDevice device, VkDescriptorSet set) const
{
    VkDescriptorImageInfo texInfo{};
    texInfo.imageLayout = _tex->GetLayout();
    texInfo.imageView = _tex->GetView();
    texInfo.sampler = _tex->GetSampler();

    VkWriteDescriptorSet texWriteSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    texWriteSet.dstSet = set;
    texWriteSet.dstBinding = setBindingInfo->binding;
    texWriteSet.descriptorType = setBindingInfo->descriptorType;
    texWriteSet.descriptorCount = 1;
    texWriteSet.dstArrayElement = 0;
    texWriteSet.pImageInfo = &texInfo;

    vkUpdateDescriptorSets(device, 1, &texWriteSet, 0, nullptr);

}



template class MaterialPropertyUniform<float>;
template class MaterialPropertyUniform<int>;
template class MaterialPropertyUniform<bool>;
template class MaterialPropertyUniform<glm::vec4>;
template class MaterialPropertyUniform<glm::mat4>;
template class MaterialPropertyUniformArray<float>;
template class MaterialPropertyUniformArray<int>;
//template class MaterialPropertyUniformArray<bool>;
template class MaterialPropertyUniformArray<glm::vec4>;
template class MaterialPropertyUniformArray<glm::mat4>;


template class MaterialPropertyImage<Texture2D>;
template class MaterialPropertyImage<TextureCube>;