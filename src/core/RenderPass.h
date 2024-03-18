#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include<optional>
#include"core\CoreUtils.h"
#include"VKDeviceResource.h"

class Device;

class RenderPass : public VKDeviceResource
{

public:
    RenderPass(Device* pDevice);
    ~RenderPass();

    NONE_COPYABLE_NONE_MOVEABLE(RenderPass)

    bool Apply();
    // static RenderPass Create(VkDevice device, 
    //                         const std::vector<AttachmentDesc>& attachments,
    //                         const std::vector<AttachmentLoadStoreAction>& attachmentLoadStoreActions,
    //                         std::vector<SubPassInputOutPutDesc>& subPassRWDescs); 
    void Release() override;
    bool IsValid() const override { return  VKHANDLE_IS_NOT_NULL(m_vkRenderPass); }

    void AddColorAttachment(VkFormat fmt,
                            VkAttachmentLoadOp loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                            VkAttachmentStoreOp storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                            VkImageLayout initLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                            size_t sampleCnt = 1);


    void AddDepthStencilAttachment(VkFormat fmt,
                                   VkAttachmentLoadOp depthLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                   VkAttachmentStoreOp depthStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                   VkAttachmentLoadOp stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                                   VkAttachmentStoreOp stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                   VkImageLayout initLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                   VkImageLayout finalLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                                   size_t sampleCnt = 1);


    void AddSubPass(const size_t* outputAttachmentIndices,
                    size_t outputAttachmentCnt, 
                    const size_t* inputAttachmentIndices = nullptr, 
                    size_t inputAttachmentCnt = 0, 
                    const std::pair<size_t, size_t>* colorResolveAttachments = nullptr,
                    size_t colorResolveAttachmentCnt = 0,
                    int depthStencilResolveAttachment = -1,
                    VkResolveModeFlagBits depthResolveMode = VK_RESOLVE_MODE_MIN_BIT,
                    VkResolveModeFlagBits stencilResolveMode = VK_RESOLVE_MODE_NONE);


    int GetAttachmentCount() const { return m_attachmentDescs.size(); }
    int GetSubPassCount() const { return m_sp_i_att_refs.size(); }
    //VkSubpassDescription2 GetSubPassDesc(size_t idx) const {return m_subpassDescs[idx]; }
    size_t GetSubPassOutputColorAttachmentCount(size_t idx) const {return m_sp_o_col_att_refs[idx].size(); }
    size_t GetSubPassInputAttachmentCount(size_t idx) const { return m_sp_i_att_refs[idx].size(); }

    VkRenderPass GetHandle() const { return m_vkRenderPass; }

private:

    //Device* _pDevice;
    VkRenderPass m_vkRenderPass{VK_NULL_HANDLE};

    std::vector<VkAttachmentDescription2> m_attachmentDescs{};

    std::vector<std::vector<VkAttachmentReference2>> m_sp_i_att_refs{};
    std::vector<std::vector<VkAttachmentReference2>> m_sp_o_col_att_refs{};
    std::vector<std::optional<VkAttachmentReference2>> m_sp_o_ds_att_refs{};
    std::vector<std::vector<VkAttachmentReference2>> m_sp_col_resolve_att_refs{};
    std::vector<std::optional<VkAttachmentReference2>> m_sp_ds_resolve_att_refs{};
    std::vector<std::optional<VkSubpassDescriptionDepthStencilResolve>> m_sp_ds_resolve_infos{};

    //std::vector<VkSubpassDescription2> m_subpassDescs{};
};

