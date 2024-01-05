#pragma once
#include"core\CoreUtils.h"
#include<vulkan\vulkan.h>
#include<vector>

class RenderPass
{

public:
    RenderPass(VkDevice device);
    RenderPass(RenderPass&& other);
    RenderPass& operator = (RenderPass&& other);
    ~RenderPass();

    NONE_COPYABLE(RenderPass)

    bool Create(bool force = false);
    static RenderPass Create(VkDevice device, 
                            const std::vector<AttachmentDesc>& attachments,
                            const std::vector<AttachmentLoadStoreAction>& attachmentLoadStoreActions,
                            std::vector<SubPassInputOutPutDesc>& subPassRWDescs); 
    void Release();

    void AddAttachment(const AttachmentDesc& attachment, AttachmentLoadStoreAction loadStoreAction);
    bool CanAddSubPass(SubPassInputOutPutDesc& subPassInfo) const;
    bool AddSubPass(SubPassInputOutPutDesc& subPassInfo);

    bool IsCreate() const { return VKHANDLE_IS_NOT_NULL(m_vkDevice) && VKHANDLE_IS_NOT_NULL(m_vkRenderPass);}

    int GetAttachmentCount() const { return m_attachmentDescs.size(); }
    int GetSubPassCount() const { return m_subpassIODesc.size(); }

    VkRenderPass GetHandle() const { return m_vkRenderPass; }
    VkDevice GetDeviceHandle() const { return m_vkDevice; }

private:

    VkDevice m_vkDevice{VK_NULL_HANDLE};
    VkRenderPass m_vkRenderPass{VK_NULL_HANDLE};

    std::vector<VkAttachmentDescription> m_attachmentDescs{};

    std::vector<VkSubpassDescription> m_subpassIODesc{};
    std::vector<std::vector<VkAttachmentReference>> m_sp_i_att_Refs{};
    std::vector<std::vector<VkAttachmentReference>> m_sp_o_col_att_refs{};
    std::vector<std::vector<VkAttachmentReference>> m_sp_o_ds_att_refs{};
};

