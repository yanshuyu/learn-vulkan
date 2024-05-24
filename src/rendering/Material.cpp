#include"Material.h"
#include<core\ShaderProgram.h>
#include<rendering\DescriptorSetManager.h>
#include<core\Device.h>


std::unordered_set<Material*> Material::s_DirtyMaterials{};

Material::Material(ShaderProgram* shader)
{
    SetShaderProgram(shader);
}

Material::~Material()
{
    _reset();
}


void Material::SetShaderProgram(ShaderProgram* program)
{
    if (program == _shaderProgram)
        return;

    _reset();

    if (program == nullptr)
        return;
    
    _shaderProgram = program;

    if (program->HasSet(SetIndices::PerMaterial))
    {
        auto perMaterialBindings = program->GetSetBinding(SetIndices::PerMaterial);
        for (size_t i = 0; i < perMaterialBindings->size(); i++)
        {
             _instantiate_set_binding_properties(perMaterialBindings->data() + i);
        }

        // alloc descriptor set
        _vkSet = DescriptorSetManager::AllocDescriptorSet(PerMaterial, DescriptorSetManager::DefaultProgramSetHash(_shaderProgram));

        // binding uniform buffers
        if (_buffers.size() > 0)
        {
            std::vector<VkDescriptorBufferInfo> _bufferDatas(_buffers.size());
            std::vector<VkWriteDescriptorSet> _bufferWriteSets(_buffers.size());
            size_t idx = 0;
            for (auto &&buffer : _buffers)
            {
                _bufferWriteSets[idx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                _bufferWriteSets[idx].dstSet = _vkSet;
                _bufferWriteSets[idx].dstBinding = buffer.first;
                _bufferWriteSets[idx].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                _bufferWriteSets[idx].descriptorCount = 1;
                _bufferWriteSets[idx].dstArrayElement = 0;
                _bufferWriteSets[idx].pBufferInfo = &_bufferDatas[idx];
                _bufferDatas[idx].buffer = buffer.second->GetHandle();
                _bufferDatas[idx].offset = 0;
                _bufferDatas[idx].range = buffer.second->GetMemorySize();
            }
            vkUpdateDescriptorSets(_shaderProgram->GetDevice()->GetHandle(), _bufferWriteSets.size(), _bufferWriteSets.data(), 0, nullptr);
        }
    }
}

void Material::_reset()
{
    if (!IsValid())
        return;
    
    if (_dirtyProperties.size() > 0)
    {
        _dirtyProperties.clear();
        s_DirtyMaterials.erase(this);
    }

    for (auto &&prop : _properties)
    {
        _destroy_property(prop.second);
    }
    _properties.clear();

    if (VKHANDLE_IS_NOT_NULL(_vkSet))
    {
        DescriptorSetManager::FreeDescriptorSet(PerMaterial, DescriptorSetManager::DefaultProgramSetHash(_shaderProgram), _vkSet);
        VKHANDLE_SET_NULL(_vkSet);
    }

    for (auto &&buffer : _buffers)
    {
        _shaderProgram->GetDevice()->DestroyBuffer(buffer.second);
    }
    _buffers.clear();

    _shaderProgram = nullptr;
}

void Material::Bind(VkCommandBuffer cmd, VkPipelineBindPoint bindPoint) const
{
    VkDescriptorSet dstSet = (!IsValid() || !HasProperties() || VKHANDLE_IS_NULL(_vkSet)) ?
                                             DescriptorSetManager::GetDummyDescriptorSet() : _vkSet;
    vkCmdBindDescriptorSets(cmd, bindPoint, GetPipelineLayout(), PerMaterial, 1, &dstSet, 0, nullptr);
}

void Material::_mark_property_dirty(MaterialProperty* p)
{
    if (_dirtyProperties.find(p) == _dirtyProperties.end())
    {
        _dirtyProperties.insert(p);
        if (s_DirtyMaterials.find(this) == s_DirtyMaterials.end())
            s_DirtyMaterials.insert(this);
    }
}

MaterialProperty *Material::_get_property(const char *name, MaterialProperty::Type t) const
{
    auto itr = _properties.find(MaterialProperty::NameToHashId(name));
    if (itr == _properties.end() || itr->second->type != t)
        return nullptr;
    
    return itr->second;
}

void Material::_instantiate_set_binding_properties(const SetBindingInfo* setBinding)
{
    Buffer *ubo{nullptr};
    MaterialProperty* imgProp{nullptr};
    switch (setBinding->descriptorType)
    {
    case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        ubo = _shaderProgram->GetDevice()->CreateBuffer(setBinding->blockInfo->paddedSize,
                                                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                                                setBinding->name ? setBinding->name : setBinding->structureName);
        _buffers.insert(std::make_pair((size_t)setBinding->binding, ubo));
        for (size_t i = 0; i < setBinding->blockInfo->uniforms.size(); i++)
        {
            const UniformInfo* uniform = &setBinding->blockInfo->uniforms[i];
            MaterialProperty* uniformProp = _instantitate_uniform_property(setBinding, uniform, ubo);
            _properties.insert(std::make_pair(uniformProp->nameHashCode, uniformProp));
        }
        break;
    
    case VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        imgProp = _instantitate_image_property(setBinding);
         _properties.insert(std::make_pair(imgProp->nameHashCode, imgProp));
        break;

    default:
        LOGE("Material property({}) is not support!", setBinding->name ? setBinding->name : setBinding->structureName);
        break;
    }
}

MaterialProperty* Material::_instantitate_uniform_property(const SetBindingInfo* binding, const UniformInfo* uniform, Buffer* buffer)
{
    MaterialProperty* p{nullptr};
    switch (uniform->type)
    {
    case UniformType::Boolen:
        return  uniform->arrayCnt > 1 ? (MaterialProperty*)new MaterialPropertyBoolArray(binding, buffer, uniform) : new MaterialPropertyBool(binding, buffer, uniform);
    
    case UniformType::Int:
        return uniform->arrayCnt > 1 ? (MaterialProperty*)new MaterialPropertyIntArray(binding, buffer, uniform) : new MaterialPropertyInt(binding, buffer, uniform);
    
    case UniformType::Float:
        return uniform->arrayCnt > 1 ? (MaterialProperty*)new MaterialPropertyFloatArray(binding, buffer, uniform) : new MaterialPropertyFloat(binding, buffer, uniform);
    
    case UniformType::Vector:
        return uniform->arrayCnt > 1 ? (MaterialProperty*)new MaterialPropertyVectorArray(binding, buffer, uniform) : new MaterialPropertyVector(binding, buffer, uniform);
    
    case UniformType::Matrix:
        return uniform->arrayCnt > 1 ? (MaterialProperty*)new MaterialPropertyMatrixArray(binding, buffer, uniform) : new MaterialPropertyMatrix(binding, buffer, uniform);

    case UniformType::UserBlock:
        assert(uniform->arrayCnt == 1);
        return new MaterialPropertyUserBlock(binding, buffer, uniform);
    default:
        break;
    }

    return nullptr;   
}

 MaterialProperty* Material::_instantitate_image_property(const SetBindingInfo* binding)
 {
    switch (binding->imageInfo->type)
    {
    case ImageType::Tex2D:
        assert(binding->imageInfo->layerArrayed == false);
        return (MaterialProperty*)new MaterialPropertyTex2D(binding);
    case ImageType::TexCube:
         assert(binding->imageInfo->layerArrayed);
         return (MaterialProperty*)new MaterialPropertyTexCube(binding);
    default:
        break;
    }

    LOGE("Material property({}) unsupport image type({})!", binding->name, binding->imageInfo->type);
    return nullptr;
 }

 void Material::_destroy_property(MaterialProperty* prop)
 {
    if (prop)
        delete prop;
 }

void Material::SetCustomBlock(const char* name, const uint8_t* data, size_t dataSz, size_t offset)
{
    MaterialProperty* p = _get_property(name, MaterialProperty::Type::UserBlock);
    if (p == nullptr)
        return;
    reinterpret_cast<MaterialPropertyUserBlock*>(p)->Set(data, dataSz, offset);
    _mark_property_dirty(p);
}



  void Material::_update()
  { 
    if (_dirtyProperties.size() > 0)
    {
        for (auto &&p : _dirtyProperties)
        {
            p->Update(_shaderProgram->GetDevice()->GetHandle(), _vkSet);
        }
        _dirtyProperties.clear();
    }
  }


 void Material::Update()
 {
    if (s_DirtyMaterials.size() > 0)
    {
        for (auto &&m : s_DirtyMaterials)
        {
            m->_update();
        }
        s_DirtyMaterials.clear();
    }
 }