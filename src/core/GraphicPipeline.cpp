#include"core\GraphicPipeline.h"
#include<algorithm>
#include<functional>
#include<iostream>
#include"core\Device.h"
#include"core\ShaderProgram.h"
#include"core\RenderPass.h"
#include"rendering\Mesh.h"

GraphicPipeline::GraphicPipeline(const Device* pDevice, const PipelineState& pipelineState)
: m_pDevice(pDevice)
, m_PipelineState(pipelineState)
{   
    if(!(pDevice && pDevice->IsValid()))
        throw std::invalid_argument("Create GraphicPipeline with invalid device!");
    
    if (VKCALL_FAILED(_create()))
        throw std::runtime_error("Create GraphicPipeline vulkan error!");
}

GraphicPipeline::GraphicPipeline(const Device* pDevice, PipelineState&& pipelineState)
: m_pDevice(pDevice)
, m_PipelineState(pipelineState)
{   
    if(!(pDevice && pDevice->IsValid()))
        throw std::invalid_argument("Create GraphicPipeline with invalid device!");
    
    if (VKCALL_FAILED(_create()))
        throw std::runtime_error("Create GraphicPipeline vulkan error!");
}

GraphicPipeline::~GraphicPipeline()
{
    Release();
}



VkResult GraphicPipeline::_create()
{
    // vertex input
    VkPipelineVertexInputStateCreateInfo viStateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    viStateInfo.vertexBindingDescriptionCount = m_PipelineState.viState.bindingDescs.size();
    viStateInfo.pVertexBindingDescriptions = m_PipelineState.viState.bindingDescs.data();
    viStateInfo.vertexAttributeDescriptionCount = m_PipelineState.viState.attrsDescs.size();
    viStateInfo.pVertexAttributeDescriptions = m_PipelineState.viState.attrsDescs.data();

    // input assembler
    VkPipelineInputAssemblyStateCreateInfo iaStateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    iaStateInfo.topology = m_PipelineState.iaState.topology;
    iaStateInfo.primitiveRestartEnable = m_PipelineState.iaState.restartEnable;

    // view port & senssior
    VkPipelineViewportStateCreateInfo vpStateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vpStateInfo.viewportCount = 1;
    vpStateInfo.pViewports = &m_PipelineState.vsState.viewport;
    vpStateInfo.scissorCount = 1;
    vpStateInfo.pScissors = &m_PipelineState.vsState.scissor;

    // multi sample
    VkPipelineMultisampleStateCreateInfo msStateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    msStateInfo.rasterizationSamples = m_PipelineState.msaaState.rasterizationSamples;
    msStateInfo.alphaToCoverageEnable = m_PipelineState.msaaState.alphaToCoverageEnable;
    msStateInfo.alphaToOneEnable = m_PipelineState.msaaState.alphaToOneEnable;
    
    // rasterization
    VkPipelineRasterizationStateCreateInfo raStateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raStateInfo.cullMode = m_PipelineState.raState.cullMode;
    raStateInfo.frontFace = m_PipelineState.raState.frontFace;
    raStateInfo.polygonMode = m_PipelineState.raState.polygonMode;
    raStateInfo.lineWidth = m_PipelineState.raState.lineWidth;
    raStateInfo.depthBiasEnable = m_PipelineState.raState.depthBiasEnable;
    raStateInfo.depthBiasConstantFactor = m_PipelineState.raState.depthBiasConstantFactor;
    raStateInfo.depthBiasSlopeFactor = m_PipelineState.raState.depthBiasSlopeFactor;
    raStateInfo.depthBiasClamp = m_PipelineState.raState.depthBiasClamp;

    // depth & stencil
    VkPipelineDepthStencilStateCreateInfo dsStateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    dsStateInfo.depthTestEnable = m_PipelineState.dsState.depthTestEnable;
    dsStateInfo.depthCompareOp = m_PipelineState.dsState.depthCompareOp;
    dsStateInfo.depthWriteEnable = m_PipelineState.dsState.depthWriteEnable;
    dsStateInfo.stencilTestEnable = m_PipelineState.dsState.stencilTestEnable;
    dsStateInfo.front = m_PipelineState.dsState.front;
    dsStateInfo.back = m_PipelineState.dsState.back;

    // blending
    VkPipelineColorBlendStateCreateInfo fbStateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    fbStateInfo.attachmentCount = m_PipelineState.fbState.attachmentBlendStates.size();
    fbStateInfo.pAttachments = m_PipelineState.fbState.attachmentBlendStates.data();

    // dynamic states
    VkPipelineDynamicStateCreateInfo dyStateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dyStateInfo.dynamicStateCount = m_PipelineState.dynamicState.size();
    dyStateInfo.pDynamicStates = m_PipelineState.dynamicState.data();

    // fix function stages
    VkGraphicsPipelineCreateInfo pipeCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    pipeCreateInfo.pDynamicState = &dyStateInfo;
    pipeCreateInfo.pVertexInputState = &viStateInfo;;
    pipeCreateInfo.pInputAssemblyState = &iaStateInfo;
    pipeCreateInfo.pViewportState = &vpStateInfo;
    pipeCreateInfo.pRasterizationState = &raStateInfo;
    pipeCreateInfo.pMultisampleState = &msStateInfo;
    pipeCreateInfo.pDepthStencilState = &dsStateInfo;
    pipeCreateInfo.pColorBlendState = &fbStateInfo;
    
    // programmable function stages
    pipeCreateInfo.stageCount = m_PipelineState.shaderState.stageInfo.size();
    pipeCreateInfo.pStages = m_PipelineState.shaderState.stageInfo.data();
    pipeCreateInfo.pTessellationState = nullptr;

    // pipeline access shader resource
    pipeCreateInfo.layout = m_PipelineState.shaderState.resourceLayout;
    // compatible render pass
    pipeCreateInfo.renderPass = m_PipelineState.renderPass;
    pipeCreateInfo.subpass = m_PipelineState.subpassIndex;
    //other
    pipeCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
    pipeCreateInfo.basePipelineIndex = 0;
    pipeCreateInfo.flags = 0;
    pipeCreateInfo.pNext = nullptr;

    VkPipeline createdPipeline = VK_NULL_HANDLE;
    VkResult result = vkCreateGraphicsPipelines(m_pDevice->GetHandle(), nullptr, 1, &pipeCreateInfo, nullptr, &createdPipeline);
    if (result == VK_SUCCESS)
        m_Pipeline = createdPipeline;
    
    return result;
}


void GraphicPipeline::Release()
{
    if (IsValid())
    {
        vkDestroyPipeline(m_pDevice->GetHandle(), m_Pipeline, nullptr);
        m_Pipeline = VK_NULL_HANDLE;
    }
}

