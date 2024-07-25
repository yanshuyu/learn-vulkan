#include"core\RenderPass.h"
#include<iostream>
#include<set>
#include"core\Device.h"

RenderPass::RenderPass(Device* pDevice): VKDeviceResource(pDevice)
{
}


RenderPass::~RenderPass()
{
    Release();
}


void RenderPass::AddColorAttachment(VkFormat fmt,
                            VkAttachmentLoadOp loadOp,
                            VkAttachmentStoreOp storeOp,
                            VkImageLayout initLayout,
                            VkImageLayout finalLayout,
                            size_t sampleCnt)
{
    assert(!vkutils_is_depth_or_stencil_format(fmt));
    VkAttachmentDescription2 desc{VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2};
    desc.flags = 0;
    desc.format = fmt;
    desc.samples = vkutils_sample_count_to_flag_bit(sampleCnt);
    desc.loadOp = loadOp;
    desc.storeOp = storeOp;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc.initialLayout = initLayout;
    desc.finalLayout = finalLayout;

    m_attachmentDescs.push_back(desc);
}

void RenderPass::AddDepthStencilAttachment(VkFormat fmt,
                                           VkAttachmentLoadOp depthLoadOp,
                                           VkAttachmentStoreOp depthStoreOp,
                                           VkAttachmentLoadOp stencilLoadOp,
                                           VkAttachmentStoreOp stencilStoreOp,
                                           VkImageLayout initLayout,
                                           VkImageLayout finalLayout,
                                           size_t sampleCnt)
{
    assert(vkutils_is_depth_or_stencil_format(fmt));
    VkAttachmentDescription2 desc{VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2};
    desc.flags = 0;
    desc.format = fmt;
    desc.samples = vkutils_sample_count_to_flag_bit(sampleCnt);
    desc.loadOp = depthLoadOp;
    desc.storeOp = depthStoreOp;
    desc.stencilLoadOp = stencilLoadOp;
    desc.stencilStoreOp = stencilStoreOp;
    desc.initialLayout = initLayout;
    desc.finalLayout = finalLayout;

    m_attachmentDescs.push_back(desc);
}

void RenderPass::AddSubPass(const size_t *outputAttachmentIndices,
                            size_t outputAttachmentCnt,
                            const size_t *inputAttachmentIndices,
                            size_t inputAttachmentCnt,
                            const std::pair<size_t, size_t> *colorResolveAttachments,
                            size_t colorResolveAttachmentCnt,
                            int depthStencilResolveAttachment,
                            VkResolveModeFlagBits depthResolveMode,
                            VkResolveModeFlagBits stencilResolveMode)
{

    //
    // create this subpass input attachment refs
    //
    std::vector<VkAttachmentReference2> iAttachmentRefs{};
    iAttachmentRefs.reserve(inputAttachmentCnt);
    for (size_t i = 0; i < inputAttachmentCnt; i++)
    {
        int attchmentIdx = inputAttachmentIndices[i];
        VkAttachmentReference2 aRef{VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2};
        aRef.attachment = attchmentIdx;
        aRef.layout = vkutils_get_render_pass_attachment_best_input_layout(m_attachmentDescs[attchmentIdx].format);
        aRef.aspectMask = vkutils_get_image_input_asepect_mask(m_attachmentDescs[attchmentIdx].format);
        iAttachmentRefs.push_back(aRef);
    }


    //
    // create this subpass color output attachment refs
    // depth/stencil attachment ref
    //
    std::vector<VkAttachmentReference2> oAttachmentRefs{};
    std::optional<VkAttachmentReference2> dsAttachmentRef{};
    oAttachmentRefs.reserve(outputAttachmentCnt);
    for (size_t i = 0; i < outputAttachmentCnt; i++)
    {
        int attachmentIdx = outputAttachmentIndices[i];
        VkAttachmentReference2 aRef{VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2};
        aRef.attachment = attachmentIdx;
        aRef.layout = vkutils_get_render_pass_attachment_best_output_layout(m_attachmentDescs[attachmentIdx].format);
        aRef.aspectMask = vkutils_get_image_output_asepect_mask(m_attachmentDescs[attachmentIdx].format);
        if (!vkutils_is_depth_or_stencil_format(m_attachmentDescs[attachmentIdx].format)) // color attachment
        {
            oAttachmentRefs.push_back(aRef);
        }
        else // depth/stencil attachemnt
        {
            if (dsAttachmentRef.has_value())
            {
                LOGW("--> Try to output to multiple d/s attachment!");
                assert(false);
            }
            else 
            {
                dsAttachmentRef = aRef;
            }
        }
    }

    // subpass color attachments resolve operation
    std::vector<VkAttachmentReference2> colorResolveAttachmentRefs{};
    if (colorResolveAttachmentCnt > 0)
    {
        colorResolveAttachmentRefs.resize(oAttachmentRefs.size());
        for (size_t i = 0; i < oAttachmentRefs.size(); i++)
        {   
            colorResolveAttachmentRefs[i].sType = VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2;
            colorResolveAttachmentRefs[i].attachment = VK_ATTACHMENT_UNUSED;
            colorResolveAttachmentRefs[i].layout = VK_IMAGE_LAYOUT_UNDEFINED;
            colorResolveAttachmentRefs[i].aspectMask = 0;

            for (size_t j = 0; j < colorResolveAttachmentCnt; j++)
            {
                if (oAttachmentRefs[i].attachment == colorResolveAttachments[j].first)
                {
                    colorResolveAttachmentRefs[i].attachment = colorResolveAttachments[j].second; // resolve attachment index
                    colorResolveAttachmentRefs[i].layout = oAttachmentRefs[i].layout;
                    colorResolveAttachmentRefs[i].aspectMask = oAttachmentRefs[i].aspectMask;
                    break;
                }
            }
        }
    }

    std::optional<VkAttachmentReference2> dsAttachmenResolveRef{};
    std::optional<VkSubpassDescriptionDepthStencilResolve> dsResolveMode{};
    // depth/stencil attachment resolve operation
    if (depthStencilResolveAttachment >= 0)
    {
        assert(dsAttachmentRef.has_value());
        assert(vkutils_is_depth_or_stencil_format(m_attachmentDescs[depthStencilResolveAttachment].format));

        VkAttachmentReference2 aRef{VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2};
        aRef.attachment = depthStencilResolveAttachment;
        aRef.layout = dsAttachmenResolveRef->layout;
        aRef.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        if (vkutils_is_depth_only_format(m_attachmentDescs[depthStencilResolveAttachment].format))
        {
            aRef.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
            stencilResolveMode = VK_RESOLVE_MODE_NONE;
            aRef.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        }
        else if (vkutils_is_stencil_only_format(m_attachmentDescs[depthStencilResolveAttachment].format))
        {
            aRef.layout = VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
            depthResolveMode = VK_RESOLVE_MODE_NONE;
            aRef.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;
        }
        dsAttachmenResolveRef = aRef;
        
        VkSubpassDescriptionDepthStencilResolve dsResolve{ VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE};
        dsResolve.depthResolveMode = depthResolveMode;
        dsResolve.stencilResolveMode = stencilResolveMode;
        dsResolveMode = dsResolve;
    }


    m_sp_i_att_refs.push_back(std::move(iAttachmentRefs));
    m_sp_o_col_att_refs.push_back(std::move(oAttachmentRefs));
    m_sp_col_resolve_att_refs.push_back(colorResolveAttachmentRefs);
    m_sp_o_ds_att_refs.push_back(dsAttachmentRef);
    m_sp_ds_resolve_att_refs.push_back(dsAttachmenResolveRef);
    m_sp_ds_resolve_infos.push_back(dsResolveMode);

    if (m_sp_ds_resolve_infos.back().has_value())
        m_sp_ds_resolve_infos.back()->pDepthStencilResolveAttachment = &(*m_sp_ds_resolve_att_refs.back());

}

bool RenderPass::Apply()
{
    if (IsValid())
        return true;
    
    if (m_attachmentDescs.empty())
        return false;

    // add a default subpass if there is not any
    if (GetSubPassCount() <= 0)
    {
        std::vector<size_t> outputAttachmentIndices(m_attachmentDescs.size());
        for (size_t i = 0; i < outputAttachmentIndices.size(); i++)
            outputAttachmentIndices[i] = i;
        AddSubPass(outputAttachmentIndices.data(), outputAttachmentIndices.size());  
    }

    // if user didn't set proper init layout
    // set attachment's init layout to first subpass's layout which use this attachment as input
    std::set<uint32_t> subPassOutputAttachmentIndices{};
    for (size_t subpassIdx = 0; subpassIdx < GetSubPassCount(); subpassIdx++)
    {
        for (auto &&attRef : m_sp_i_att_refs[subpassIdx])
        {
            if (m_attachmentDescs[attRef.attachment].initialLayout == VK_IMAGE_LAYOUT_UNDEFINED
                && subPassOutputAttachmentIndices.find(attRef.attachment) == subPassOutputAttachmentIndices.end())
                m_attachmentDescs[attRef.attachment].initialLayout = attRef.layout;
        }
        
        for (auto &&attRef : m_sp_o_col_att_refs[subpassIdx])
            subPassOutputAttachmentIndices.insert(attRef.attachment);
        
        for (auto &&attRef : m_sp_col_resolve_att_refs[subpassIdx])
            subPassOutputAttachmentIndices.insert(attRef.attachment);
        
        if (m_sp_o_ds_att_refs[subpassIdx].has_value())
            subPassOutputAttachmentIndices.insert(m_sp_o_ds_att_refs[subpassIdx]->attachment);

        if (m_sp_ds_resolve_att_refs[subpassIdx].has_value())
            subPassOutputAttachmentIndices.insert(m_sp_ds_resolve_att_refs[subpassIdx]->attachment); 
        
    }
    
    
    // if user didn't set proper final layout
    // set attachment's final layout to last subpass's layout which use this attachment as input/output 
    for (int subpassIdx = GetSubPassCount()-1; subpassIdx >= 0; subpassIdx--)
    {
        for (auto&& attRef : m_sp_o_col_att_refs[subpassIdx])
        {
            if (vkutils_is_initailable_layout(m_attachmentDescs[attRef.attachment].finalLayout))
                m_attachmentDescs[attRef.attachment].finalLayout = attRef.layout;
        }

        if (m_sp_o_ds_att_refs[subpassIdx].has_value())
        {   
            auto& attRef = *m_sp_o_ds_att_refs[subpassIdx];
            if (vkutils_is_initailable_layout(m_attachmentDescs[attRef.attachment].finalLayout))
                m_attachmentDescs[attRef.attachment].finalLayout = attRef.layout;
        }

        for (auto&& attRef : m_sp_i_att_refs[subpassIdx])
        {
            if (vkutils_is_initailable_layout(m_attachmentDescs[attRef.attachment].finalLayout))
                m_attachmentDescs[attRef.attachment].finalLayout = attRef.layout;
        }
        
    }

    // set resolve attachemt's final layout
    for (size_t subpassIdx = 0; subpassIdx < GetSubPassCount(); subpassIdx++)
    {
        for (auto&& attRef : m_sp_col_resolve_att_refs[subpassIdx])
        {
            if (vkutils_is_initailable_layout(m_attachmentDescs[attRef.attachment].finalLayout))
                m_attachmentDescs[attRef.attachment].finalLayout = attRef.layout;
        }
        
        if (m_sp_ds_resolve_att_refs[subpassIdx].has_value())
        {
            auto& attRef = *m_sp_ds_resolve_att_refs[subpassIdx];
            if (vkutils_is_initailable_layout(m_attachmentDescs[attRef.attachment].finalLayout))
                m_attachmentDescs[attRef.attachment].finalLayout = attRef.layout;
        }
    }


    // create sub pass dependentcy
    std::vector<VkSubpassDependency2> spDenpendentcys(GetSubPassCount() - 1);
    for (size_t i = 0; i < spDenpendentcys.size(); i++)
    {
        // mark src subpass attachment act as input for dst subpass
        spDenpendentcys[i].sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
        spDenpendentcys[i].srcSubpass = i;
        spDenpendentcys[i].dstSubpass = i + 1;
        spDenpendentcys[i].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        spDenpendentcys[i].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        spDenpendentcys[i].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        spDenpendentcys[i].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
        spDenpendentcys[i].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    }
    

    std::vector<VkSubpassDescription2> subPassDescs(GetSubPassCount());
    for (size_t i = 0; i < GetSubPassCount(); i++)
    {
        subPassDescs[i].sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
        subPassDescs[i].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subPassDescs[i].inputAttachmentCount = m_sp_i_att_refs[i].size();
        subPassDescs[i].pInputAttachments = m_sp_i_att_refs[i].data();
        subPassDescs[i].colorAttachmentCount = m_sp_o_col_att_refs[i].size();
        subPassDescs[i].pColorAttachments = m_sp_o_col_att_refs[i].data();
        subPassDescs[i].pResolveAttachments = m_sp_col_resolve_att_refs[i].size() > 0 ? m_sp_col_resolve_att_refs[i].data() : nullptr;
        subPassDescs[i].pDepthStencilAttachment = m_sp_o_ds_att_refs[i].has_value() ?  &(*m_sp_o_ds_att_refs[i]) : nullptr;
        subPassDescs[i].pNext = m_sp_ds_resolve_infos[i].has_value() ? &(*m_sp_ds_resolve_infos[i]) : nullptr;
    }

    VkRenderPassCreateInfo2 createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2;
    createInfo.pNext = nullptr;
    createInfo.flags = 0;
    createInfo.attachmentCount = m_attachmentDescs.size();
    createInfo.pAttachments = m_attachmentDescs.data();
    createInfo.subpassCount = subPassDescs.size();
    createInfo.pSubpasses = subPassDescs.data();
    createInfo.dependencyCount = spDenpendentcys.size();
    createInfo.pDependencies = spDenpendentcys.data();

    VkRenderPass createdRenderPass{VK_NULL_HANDLE};
    VkResult result = vkCreateRenderPass2(_pDevice->GetHandle(), &createInfo, nullptr, &createdRenderPass);
    if (result != VK_SUCCESS)
    {
        LOGE("--> Create Render Pass Error: {}", result);
        return false;
    }

    m_vkRenderPass = createdRenderPass;


    return true;
}

// RenderPass RenderPass::Create(VkDevice device,
//                               const std::vector<AttachmentDesc> &attachments,
//                               const std::vector<AttachmentLoadStoreAction> &attachmentLoadStoreActions,
//                               std::vector<SubPassInputOutPutDesc> &subPassRWDescs)
// {
//     RenderPass rp(device);
//     for (size_t i = 0; i < attachments.size(); i++)
//     {
//         AttachmentLoadStoreAction defaultLoadStore{};
//         rp.AddAttachment(attachments[i], i < attachmentLoadStoreActions.size() ? attachmentLoadStoreActions[i] : defaultLoadStore);
//     }

//     for (size_t j = 0; j < subPassRWDescs.size(); j++)
//     {
//         rp.AddSubPass(subPassRWDescs[j]);
//     }
    
//     rp.Create();
     
//     return std::move(rp);
// }

void RenderPass::Release()
{
    if (IsValid())
    {
        vkDestroyRenderPass(_pDevice->GetHandle(), m_vkRenderPass, nullptr);
        m_vkRenderPass = VK_NULL_HANDLE;
        m_attachmentDescs.clear();
        m_sp_i_att_refs.clear();
        m_sp_o_col_att_refs.clear();
        m_sp_o_ds_att_refs.clear();
        m_sp_col_resolve_att_refs.clear();
        m_sp_ds_resolve_att_refs.clear();
        m_sp_ds_resolve_infos.clear();
    }
}


