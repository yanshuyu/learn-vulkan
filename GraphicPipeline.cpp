#include"GraphicPipeline.h"
#include<algorithm>
#include<functional>
#include<iostream>

GraphicPipeline::GraphicPipeline()
{
    m_RSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    m_DSCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ResetDefaultStates();
    m_Pipeline = VK_NULL_HANDLE;
    m_PipelineLayout = VK_NULL_HANDLE;
    m_Device = VK_NULL_HANDLE;
}

GraphicPipeline::~GraphicPipeline()
{
    Release();
}


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
    m_IATopology = pt;
    m_IAPrimitiveRestart = ptRestarted;
}


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


 void GraphicPipeline::ResetDefaultStates()
 {
    m_DynamicStates.clear();

    m_VIBindingDesc.clear();
    m_VIAttrsDesc.clear();

    m_IATopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_IAPrimitiveRestart = false;

    m_Viewport.x = m_Viewport.y = 0;
    m_Viewport.width = m_Viewport.height = 00;
    m_Viewport.minDepth = 0;
    m_Viewport.maxDepth = 1;
    m_Scissor = {};

    m_RSCreateInfo = GetDefaultRasterizationState();
    
    m_DSCreateInfo = GetDefaultDepthStencilState();

    m_FBAttchmentStates.clear();

    m_MsSampleCount = VK_SAMPLE_COUNT_1_BIT;
    m_AlphaToCoverageEnable = false;

    m_ShaderInfo.clear();

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
        return;

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


void GraphicPipeline::SetDynamicStateHint(VkDynamicState state)
{
    if (std::find(m_DynamicStates.begin(), m_DynamicStates.end(), state) == m_DynamicStates.end())
        m_DynamicStates.push_back(state);
}

void GraphicPipeline::UnsetDynamicStateHint(VkDynamicState state)
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
        m_MsSampleCount = VK_SAMPLE_COUNT_1_BIT;
        break;
    case 4:
        m_MsSampleCount = VK_SAMPLE_COUNT_4_BIT;
    case 16:
        m_MsSampleCount = VK_SAMPLE_COUNT_16_BIT;
    default:
        break;
    }
}

int GraphicPipeline::MSGetSampleCount() const
{
    switch (m_MsSampleCount)
    {
    case VK_SAMPLE_COUNT_4_BIT:
        return 4;
    case VK_SAMPLE_COUNT_16_BIT:
        return 16;
    
    default:
        return 1;
    }
}



bool GraphicPipeline::Create(VkDevice device, VkRenderPass renderPass, uint32_t subPass, bool forceCreate)
{
    if (device == VK_NULL_HANDLE)
        return false;

    if (forceCreate && IsCreate())
        Release();
    
    VkPipelineVertexInputStateCreateInfo viInfo{};
    viInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    viInfo.flags = 0;
    viInfo.pNext = 0;
    viInfo.vertexBindingDescriptionCount = m_VIBindingDesc.size();
    viInfo.pVertexBindingDescriptions = m_VIBindingDesc.data();
    viInfo.vertexAttributeDescriptionCount = m_VIAttrsDesc.size();
    viInfo.pVertexAttributeDescriptions = m_VIAttrsDesc.data();

    VkPipelineInputAssemblyStateCreateInfo iaInfo{};

    iaInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    iaInfo.flags = 0;
    iaInfo.pNext = nullptr;;
    iaInfo.topology = m_IATopology;
    iaInfo.primitiveRestartEnable = m_IAPrimitiveRestart;

    VkPipelineViewportStateCreateInfo viewportScissorInfo{};
    viewportScissorInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportScissorInfo.flags = 0;
    viewportScissorInfo.pNext = nullptr;
    viewportScissorInfo.viewportCount = 1;
    viewportScissorInfo.pViewports = &m_Viewport;
    viewportScissorInfo.scissorCount = 1;
    viewportScissorInfo.pScissors = &m_Scissor;

    VkPipelineColorBlendStateCreateInfo fbInfo{};
    fbInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    fbInfo.flags = 0;
    fbInfo.pNext = nullptr;
    fbInfo.logicOpEnable = false;
    fbInfo.attachmentCount = m_FBAttchmentStates.size();
    fbInfo.pAttachments = m_FBAttchmentStates.data();

    VkPipelineMultisampleStateCreateInfo msInfo{};
    msInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    msInfo.flags = 0;
    msInfo.pNext = nullptr;
    msInfo.alphaToCoverageEnable = m_AlphaToCoverageEnable;
    msInfo.alphaToOneEnable = false;
    msInfo.pSampleMask = nullptr;
    msInfo.sampleShadingEnable = false;
    msInfo.minSampleShading = 0;
    msInfo.rasterizationSamples = m_MsSampleCount;

    VkPipelineDynamicStateCreateInfo dyStateInfo{};
    dyStateInfo.flags = 0;
    dyStateInfo.pNext = nullptr;
    dyStateInfo.dynamicStateCount = m_DynamicStates.size();
    dyStateInfo.pDynamicStates = m_DynamicStates.data();


    vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos{};
    shaderStageCreateInfos.reserve(m_ShaderInfo.size());
    for (size_t i = 0; i < m_ShaderInfo.size(); i++)
    {   
        VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
        shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfo.pNext = nullptr;
        shaderStageCreateInfo.flags = 0;
        shaderStageCreateInfo.module = m_ShaderInfo[i].shaderMoudle;
        shaderStageCreateInfo.pName = m_ShaderInfo[i].pEntryName;
        shaderStageCreateInfo.stage = m_ShaderInfo[i].stage;
        shaderStageCreateInfo.pSpecializationInfo = nullptr;
    
        shaderStageCreateInfos.push_back(shaderStageCreateInfo);
    }


    vector<VkDescriptorSetLayout> descriptorSetLayouts{};
    vector<std::unique_ptr<VkDescriptorSetLayout, std::function<void(VkDescriptorSetLayout*)>>> descriptorSetLayoutsAutoReleaseTemp{};
    descriptorSetLayouts.reserve(m_DescriptorSetLayoutsBinding.size());
    descriptorSetLayoutsAutoReleaseTemp.reserve(m_DescriptorSetLayoutsBinding.size());
    for (size_t i = 0; i < m_DescriptorSetLayoutsBinding.size(); i++)
    {
        auto& descriptorSetBindings = m_DescriptorSetLayoutsBinding[i];
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.flags = 0;
        descriptorSetLayoutCreateInfo.pNext = nullptr;
        descriptorSetLayoutCreateInfo.bindingCount = descriptorSetBindings.size();
        descriptorSetLayoutCreateInfo.pBindings = descriptorSetBindings.data();
        VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
        VkResult result = vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &setLayout);
        if( result != VK_SUCCESS)
        {
            std::cout << "-->Create Descriptor Set Layout Failed, Error: " << result << std::endl;
            return false;
        }

        descriptorSetLayouts.push_back(setLayout);
        descriptorSetLayoutsAutoReleaseTemp.emplace_back(&descriptorSetLayouts[i], [=](VkDescriptorSetLayout* pSetLayout)
        {
            if (*pSetLayout != VK_NULL_HANDLE)
            { 
                vkDestroyDescriptorSetLayout(device, *pSetLayout, nullptr);
                *pSetLayout = VK_NULL_HANDLE;
            };
        });
    }

    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
    pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCreateInfo.flags = 0;
    pipelineLayoutCreateInfo.pNext = nullptr;
    pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
    pipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayouts.size();
    pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkResult result = vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout);
    if (result != VK_SUCCESS)
    {
         std::cout << "-->Create Graphics Pipeline Layout Failed, Error: " << result << std::endl;
         return false;
    }

    VkGraphicsPipelineCreateInfo createInfo{};
    // fix function stages
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;;
    createInfo.pDynamicState = &dyStateInfo;
    createInfo.pVertexInputState = &viInfo;;
    createInfo.pInputAssemblyState = &iaInfo;
    createInfo.pViewportState = &viewportScissorInfo;
    createInfo.pRasterizationState = &m_RSCreateInfo;
    createInfo.pDepthStencilState = &m_DSCreateInfo;
    createInfo.pColorBlendState = &fbInfo;
    createInfo.pMultisampleState = &msInfo;
    // programmable function stages
    createInfo.stageCount = shaderStageCreateInfos.size();
    createInfo.pStages = shaderStageCreateInfos.data();
    createInfo.pTessellationState = nullptr;
    // pipeline access shader resource
    createInfo.layout = pipelineLayout;
    // compatible render pass
    createInfo.renderPass = renderPass;
    createInfo.subpass = subPass;
    //other
    createInfo.basePipelineHandle = VK_NULL_HANDLE;
    createInfo.basePipelineIndex = 0;
    createInfo.flags = 0;
    createInfo.pNext = nullptr;
    

    VkPipeline createdPipeline = VK_NULL_HANDLE;
    result = vkCreateGraphicsPipelines(device, nullptr, 1, &createInfo, nullptr, &createdPipeline);
    if (result != VK_SUCCESS)
    {
        std::cout << "-->Create Graphics Pipeline Failed, Error: " << result << std::endl;
    }

    
    if (result == VK_SUCCESS)
    {
        m_Device = device;
        m_Pipeline = createdPipeline;
        m_PipelineLayout = pipelineLayout;
    }

    return IsCreate();
}


void GraphicPipeline::SetShader(VkShaderModule shaderMoule, VkShaderStageFlagBits shaderStage, const char* entryName)
{
    auto pos = std::find_if(m_ShaderInfo.begin(), m_ShaderInfo.end(), [=](const ShaderStageInfo &shaderInfo)
        { return shaderInfo.stage == shaderStage; });

    if(pos == m_ShaderInfo.end())
    {
        ShaderStageInfo shaderInfo{};
        shaderInfo.shaderMoudle = shaderMoule;
        shaderInfo.pEntryName = entryName;
        shaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        m_ShaderInfo.push_back(shaderInfo);
    }
    else 
    {
        pos->shaderMoudle = shaderMoule;
        pos->pEntryName = entryName;
    }
}


void GraphicPipeline::SRBindResource(uint32_t bindingLocation, VkDescriptorType resourceType, uint32_t resourceArrayElementCnt, VkShaderStageFlags accessStages, int layoutSetIdx)
{
    if (layoutSetIdx >= m_DescriptorSetLayoutsBinding.size())
        m_DescriptorSetLayoutsBinding.resize(layoutSetIdx + 1);
    
    auto& descriptorSetBindings = m_DescriptorSetLayoutsBinding[layoutSetIdx];
    auto pos = std::find_if(descriptorSetBindings.begin(), descriptorSetBindings.end(), [=](const VkDescriptorSetLayoutBinding &binding)
                            { return binding.binding == bindingLocation && binding.descriptorType == resourceType; });
    if (pos == descriptorSetBindings.end())
    {
        VkDescriptorSetLayoutBinding newBinding{};
        newBinding.binding = bindingLocation;
        newBinding.descriptorType = resourceType;
        newBinding.descriptorCount = resourceArrayElementCnt;
        newBinding.stageFlags = accessStages;
        newBinding.pImmutableSamplers = nullptr;
        descriptorSetBindings.push_back(newBinding);
    }
    else
    {
        pos->descriptorCount = resourceArrayElementCnt;
        pos->stageFlags = accessStages;
    }
}



void GraphicPipeline::Release()
{
    if (IsCreate())
    {
        vkDestroyPipelineLayout(m_Device, m_PipelineLayout, nullptr);
        vkDestroyPipeline(m_Device, m_Pipeline, nullptr);
        ResetDefaultStates();
        m_Device = VK_NULL_HANDLE;
        m_Pipeline = VK_NULL_HANDLE;
        m_PipelineLayout = VK_NULL_HANDLE;
    }
}