#include"PipelineManager.h"
#include<core\RenderPass.h>
#include<rendering\Mesh.h>
#include<rendering\Material.h>


std::unordered_map<size_t, std::unique_ptr<GraphicPipeline>> PipelineManager::s_GraphicPipelines{};
std::unordered_map<size_t, size_t> PipelineManager::s_RefrenceCounter{};
std::unordered_map<GraphicPipeline*, size_t> PipelineManager::s_PipelineHashes{};
Device* PipelineManager::s_pDevice{};

void PipelineManager::Initailize(Device* pdevice)
{
    if (s_pDevice == nullptr)
        s_pDevice = pdevice;
}

void PipelineManager::DeInitailize()
{
    if (s_GraphicPipelines.size() > 0)
    {
        LOGW("-->PipelineManager Memory leaks:\n")
        for (auto itr = s_GraphicPipelines.begin(); itr != s_GraphicPipelines.end(); itr++)
        {
            if (itr->second->IsValid())
                LOGW("Pipeline({}) hash({}) refcount({})", (void*)itr->second.get(), itr->first, s_RefrenceCounter[itr->first]);
        }
    }

    s_GraphicPipelines.clear();
    s_PipelineHashes.clear();
    s_RefrenceCounter.clear();
    s_pDevice = nullptr;
}

PipelineState PipelineManager::MakePipelineState(const Mesh* mesh, const Material* material, const RenderPass* renderPass, size_t subPassIdx)
{
    const ShaderProgram* shader = material->GetShaderProgram();
    PipelineState ps{};
    // vertex input
    ps.viState.bindingDescs.resize(shader->GetAttributeCount());
    ps.viState.attrsDescs.resize(shader->GetAttributeCount());
    size_t idx = 0;
    for (auto &&attr : shader->GetAttributes())
    {
        ps.viState.bindingDescs[idx].binding = mesh->GetAttributeBinding(attr.attrType);
        ps.viState.bindingDescs[idx].stride = mesh->GetAttributeStride(attr.attrType);
        ps.viState.bindingDescs[idx].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        ps.viState.attrsDescs[idx].location = attr.location;
        ps.viState.attrsDescs[idx].format = attr.format;
        ps.viState.attrsDescs[idx].offset = 0;
        ps.viState.attrsDescs[idx].binding = ps.viState.bindingDescs[idx].binding;
        idx++;
    }
    
    // input assembler
    ps.iaState.topology = mesh->GetTopology();

    //view port & scissor
    ps.dyState.states.push_back(VK_DYNAMIC_STATE_VIEWPORT);
    ps.dyState.states.push_back(VK_DYNAMIC_STATE_SCISSOR);

    // Msaa
    //ps.dynamicState.push_back(VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT);
    
    // rasterization
    ps.raState = material->_rasterizationState;
    
    //depth stencil
    ps.dsState = material->_depthStencilState;

    // blend
    ps.fbState.attachmentBlendStates.resize(renderPass->GetSubPassOutputColorAttachmentCount(subPassIdx), _make_color_blend_state(material));

    // shader stage
    idx = 0;
    ps.shaderState.resourceLayout = shader->GetPipelineLayout();
    ps.shaderState.stageInfo.resize(shader->GetShaderStageCount());
    for (auto &&stageInfo : shader->GetShaderStageInfos())
    {
        ps.shaderState.stageInfo[idx] = {VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        ps.shaderState.stageInfo[idx].stage = stageInfo->stage;
        ps.shaderState.stageInfo[idx].pName = stageInfo->pEntryName.c_str();
        ps.shaderState.stageInfo[idx].module = stageInfo->shaderMoudle;
        idx++;
    }

    ps.rpState.renderPass = renderPass->GetHandle();
    ps.rpState.subPass = subPassIdx;
        
    return std::move(ps);
}


GraphicPipeline* PipelineManager::Request(const Mesh* mesh, const Material* material, const RenderPass* renderPass, size_t subPassIdx)
{
    bool sameDevice = mesh->GetDevice() == material->GetShaderProgram()->GetDevice() &&
                     mesh->GetDevice() == renderPass->GetDevice();
    assert(sameDevice);
    PipelineState ps = MakePipelineState(mesh, material, renderPass, subPassIdx);
    size_t hash = PipelineStateHasher<PipelineState>{}(ps);
    auto pos = s_GraphicPipelines.find(hash);
    if (pos == s_GraphicPipelines.end())
    {    
        pos = s_GraphicPipelines.insert(std::make_pair(hash, std::make_unique<GraphicPipeline>(s_pDevice, std::move(ps)))).first;
        s_PipelineHashes.insert(std::make_pair(pos->second.get(), hash));
    }
    
    s_RefrenceCounter[hash]++;

    return pos->second.get();
}

void PipelineManager::Release(GraphicPipeline* pipeline)
{
    auto hash_itr = s_PipelineHashes.find(pipeline);
    if (hash_itr == s_PipelineHashes.end())
        return;
    
    s_RefrenceCounter[hash_itr->second]--;
    
    if (s_RefrenceCounter[hash_itr->second] == 0)
    {
        s_GraphicPipelines.erase(hash_itr->second);
        s_PipelineHashes.erase(hash_itr->first);
    }
}

VkPipelineColorBlendAttachmentState PipelineManager::_get_default_blend_state()
{
    VkPipelineColorBlendAttachmentState bs;
    bs.blendEnable = VK_FALSE;
    bs.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    bs.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    bs.colorBlendOp = VK_BLEND_OP_ADD;
    bs.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    bs.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    bs.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    bs.alphaBlendOp = VK_BLEND_OP_ADD;
    return bs;
}


VkPipelineColorBlendAttachmentState PipelineManager::_make_color_blend_state(const Material* material)
{
    VkPipelineColorBlendAttachmentState bs = _get_default_blend_state();
    bs.blendEnable = material->GetColorBlendMode() != BlendMode::None
                    || material->GetAlphaBlendMode() != BlendMode::None;
    switch (material->GetColorBlendMode())
    {
    case BlendMode::Alpha:
        bs.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        bs.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        bs.colorBlendOp = VK_BLEND_OP_ADD;
        break;
    case BlendMode::Additive:
        bs.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        bs.dstColorBlendFactor = VK_BLEND_FACTOR_ONE;
        bs.colorBlendOp = VK_BLEND_OP_ADD;
        break;
    case BlendMode::Multiply:
        bs.srcColorBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
        bs.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
        bs.colorBlendOp = VK_BLEND_OP_ADD;
        break;

    default:
        break;
    }

    switch (material->GetAlphaBlendMode())
    {
    case BlendMode::Alpha:
        bs.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        bs.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        bs.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    
    case BlendMode::Additive:
        bs.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        bs.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        bs.alphaBlendOp = VK_BLEND_OP_ADD;
        break;

    case BlendMode::Multiply:
        bs.srcAlphaBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
        bs.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        bs.alphaBlendOp = VK_BLEND_OP_ADD;
        break;
    
    default:
        break;
    }

    switch (material->GetColorMask())
    {
    case ColorMask::R:
        bs.colorWriteMask = VK_COLOR_COMPONENT_R_BIT;
        break;
    case ColorMask::RG:
        bs.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT;
        break;
    case ColorMask::RGB:
        bs.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT;
        break;
    case ColorMask::RGBA:
        bs.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        break;
    case ColorMask::A:
        bs.colorWriteMask = VK_COLOR_COMPONENT_A_BIT;
        break;
    default:
        break;
    }

    return bs;
}
