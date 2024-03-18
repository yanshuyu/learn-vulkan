#include"core\GraphicPipeline.h"
#include<algorithm>
#include<functional>
#include<iostream>
#include"core\Device.h"
#include"core\ShaderProgram.h"
#include"core\RenderPass.h"
#include"rendering\Mesh.h"

GraphicPipeline::GraphicPipeline(Device* pDevice, ShaderProgram* program, Mesh* mesh, RenderPass* renderPass, uint32_t subPass)
: m_pDevice(pDevice)
, m_AssociateProgram(program)
, m_AssociateMesh(mesh)
, m_AssociateRenderPass(renderPass)
, m_AssociateSubpass(subPass)
{
    assert(pDevice && pDevice->IsValid());
    assert(program && program->IsValid());
    assert(mesh && mesh->IsValid());
    assert(renderPass);
    ResetDefaultStates();
}

GraphicPipeline::~GraphicPipeline()
{
    Release();
    m_pDevice = nullptr;
    m_AssociateProgram = nullptr;
    m_AssociateMesh = nullptr;
    m_AssociateRenderPass = VK_NULL_HANDLE;
    m_AssociateSubpass = 0;
}

/*
void GraphicPipeline::VISetBinding(uint32_t location, uint32_t stride, VkVertexInputRate rate)
{
    auto pos = std::find_if(m_VIBindingDesc.begin(), m_VIBindingDesc.end(), [=](const VkVertexInputBindingDescription &bindingDesc)
                            { return bindingDesc.binding == location; });
                            
    if (pos == m_VIBindingDesc.end())
    {
        m_VIBindingDesc.emplace_back();
        pos = m_VIBindingDesc.end();
        pos--;
    }

    pos->binding = location;
    pos->stride = stride;
    pos->inputRate = rate;
}


void GraphicPipeline::VISetAttribute(uint32_t location, uint32_t binding, VkFormat fmt, uint32_t offset)
{
    auto pos = std::find_if(m_VIAttrsDesc.begin(), m_VIAttrsDesc.end(), [=](const VkVertexInputAttributeDescription &attrDesc)
                            { return attrDesc.location == location; });
    if (pos == m_VIAttrsDesc.end())
    {
        m_VIAttrsDesc.emplace_back();
        pos = m_VIAttrsDesc.end();
        pos--;
    }

    pos->location = location;
    pos->binding = binding;
    pos->format = fmt;
    pos->offset = offset;
}


void GraphicPipeline::IASetTopology(VkPrimitiveTopology pt, bool ptRestarted)
{
    m_IADesc.topology = pt;
    m_IADesc.primitiveRestartEnable = ptRestarted;
}
*/

void GraphicPipeline::VSSetViewportScissorRect(VkRect2D viewportRect, VkRect2D scissorRect)
{
    m_Viewport.x = static_cast<float>(viewportRect.offset.x);
    m_Viewport.y = static_cast<float>(viewportRect.offset.y);
    m_Viewport.width = static_cast<float>(viewportRect.extent.width);
    m_Viewport.height = static_cast<float>(viewportRect.extent.height);
    m_Scissor = scissorRect;
}


 void GraphicPipeline::RSSetDepthBias(float constFactor, float slotFactor, float clamp)
 {
    m_RSCreateInfo.depthBiasConstantFactor = constFactor;
    m_RSCreateInfo.depthBiasSlopeFactor = slotFactor;;
    m_RSCreateInfo.depthBiasClamp = clamp;
 }

 void GraphicPipeline::DSSetStencilOp(uint32_t refVal, uint32_t readMask,
                                      uint32_t writeMask,
                                      VkCompareOp cmp,
                                      VkStencilOp passOp,
                                      VkStencilOp depthFailOp,
                                      VkStencilOp failOp)
 {
    VkStencilOpState stencilState{};
    stencilState.reference = refVal;
    stencilState.compareMask = readMask;
    stencilState.writeMask = writeMask;
    stencilState.compareOp = cmp;
    stencilState.passOp = passOp;
    stencilState.depthFailOp = depthFailOp;
    stencilState.failOp = failOp;
    m_DSCreateInfo.front = m_DSCreateInfo.back = stencilState;

 }

 void GraphicPipeline::ResetDefaultStates()
 {
    m_DynamicStates.clear();

    //m_IADesc = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};

    m_Viewport.x = m_Viewport.y = 0;
    m_Viewport.width = m_Viewport.height = 00;
    m_Viewport.minDepth = 0;
    m_Viewport.maxDepth = 1;
    m_Scissor = {};

    m_MSDesc = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};

    m_RSCreateInfo = GetDefaultRasterizationState();
    
    m_DSCreateInfo = GetDefaultDepthStencilState();

    m_FBAttchmentStates.clear();

 }



void GraphicPipeline::FBEnableBlend(int attachmentIdx)
{
    if (attachmentIdx >= m_FBAttchmentStates.size())
        m_FBAttchmentStates.resize(attachmentIdx + 1, GetDefaultBlendState());
    
    m_FBAttchmentStates[attachmentIdx].blendEnable = VK_TRUE;
}

void GraphicPipeline::FBDisableBlend(int attachmentIdx)
{
    if (attachmentIdx >= m_FBAttchmentStates.size())
         m_FBAttchmentStates.resize(attachmentIdx + 1, GetDefaultBlendState());

     m_FBAttchmentStates[attachmentIdx].blendEnable = VK_FALSE;
}

bool GraphicPipeline::FBGetBlendEnabled(int attachmentIdx) const
{
    if (attachmentIdx >= m_FBAttchmentStates.size())
        return false;
    
    return m_FBAttchmentStates[attachmentIdx].blendEnable;
}


void GraphicPipeline::FBSetColorBlendOp(int attachmentIdx, VkBlendFactor srcFactor, VkBlendFactor dstFactor, VkBlendOp op,  VkColorComponentFlags colorWriteMask)
{
    if (attachmentIdx >= m_FBAttchmentStates.size())
        m_FBAttchmentStates.resize(attachmentIdx+1, GetDefaultBlendState());

    m_FBAttchmentStates[attachmentIdx].blendEnable = VK_TRUE;
    m_FBAttchmentStates[attachmentIdx].srcColorBlendFactor = srcFactor;
    m_FBAttchmentStates[attachmentIdx].dstColorBlendFactor = dstFactor;
    m_FBAttchmentStates[attachmentIdx].colorBlendOp = op;
    m_FBAttchmentStates[attachmentIdx].colorWriteMask = colorWriteMask;
}

bool GraphicPipeline::FBGetColorBlendOp(int attachmentIdx, VkBlendFactor& srcFactor, VkBlendFactor& dstFactor, VkBlendOp& op, VkColorComponentFlags& colorWriteMask) const
{
    if (attachmentIdx >= m_FBAttchmentStates.size())
        return false;

    srcFactor = m_FBAttchmentStates[attachmentIdx].srcColorBlendFactor;
    dstFactor = m_FBAttchmentStates[attachmentIdx].dstColorBlendFactor;
    op = m_FBAttchmentStates[attachmentIdx].colorBlendOp;
    colorWriteMask = m_FBAttchmentStates[attachmentIdx].colorWriteMask;

    return true;
}

void GraphicPipeline::FBSetAlphaBlendOp(int attachmentIdx, VkBlendFactor srcFactor, VkBlendFactor dstFactor, VkBlendOp op)
{
    if (attachmentIdx >= m_FBAttchmentStates.size())
        m_FBAttchmentStates.resize(attachmentIdx+1, GetDefaultBlendState());

    m_FBAttchmentStates[attachmentIdx].srcAlphaBlendFactor = srcFactor;
    m_FBAttchmentStates[attachmentIdx].dstAlphaBlendFactor = dstFactor;
    m_FBAttchmentStates[attachmentIdx].alphaBlendOp = op;

}

bool GraphicPipeline::FBGetAlphaBlendOp(int attachmentIdx, VkBlendFactor& srcFactor, VkBlendFactor& dstFactor, VkBlendOp& op) const
{
    if (attachmentIdx >= m_FBAttchmentStates.size())
        return false;;
    
    srcFactor = m_FBAttchmentStates[attachmentIdx].srcAlphaBlendFactor;
    dstFactor = m_FBAttchmentStates[attachmentIdx].dstAlphaBlendFactor;
    op = m_FBAttchmentStates[attachmentIdx].alphaBlendOp;;
    
    return true;
}


void GraphicPipeline::EnableDynamicState(VkDynamicState state)
{
    if (std::find(m_DynamicStates.begin(), m_DynamicStates.end(), state) == m_DynamicStates.end())
        m_DynamicStates.push_back(state);
}

void GraphicPipeline::DisableDynamicState(VkDynamicState state)
{
    auto pos = std::find(m_DynamicStates.begin(), m_DynamicStates.end(), state);
    if (pos == m_DynamicStates.end())
        return;;
    
    m_DynamicStates.erase(pos);
}

bool GraphicPipeline::IsDynamicState(VkDynamicState state) const
{
    return std::find(m_DynamicStates.begin(), m_DynamicStates.end(), state) != m_DynamicStates.end();
}


void GraphicPipeline::MSSetSampleCount(int sampleCnt)
{
    switch (sampleCnt)
    {
    case 1:
        m_MSDesc.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        break;
    case 4:
        m_MSDesc.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
    case 8:
        m_MSDesc.rasterizationSamples = VK_SAMPLE_COUNT_8_BIT;
    case 16:
        m_MSDesc.rasterizationSamples = VK_SAMPLE_COUNT_16_BIT;
    default:
        break;
    }
}

int GraphicPipeline::MSGetSampleCount() const
{
    switch (m_MSDesc.rasterizationSamples)
    {
    case VK_SAMPLE_COUNT_4_BIT:
        return 4;
    case VK_SAMPLE_COUNT_8_BIT:
        return 8;    
    case VK_SAMPLE_COUNT_16_BIT:
        return 16;
    
    default:
        return 1;
    }
}



bool GraphicPipeline::Apply()
{
    if (IsValid())
        return true;

    // vertex input
    // mesh attributes
    std::vector<VkVertexInputBindingDescription> viBindingDesc{};
    viBindingDesc.reserve(m_AssociateMesh->GetAttributeCount());
    for (size_t i = 0; i < MaxAttribute; i++)
    {
        Attribute attr = (Attribute)i;
        if (m_AssociateMesh->HasAttribute(attr))
        {
            VkVertexInputBindingDescription bindingDesc{};
            bindingDesc.binding = m_AssociateMesh->GetAttributeBinding(attr);
            bindingDesc.stride = m_AssociateMesh->GetAttributeStride(attr);
            bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
            viBindingDesc.push_back(bindingDesc);
        }
    }
    // shader access attributes
    std::vector<VkVertexInputAttributeDescription> viAttrsDesc{};
    viAttrsDesc.reserve(m_AssociateProgram->GetAttributeCount());
    for (auto &&attrInfo : m_AssociateProgram->GetAttributes())
    {
        if (!m_AssociateMesh->HasAttribute(attrInfo.attrType))
        {
            LOGW("Shader Program({}) access attribute({}), is not exsit in mesh({})", m_AssociateProgram->GetName(), attrInfo.attrType, m_AssociateMesh->GetName());
            continue;
        }

        VkVertexInputAttributeDescription attrDesc{};
        attrDesc.binding = m_AssociateMesh->GetAttributeBinding(attrInfo.attrType);
        attrDesc.format = m_AssociateMesh->GetAttributeFormat(attrInfo.attrType);
        attrDesc.location = attrInfo.location;
        attrDesc.offset = 0;
        viAttrsDesc.push_back(attrDesc);
    }
    
    
    VkPipelineVertexInputStateCreateInfo viInfo{};
    viInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    viInfo.flags = 0;
    viInfo.pNext = 0;
    viInfo.vertexBindingDescriptionCount = viBindingDesc.size();
    viInfo.pVertexBindingDescriptions = viBindingDesc.data();
    viInfo.vertexAttributeDescriptionCount = viAttrsDesc.size();
    viInfo.pVertexAttributeDescriptions = viAttrsDesc.data();

    // input assembler
    VkPipelineInputAssemblyStateCreateInfo iaInfo{};
    iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaInfo.topology = m_AssociateMesh->GetTopology();
    iaInfo.primitiveRestartEnable = false;

    // view port & senssior
    VkPipelineViewportStateCreateInfo viewportScissorInfo{};
    viewportScissorInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportScissorInfo.flags = 0;
    viewportScissorInfo.pNext = nullptr;
    viewportScissorInfo.viewportCount = 1;
    viewportScissorInfo.pViewports = &m_Viewport;
    viewportScissorInfo.scissorCount = 1;
    viewportScissorInfo.pScissors = &m_Scissor;

    // multi sample
    if(m_MSDesc.rasterizationSamples == 0)
        m_MSDesc.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    
    // rasterization
    // ...

    // depth & stencil
    //...

    // blending
    assert(m_AssociateRenderPass->GetSubPassOutputColorAttachmentCount(m_AssociateSubpass) == m_FBAttchmentStates.size());
    VkPipelineColorBlendStateCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    fbInfo.flags = 0;
    fbInfo.pNext = nullptr;
    fbInfo.logicOpEnable = false;
    fbInfo.attachmentCount = m_FBAttchmentStates.size();
    fbInfo.pAttachments = m_FBAttchmentStates.data();

    VkPipelineDynamicStateCreateInfo dyStateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dyStateInfo.flags = 0;
    dyStateInfo.pNext = nullptr;
    dyStateInfo.dynamicStateCount = m_DynamicStates.size();
    dyStateInfo.pDynamicStates = m_DynamicStates.data();

    VkGraphicsPipelineCreateInfo createInfo{};
    // fix function stages
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;;
    createInfo.pDynamicState = &dyStateInfo;
    createInfo.pVertexInputState = &viInfo;;
    createInfo.pInputAssemblyState = &iaInfo;
    createInfo.pViewportState = &viewportScissorInfo;
    createInfo.pMultisampleState = &m_MSDesc;
    createInfo.pRasterizationState = &m_RSCreateInfo;
    createInfo.pDepthStencilState = &m_DSCreateInfo;
    createInfo.pColorBlendState = &fbInfo;

    
    // programmable function stages
    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos(m_AssociateProgram->GetShaderStageCount());
    for (size_t i = 0; i < m_AssociateProgram->GetShaderStageCount(); i++)
    {
        const ShaderStageInfo& stageInfo = m_AssociateProgram->GetShaderStageInfo(i);
        shaderStageCreateInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfos[i].stage = stageInfo.stage;
        shaderStageCreateInfos[i].module = stageInfo.shaderMoudle;
        shaderStageCreateInfos[i].pName = stageInfo.pEntryName.c_str();
    }
    createInfo.stageCount = m_AssociateProgram->GetShaderStageCount();
    createInfo.pStages = shaderStageCreateInfos.data();
    createInfo.pTessellationState = nullptr;
    // pipeline access shader resource
    createInfo.layout = m_AssociateProgram->GetPipelineLayout();
    // compatible render pass
    createInfo.renderPass = m_AssociateRenderPass->GetHandle();
    createInfo.subpass = m_AssociateSubpass;
    //other
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    

    VkPipeline createdPipeline = VK_NULL_HANDLE;
    VkResult result = vkCreateGraphicsPipelines(m_pDevice->GetHandle(), nullptr, 1, &createInfo, nullptr, &createdPipeline);
    if (result != VK_SUCCESS)
    {
        std::cout << "-->Create Graphics Pipeline Failed, Error: " << result << std::endl;
        return false;
    }
    m_Pipeline = createdPipeline;

    // when pipeline is created shader moudles are not need any more (maybe others pipeline still need same program)
    //program->ReleaseShaderMoudles();

    return true;
}

VkPipelineLayout GraphicPipeline::GetLayoutHandle() const
{
    return IsValid() ? m_AssociateProgram->GetPipelineLayout() : VK_NULL_HANDLE;
}

void GraphicPipeline::Release()
{
    if (IsValid())
    {
        vkDestroyPipeline(m_pDevice->GetHandle(), m_Pipeline, nullptr);
        ResetDefaultStates();
        m_Pipeline = VK_NULL_HANDLE;
    }
}

