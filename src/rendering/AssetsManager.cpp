#include"AssetsManager.h"
#include"core\Device.h"
#include"core\ShaderProgram.h"
#include"rendering\Texture2D.h"
#include"rendering\DescriptorSetManager.h"
#include<stb\stb_image.h>
#include<spirv-reflect/spirv_reflect.h>
#include<regex>


#define PRPGRAM_KEY(vs, fs) std::string

std::unordered_map<std::string, std::unique_ptr<ShaderProgram>> AssetsManager::s_programs{};
std::unordered_map<std::string, ShaderStageInfo> AssetsManager::s_shaderModules{};
Device* AssetsManager::s_pDevice{nullptr};


void AssetsManager::Initailize(Device* pDevice)
{
    assert(pDevice && pDevice->IsValid());
    s_pDevice = pDevice;
}



void AssetsManager::DeInitailize()
 {
    for (auto &&itr : s_programs)
    {
        itr.second->Release();
    }
    s_programs.clear();

    for (auto &&itr : s_shaderModules)
    {
        vkDestroyShaderModule(s_pDevice->GetHandle(), itr.second.shaderMoudle, nullptr);
    }
    s_shaderModules.clear();
 }


static bool _is_spv_file_extendsion(const char* fileName)
{
    size_t len = std::strlen(fileName);
    while ( *(fileName+len) != '.' && len > 0)
        len--;

    return std::strcmp(fileName+len, ".spv") == 0;
    
}


ShaderProgram* AssetsManager::LoadProgram(const char* vs, const char* vsName, const char* fs, const char* fsName)
{
    std::string k_program(vs);
    k_program += ":";
    k_program += fs;
    auto pos = s_programs.find(k_program);
    if (pos != s_programs.end())
        return pos->second.get();

    auto vsShader = _is_spv_file_extendsion(vs) ?
                _load_shader_moudle_spv(vs, vsName, VK_SHADER_STAGE_VERTEX_BIT) : _load_shader_moudle_glsl(vs, vsName, VK_SHADER_STAGE_VERTEX_BIT);
    auto fsShader = _is_spv_file_extendsion(fs) ? 
                _load_shader_moudle_spv(fs, fsName, VK_SHADER_STAGE_FRAGMENT_BIT) : _load_shader_moudle_glsl(fs, fsName, VK_SHADER_STAGE_FRAGMENT_BIT);

    if (!vsShader || !fsShader)
        return nullptr;

    auto result = s_programs.emplace(std::make_pair(k_program, new ShaderProgram(s_pDevice)));
    ShaderProgram *program = result.first->second.get();
    program->SetName(k_program.c_str());
    program->AddShaderStage(vsShader);
    program->AddShaderStage(fsShader);
    assert(_parse_shader_reflection(program));
    assert(program->Apply());
    if (program->HasSet(SetIndices::PerMaterial))
    {   
        auto setBindingInfos = program->GetSetBinding(SetIndices::PerMaterial);
        std::vector<VkDescriptorSetLayoutBinding> setBindings(setBindingInfos->size());
        std::transform(setBindingInfos->begin(), setBindingInfos->end(), setBindings.begin(), [](const SetBindingInfo& sbi) {
            VkDescriptorSetLayoutBinding aBinding{};
            aBinding.binding = sbi.binding;
            aBinding.descriptorType = sbi.descriptorType;
            aBinding.descriptorCount = sbi.descriptorCount;
            aBinding.stageFlags = sbi.stageFlags;
            return aBinding;
        });
        DescriptorSetManager::RegisterSetLayout(PerMaterial, DescriptorSetManager::DefaultProgramSetHash(program), std::move(setBindings), true, 2);
    }

    return program;
}


void AssetsManager::UnloadProgram(ShaderProgram* program)
{
    auto itr = s_programs.find(program->GetName());
    if (itr != s_programs.end())
    {
        itr->second->Release();
        s_programs.erase(itr);
    }
}

const ShaderStageInfo *AssetsManager::_load_shader_moudle_spv(const char *srcFile, const char *entryName, VkShaderStageFlagBits stage)
{
    std::string fullPath(ASSETS_DIR);
    fullPath += srcFile;
    auto itr = s_shaderModules.find(fullPath);
    if (itr == s_shaderModules.end())
    {
        file_bytes spvSrc = futils_read_file_bytes(fullPath.c_str());
        VkShaderModule shaderMou{VK_NULL_HANDLE};
        VkShaderModuleCreateInfo shaderMouCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderMouCreateInfo.pCode = (uint32_t *)spvSrc.bytes;
        shaderMouCreateInfo.codeSize = spvSrc.byteCnt;
        if (VKCALL_FAILED(vkCreateShaderModule(s_pDevice->GetHandle(), &shaderMouCreateInfo, nullptr, &shaderMou)))
        {   
            futils_free_file_bytes(spvSrc); 
            return nullptr;
        }

        itr = s_shaderModules.insert(std::make_pair(fullPath, ShaderStageInfo())).first;
        itr->second.srcPath = fullPath;
        itr->second.pEntryName = entryName;
        itr->second.stage = stage;
        itr->second.shaderMoudle = shaderMou;
        itr->second.spvCodes.resize(spvSrc.byteCnt);
        memcpy((void*)itr->second.spvCodes.data(), spvSrc.bytes, spvSrc.byteCnt);
        futils_free_file_bytes(spvSrc); 
    }

    return &itr->second;
}

const ShaderStageInfo *AssetsManager::_load_shader_moudle_glsl(const char *srcFile, const char *entryName, VkShaderStageFlagBits stage)
{
    std::string fullPath(ASSETS_DIR);
    fullPath += srcFile;
    auto itr = s_shaderModules.find(fullPath);
    if (itr == s_shaderModules.end())
    {
       
        std::vector<char> spvBytes;
        if(!s_pDevice->CompileShader(fullPath.c_str(), stage, spvBytes))
            return nullptr;

        VkShaderModule shaderMou{VK_NULL_HANDLE};
        VkShaderModuleCreateInfo shaderMouCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        shaderMouCreateInfo.pCode = (uint32_t *)spvBytes.data();
        shaderMouCreateInfo.codeSize = spvBytes.size();
        if (VKCALL_FAILED(vkCreateShaderModule(s_pDevice->GetHandle(), &shaderMouCreateInfo, nullptr, &shaderMou)))
            return nullptr;
    
        itr = s_shaderModules.insert(std::make_pair(fullPath, ShaderStageInfo())).first;
        itr->second.srcPath = fullPath;
        itr->second.pEntryName = entryName;
        itr->second.spvCodes = std::move(spvBytes);
        itr->second.stage = stage;
        itr->second.shaderMoudle = shaderMou;
    }

    return &itr->second;
}

static VertexAttribute _attr_name_to_type(const char *name)
 {
    if (std::regex_search(name, std::regex(".*pos.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*vertex.*", std::regex::icase)))
        return VertexAttribute::Position;
    
    if (std::regex_search(name, std::regex(".*nor.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*normal.*", std::regex::icase)))
        return VertexAttribute::Normal;

    if (std::regex_search(name, std::regex(".*tan.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*tangent.*", std::regex::icase)))
        return VertexAttribute::Tangent;

    if (std::regex_search(name, std::regex(".*col.*", std::regex::icase)) 
        || std::regex_search(name, std::regex(".*color.*", std::regex::icase)))
        return VertexAttribute::Color;

    if (std::regex_search(name, std::regex(".*uv.*", std::regex::icase))
        || std::regex_search(name, std::regex(".*uv0.*", std::regex::icase)))
        return VertexAttribute::UV0;

    if (std::regex_search(name, std::regex(".*uv1.*", std::regex::icase)))
        return VertexAttribute::UV1;
    
    return VertexAttribute::MaxAttribute;
 }

static UniformType _spv_type_to_uniform_type(SpvReflectTypeFlags typeFlag, const SpvReflectNumericTraits& numericTraits)
{
    if (typeFlag & SPV_REFLECT_TYPE_FLAG_STRUCT)
        return UniformType::UserBlock;
    
    if (typeFlag & SPV_REFLECT_TYPE_FLAG_MATRIX)
    {
        if (numericTraits.matrix.row_count != 4 || numericTraits.matrix.column_count != 4)
        {
            LOGE("Matrix {} row {} col is not support!", numericTraits.matrix.column_count, numericTraits.matrix.column_count);
            return UniformType::UnknowUniformType;
        }
        return UniformType::Matrix;
    }

    if (typeFlag & SPV_REFLECT_TYPE_FLAG_VECTOR)
    {
        if (numericTraits.vector.component_count != 4)
        {
            LOGE("Vector {} component is not support!", numericTraits.vector.component_count);
            return UniformType::UnknowUniformType;
        }
        return UniformType::Vector;
    }

    if (typeFlag & SPV_REFLECT_TYPE_FLAG_FLOAT)
        return UniformType::Float;
    
    if (typeFlag & SPV_REFLECT_TYPE_FLAG_INT)
        return UniformType::Int;
    
    if (typeFlag & SPV_REFLECT_TYPE_FLAG_BOOL)
        return UniformType::Boolen;


    return UniformType::UnknowUniformType;
}

static ImageType _spv_imgdim_to_image_type(SpvDim dim)
{
    switch (dim)
    {
    case SpvDim1D:
        return ImageType::Tex1D;
        
    case SpvDim2D:
        return ImageType::Tex2D;
    
    case SpvDim3D:
        return ImageType::Tex3D;
    
    case SpvDimCube:
        return ImageType::TexCube;
    
    default:
        return ImageType::UnKnowImageType;
    }
}

static VertexAttributeInfo _make_vertex_attr_info(const SpvReflectInterfaceVariable& iVar)
{
    VertexAttributeInfo attr{};
    attr.attrType = _attr_name_to_type(iVar.name);
    attr.location = iVar.location;
    attr.format = static_cast<VkFormat>(iVar.format);
    strcpy_s(attr.name, MAX_SHADER_VARIABLE_NAME, iVar.name);
    return std::move(attr);
}

static bool _populate_set_binding_ubo_info(const SpvReflectBlockVariable& spvBlock, ShaderBlockInfo& sbi)
{
    sbi.size = spvBlock.size;
    sbi.paddedSize = spvBlock.padded_size;
    sbi.uniforms.reserve(spvBlock.member_count);
    for (size_t i = 0; i < spvBlock.member_count; i++)
    {
        const SpvReflectBlockVariable uboMem = spvBlock.members[i];
        UniformInfo uniform{};
        strcpy_s(uniform.name, MAX_SHADER_VARIABLE_NAME, uboMem.name);
        uniform.type = _spv_type_to_uniform_type(uboMem.type_description->type_flags, uboMem.numeric);
        uniform.size = uboMem.size;
        uniform.paddedSize = uboMem.padded_size;
        uniform.offset = uboMem.offset;
        uniform.absoluteOffset = uboMem.absolute_offset;
        uniform.arrayCnt = 1;
        for (size_t j = 0; j < uboMem.array.dims_count; j++)
        {
            uniform.arrayCnt *= uboMem.array.dims[j];
        }
        if (uniform.type == UniformType::UnknowUniformType)
            return false;
        else
            sbi.uniforms.push_back(uniform);
    }
    
    return true;
}

static bool _populate_set_binding_image_info(const SpvReflectImageTraits& imgTraits, ShaderImageInfo& sii)
{
    ImageType it = _spv_imgdim_to_image_type(imgTraits.dim);
    if (it == ImageType::UnKnowImageType)
        return false;

    sii.type = it;
    sii.layerArrayed = imgTraits.arrayed;
    sii.multiSampled = imgTraits.ms;
    sii.sampled = imgTraits.sampled;
    return true;
}

static bool _populate_set_binding_info(const SpvReflectDescriptorBinding& spvBinding, SetBindingInfo& sbi)
{
    sbi.set = spvBinding.set;
    sbi.binding = spvBinding.binding;
    sbi.descriptorType = (VkDescriptorType)spvBinding.descriptor_type;
    sbi.descriptorCount = 1;
    for (size_t i = 0; i < spvBinding.array.dims_count; i++)
    {
        sbi.descriptorCount *= spvBinding.array.dims[i];
    }
    sbi.accessed = spvBinding.accessed;
    if (spvBinding.name) strcpy_s(sbi.name, MAX_SHADER_VARIABLE_NAME, spvBinding.name);
    if (spvBinding.type_description->type_name) strcpy_s(sbi.structureName, MAX_SHADER_VARIABLE_NAME, spvBinding.type_description->type_name);
    ShaderBlockInfo blockInfo{};
    ShaderImageInfo imgInfo{};
    switch (spvBinding.descriptor_type)
    {
    case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
        if (!_populate_set_binding_ubo_info(spvBinding.block, blockInfo))    
        {   
            LOGE("Uniform Buffer({}) is not support!", spvBinding.name); 
            return false;
        }
        sbi.blockInfo = std::move(blockInfo);
        break;

    case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER:
        if (!_populate_set_binding_image_info(spvBinding.image, imgInfo))
        {
            LOGE("Texture({}) Type({}) is not support!", spvBinding.name, spvBinding.image.dim);
            return false;
        }
        sbi.imageInfo = std::move(imgInfo);
        break;

    default:
        LOGE("Descriptor type({}) is not support!", spvBinding.descriptor_type);
        return false;
        break;
    }

    return true;
}



 bool AssetsManager::_parse_shader_reflection(ShaderProgram *shader)
 {
    std::vector<VertexAttributeInfo> shaderInputAttrInfos{};
    std::map<int, std::vector<SetBindingInfo>> shaderSetBindingInfos{};
    bool success = true;
    for (auto &&shaderStageInfo : shader->GetShaderStageInfos()) // for each shader stage 
    {
        SpvReflectShaderModule refltShaderModule{};
        SpvReflectResult result = spvReflectCreateShaderModule(shaderStageInfo->spvCodes.size(), shaderStageInfo->spvCodes.data(), &refltShaderModule);
        if (result != SPV_REFLECT_RESULT_SUCCESS)
        {   
            LOGE("Failed to create reflect shader module: {}", shaderStageInfo->srcPath.c_str()); 
            success = false;
            break;
        }

        // parse vertex shader input attributes if current shader module stage is vertex
        if (refltShaderModule.shader_stage == SPV_REFLECT_SHADER_STAGE_VERTEX_BIT)
        {
            uint32_t inputVarCnt = 0;
            spvReflectEnumerateInputVariables(&refltShaderModule, &inputVarCnt, nullptr);
            std::vector<SpvReflectInterfaceVariable*> inputVarInfos(inputVarCnt);
            spvReflectEnumerateInputVariables(&refltShaderModule, &inputVarCnt, inputVarInfos.data());
            for (size_t i = 0; i < inputVarCnt; i++)
            {
                const SpvReflectInterfaceVariable& inputVar = *inputVarInfos[i];
                // ignore built in variables
                if (inputVar.decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN)
                    continue;

                shaderInputAttrInfos.emplace_back(_make_vertex_attr_info(inputVar));
            } 
        }

        // parse descriptor set & bindings
        uint32_t setCnt = 0;
        spvReflectEnumerateDescriptorSets(&refltShaderModule, &setCnt, nullptr);
        std::vector<SpvReflectDescriptorSet*> setInfos(setCnt);
        spvReflectEnumerateDescriptorSets(&refltShaderModule, &setCnt, setInfos.data());
        for (size_t i = 0; i < setCnt; i++) // for current shader stage's each descriptor set
        {
            SpvReflectDescriptorSet reflSet = *setInfos[i];
            if (shaderSetBindingInfos.find(reflSet.set) == shaderSetBindingInfos.end())
            {  
                shaderSetBindingInfos[reflSet.set] = {};
                shaderSetBindingInfos[reflSet.set].reserve(reflSet.binding_count);
            }
            std::vector<SetBindingInfo>& dstSetBindings = shaderSetBindingInfos[reflSet.set];

            for (size_t j = 0; j < reflSet.binding_count; j++) // for current shader stage's current descriptor set's each binding
            {
                const SpvReflectDescriptorBinding &reflSetBinding = *reflSet.bindings[j];
                auto itr = std::find_if(dstSetBindings.begin(), dstSetBindings.end(), [&](const SetBindingInfo &sbi)
                                        { return sbi.binding == reflSetBinding.binding; });
                if (itr == dstSetBindings.end()) 
                {
                    // binding hasn't add to set
                    SetBindingInfo sbi{};
                    if (_populate_set_binding_info(reflSetBinding, sbi)) 
                    {
                        sbi.stageFlags |= refltShaderModule.shader_stage;
                        dstSetBindings.push_back(std::move(sbi));
                    }
                    else // binding is not supported
                    {
                        success = false;
                        break;
                    }
                }
                else
                {
                    // safe check
                    assert(reflSetBinding.set == itr->set);
                    assert(reflSetBinding.binding == itr->binding);
                    assert(std::strcmp(reflSetBinding.name, itr->name) == 0);
                    assert((VkDescriptorType)reflSetBinding.descriptor_type == itr->descriptorType);

                    // binding has exist in set, access by other stage
                    itr->accessed |= reflSetBinding.accessed;
                    itr->stageFlags |= refltShaderModule.shader_stage;
                }
            }

            if (!success)
                break;
        }
        
        spvReflectDestroyShaderModule(&refltShaderModule);

        if (!success)
            break;
    }

    if (success)
    {
        for (auto &&attr : shaderInputAttrInfos)
            shader->AddAttribute(std::move(attr));
        
        for (auto &&set : shaderSetBindingInfos)
            shader->AddSetBinding(set.first, std::move(set.second));
    }

    return success;
 }





