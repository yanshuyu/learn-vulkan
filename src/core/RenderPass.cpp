#include"core\RenderPass.h"
#include<iostream>
#include<set>

RenderPass::RenderPass(VkDevice device)
: m_vkDevice(device)
{
}


RenderPass::RenderPass(RenderPass&& other)
{
    std::swap(m_vkDevice, other.m_vkDevice);
    std::swap(m_vkRenderPass, other.m_vkRenderPass);
    std::swap(m_attachmentDescs, other.m_attachmentDescs);
    std::swap(m_subpassIODesc, other.m_subpassIODesc);
    std::swap(m_sp_i_att_Refs, other.m_sp_i_att_Refs);
    std::swap(m_sp_o_col_att_refs, other.m_sp_o_col_att_refs);
    std::swap(m_sp_o_ds_att_refs, other.m_sp_o_ds_att_refs);
}

RenderPass& RenderPass::operator = (RenderPass&& other)
{
    if (this != &other)
    {
        Release();
        std::swap(m_vkDevice, other.m_vkDevice);
        std::swap(m_vkRenderPass, other.m_vkRenderPass);
        std::swap(m_attachmentDescs, other.m_attachmentDescs);
        std::swap(m_subpassIODesc, other.m_subpassIODesc);
        std::swap(m_sp_i_att_Refs, other.m_sp_i_att_Refs);
        std::swap(m_sp_o_col_att_refs, other.m_sp_o_col_att_refs);
        std::swap(m_sp_o_ds_att_refs, other.m_sp_o_ds_att_refs);
    }

    return *this;
}

RenderPass::~RenderPass()
{
    Release();
}


void RenderPass::AddAttachment(const AttachmentDesc& attachment, AttachmentLoadStoreAction loadStoreAction)
{
    VkAttachmentDescription desc{};
    desc.format = attachment.format;
    desc.samples = attachment.sampleCount;
    desc.loadOp = loadStoreAction.colorDepthLoadAction;
    desc.storeOp = loadStoreAction.colorDepthStoreAction;
    desc.stencilLoadOp = loadStoreAction.stencilLoadAction;
    desc.stencilStoreOp = loadStoreAction.stencilStoreAction;
    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    desc.finalLayout = vkutils_get_render_pass_attachment_best_output_layout(attachment.format);
    desc.flags = 0;

    m_attachmentDescs.push_back(desc);
}


bool RenderPass::CanAddSubPass(SubPassInputOutPutDesc& subPassInfo) const
{
    // ensure attchment ref idx is valid
    int maxInputIdx = subPassInfo.inputAttachmentIndices.empty() ? 0
                                                                 : *std::max_element(subPassInfo.inputAttachmentIndices.begin(), subPassInfo.inputAttachmentIndices.end());
    int maxOutputIdx = subPassInfo.outputAttachmentIndices.empty() ? 0
                                                                   : *std::max_element(subPassInfo.outputAttachmentIndices.begin(), subPassInfo.outputAttachmentIndices.end());
    int maxIdx = std::max(maxInputIdx, maxOutputIdx);
    if (maxIdx >= GetAttachmentCount())
        return false;

    // ensure no input/output attachments at the same time
    std::sort(subPassInfo.inputAttachmentIndices.begin(), subPassInfo.inputAttachmentIndices.end());
    std::sort(subPassInfo.outputAttachmentIndices.begin(), subPassInfo.outputAttachmentIndices.end());
    std::vector<int> interset_result;
    std::set_intersection(subPassInfo.inputAttachmentIndices.begin(),
                          subPassInfo.inputAttachmentIndices.end(),
                          subPassInfo.outputAttachmentIndices.begin(),
                          subPassInfo.outputAttachmentIndices.end(),
                          std::back_inserter(interset_result));

    return interset_result.empty();
}

bool RenderPass::AddSubPass(SubPassInputOutPutDesc& subPassInfo)
{
    if (!CanAddSubPass(subPassInfo))
    {
        LOGW("--> Failed to Add Render sub pass!");
        return false;
    }
    //
    // create this subpass input attachment refs
    //
    std::vector<VkAttachmentReference> iAttachmentRefs{};
    iAttachmentRefs.reserve(subPassInfo.inputAttachmentIndices.size());
    for (size_t i = 0; i < subPassInfo.inputAttachmentIndices.size(); i++)
    {
        int attchmentIdx = subPassInfo.inputAttachmentIndices[i];
        VkAttachmentReference aRef{};
        aRef.attachment = attchmentIdx;
        aRef.layout = vkutils_get_render_pass_attachment_best_input_layout(m_attachmentDescs[attchmentIdx].format);
        iAttachmentRefs.push_back(aRef);
    }
    m_sp_i_att_Refs.push_back(std::move(iAttachmentRefs));

    //
    // create this subpass output color attachment refs
    // depth/stencil attachment ref
    //
    std::vector<VkAttachmentReference> oAttachmentRefs{};
    std::vector<VkAttachmentReference> dsAttachmentRefs{};
    oAttachmentRefs.reserve(subPassInfo.outputAttachmentIndices.size());
    for (size_t i = 0; i < subPassInfo.outputAttachmentIndices.size(); i++)
    {
        int attachmentIdx = subPassInfo.outputAttachmentIndices[i];
        VkAttachmentReference aRef{};
        aRef.attachment = attachmentIdx;
        aRef.layout = vkutils_get_render_pass_attachment_best_output_layout(m_attachmentDescs[attachmentIdx].format);
        if (!vkutils_is_depth_or_stencil_format(m_attachmentDescs[attachmentIdx].format)) // color attachment
        {
            oAttachmentRefs.push_back(aRef);
        }
        else // depth/stencil attachemnt
        {
            if (!dsAttachmentRefs.empty())
            {
                LOGW("--> Try to output to multiple d/s attachment! Ignore this output");
            }
            else 
            {
                dsAttachmentRefs.push_back(aRef);
            }
        }
    }
    m_sp_o_col_att_refs.push_back(std::move(oAttachmentRefs));
    m_sp_o_ds_att_refs.push_back(std::move(dsAttachmentRefs));


    auto& ia = m_sp_i_att_Refs.back();
    auto& oa = m_sp_o_col_att_refs.back();
    auto& dsa = m_sp_o_ds_att_refs.back();
    VkSubpassDescription subPassDesc{};
    subPassDesc.flags = 0;
    subPassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subPassDesc.inputAttachmentCount = ia.size();
    subPassDesc.pInputAttachments = ia.data();
    subPassDesc.colorAttachmentCount = oa.size();
    subPassDesc.pColorAttachments = oa.data();
    subPassDesc.pDepthStencilAttachment = dsa.data();
    subPassDesc.pResolveAttachments = nullptr; // feature support
    subPassDesc.preserveAttachmentCount = 0;
    subPassDesc.pResolveAttachments = nullptr; // feature support
    
    m_subpassIODesc.push_back(std::move(subPassDesc));
    
    return true;
}


bool RenderPass::Create(bool force)
{
    if (force)
        Release();

    if (IsCreate())
        return true;
    
    if (VKHANDLE_IS_NULL(m_vkDevice) || m_attachmentDescs.empty())
        return false;

    // add a default subpass if there is not any
    if (m_subpassIODesc.empty())
    {
        SubPassInputOutPutDesc spIODesc{};
        for (size_t i = 0; i < m_attachmentDescs.size(); i++)
        {
            spIODesc.outputAttachmentIndices.push_back(i);
        }
        
        AddSubPass(spIODesc);
    }

    // create sub pass dependentcy
    std::vector<VkSubpassDependency> spDenpendentcys(m_subpassIODesc.size() - 1);
    for (size_t i = 0; i < spDenpendentcys.size(); i++)
    {
        // mark src subpass attachment act as input for dst subpass
        spDenpendentcys[i].srcSubpass = i;
        spDenpendentcys[i].dstSubpass = i + 1;
        spDenpendentcys[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        spDenpendentcys[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        spDenpendentcys[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        spDenpendentcys[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        spDenpendentcys[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    }
    
    // set attachment's init layout to first subpass's layout
    for (auto &&subpassDesc : m_subpassIODesc)
    {
        for (size_t i = 0; i < subpassDesc.colorAttachmentCount; i++)
        {
            auto& attachmentRef = subpassDesc.pColorAttachments[i];
            if (m_attachmentDescs[attachmentRef.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                m_attachmentDescs[attachmentRef.attachment].initialLayout = attachmentRef.layout;
        }

        for (size_t j = 0; j < subpassDesc.inputAttachmentCount; j++)
        {
            auto& attachmentRef = subpassDesc.pInputAttachments[j];
            if (m_attachmentDescs[attachmentRef.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                m_attachmentDescs[attachmentRef.attachment].initialLayout = attachmentRef.layout;
        }

        if (subpassDesc.pDepthStencilAttachment != nullptr)
        {
            auto& attachmentRef = *subpassDesc.pDepthStencilAttachment;
            if (m_attachmentDescs[attachmentRef.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED)
                m_attachmentDescs[attachmentRef.attachment].initialLayout = attachmentRef.layout;
        }

    }
    // set attachment's final layout to last sub pass's layout
    auto& subpassDesc = m_subpassIODesc.back();
    for (size_t i = 0; i < subpassDesc.colorAttachmentCount; i++)
    {
        auto &attachmentRef = subpassDesc.pColorAttachments[i];
        m_attachmentDescs[attachmentRef.attachment].finalLayout = attachmentRef.layout;
    }

    for (size_t j = 0; j < subpassDesc.inputAttachmentCount; j++)
    {
        auto &attachmentRef = subpassDesc.pInputAttachments[j];
        m_attachmentDescs[attachmentRef.attachment].finalLayout = attachmentRef.layout;
    }

    if (subpassDesc.pDepthStencilAttachment != nullptr)
    {
        auto &attachmentRef = *subpassDesc.pDepthStencilAttachment;
        m_attachmentDescs[attachmentRef.attachment].finalLayout = attachmentRef.layout;
    }

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = m_attachmentDescs.size();
    createInfo.pAttachments = m_attachmentDescs.data();
    createInfo.subpassCount = m_subpassIODesc.size();
    createInfo.pSubpasses = m_subpassIODesc.data();
    createInfo.dependencyCount = spDenpendentcys.size();
    createInfo.pDependencies = spDenpendentcys.data();


    VkRenderPass createdRenderPass{VK_NULL_HANDLE};
    VkResult result = vkCreateRenderPass(m_vkDevice, &createInfo, nullptr, &createdRenderPass);
    if (result != VK_SUCCESS)
    {
        LOGE("--> Create Render Pass Error: %d", result);
        return false;
    }

    m_vkRenderPass = createdRenderPass;

    
    

    return true;
}

RenderPass RenderPass::Create(VkDevice device,
                              const std::vector<AttachmentDesc> &attachments,
                              const std::vector<AttachmentLoadStoreAction> &attachmentLoadStoreActions,
                              std::vector<SubPassInputOutPutDesc> &subPassRWDescs)
{
    RenderPass rp(device);
    for (size_t i = 0; i < attachments.size(); i++)
    {
        AttachmentLoadStoreAction defaultLoadStore{};
        rp.AddAttachment(attachments[i], i < attachmentLoadStoreActions.size() ? attachmentLoadStoreActions[i] : defaultLoadStore);
    }

    for (size_t j = 0; j < subPassRWDescs.size(); j++)
    {
        rp.AddSubPass(subPassRWDescs[j]);
    }
    
    rp.Create();
     
    return std::move(rp);
}

void RenderPass::Release()
{
    if (IsCreate())
    {
        vkDestroyRenderPass(m_vkDevice, m_vkRenderPass, nullptr);
        m_vkDevice = VK_NULL_HANDLE;
        m_vkRenderPass = VK_NULL_HANDLE;
        m_attachmentDescs.clear();
        m_subpassIODesc.clear();
        m_sp_i_att_Refs.clear();
        m_sp_o_col_att_refs.clear();
        m_sp_o_ds_att_refs.clear();
    }
}


