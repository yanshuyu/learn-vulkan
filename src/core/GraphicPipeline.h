#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include<unordered_map>
#include<numeric>
#include"core\CoreUtils.h"


using std::vector;

class Device;
class ShaderProgram;
class RenderPass;
class Mesh;

class GraphicPipeline 
{

private:
    // vertex input
    //vector<VkVertexInputBindingDescription> m_VIBindingDesc{};
    //vector<VkVertexInputAttributeDescription> m_VIAttrsDesc{};

    // input assembly
    //VkPipelineInputAssemblyStateCreateInfo m_IADesc{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO}; 

    // viewport & scissor
    VkViewport m_Viewport{};
    VkRect2D m_Scissor{};

    // rasterizer
    VkPipelineRasterizationStateCreateInfo m_RSCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};

    // depth & stencil test
    VkPipelineDepthStencilStateCreateInfo m_DSCreateInfo {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    
    // framebuffer blending
    vector<VkPipelineColorBlendAttachmentState> m_FBAttchmentStates {};

    // dynamic state
    vector<VkDynamicState> m_DynamicStates;

    // multisample state
    VkPipelineMultisampleStateCreateInfo m_MSDesc{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    
    VkPipeline m_Pipeline{VK_NULL_HANDLE};

    Device* m_pDevice{nullptr};
    ShaderProgram* m_AssociateProgram{nullptr};
    Mesh* m_AssociateMesh{nullptr};
    RenderPass* m_AssociateRenderPass{nullptr};
    uint32_t m_AssociateSubpass{0};

public:
    GraphicPipeline(Device* pDevice, ShaderProgram* program, Mesh* mesh, RenderPass* renderPass, uint32_t subPas = 0);
    ~GraphicPipeline();


    //vertex input ----> driven from mesh and shader program 
    /*
    void VISetBinding(uint32_t location, uint32_t stride, VkVertexInputRate rate = VK_VERTEX_INPUT_RATE_VERTEX);
    VkVertexInputBindingDescription VIGetBinding(int idx) const { return m_VIBindingDesc[idx]; }
    int VIGetBindingCount() const { return m_VIBindingDesc.size(); }
    
    void VISetAttribute(uint32_t location, uint32_t binding, VkFormat fmt, uint32_t offset);
    VkVertexInputAttributeDescription VIGetAttribute(int idx) const { return m_VIAttrsDesc[idx]; }
    int VIGetAttributesCount() const { return m_VIAttrsDesc.size(); }
    

    // input assembly
    void IASetTopology(VkPrimitiveTopology pt, bool ptRestarted = false);
    VkPrimitiveTopology IAGetTopology() const { return m_IADesc.topology; }
    */

    // viewport & scissor
    void VSSetViewportScissorRect(VkRect2D viewportRect, VkRect2D scissorRect);
    VkRect2D VSGetScissorRect() const { return m_Scissor; }
    VkRect2D VSGetViewportRect() const 
    { 
        VkRect2D rect;
        rect.offset.x = m_Viewport.x;
        rect.offset.y = m_Viewport.y;
        rect.extent.width = m_Viewport.width;
        rect.extent.height = m_Viewport.height;
        return  rect;
    }

    // rasterization
    void RSSetCullFace(VkCullModeFlags cullMode) { m_RSCreateInfo.cullMode = cullMode; }
    void RSSetFillMode(VkPolygonMode fillMode) { m_RSCreateInfo.polygonMode = fillMode; }
    void RSSetFrontFaceOrder(VkFrontFace faceOrder) { m_RSCreateInfo.frontFace = faceOrder; }
    void RSSetLineWidth(uint32_t lw) { m_RSCreateInfo.lineWidth = lw; }
    void RSEnableDepthBias() { m_RSCreateInfo.depthBiasEnable = VK_TRUE; }
    void RSDisableDepthBias() { m_RSCreateInfo.depthBiasEnable = VK_FALSE; }
    void RSSetDepthBias(float constFactor, float slotFactor, float clamp = 0);
    VkCullModeFlags RSGetCullMode() const { return m_RSCreateInfo.cullMode; }
    VkPolygonMode RSGetFillMode() const { return m_RSCreateInfo.polygonMode; }
    VkFrontFace RSGetFrontFace()  const { return m_RSCreateInfo.frontFace; }
    bool RSGetDepthBiasEnabled() const { return m_RSCreateInfo.depthBiasEnable; }

    // depth & stencil test
    void DSEnableZTest() { m_DSCreateInfo.depthTestEnable = VK_TRUE; }
    void DSDisableZTest() { m_DSCreateInfo.depthTestEnable = VK_FALSE; }
    bool DSGetZTestEnabled() const { return m_DSCreateInfo.depthTestEnable; }
    void DSEnableZWrite() { m_DSCreateInfo.depthWriteEnable = VK_TRUE; }
    void DSDisableZWrite() { m_DSCreateInfo.depthWriteEnable = VK_FALSE;}
    bool DSGetZWriteEnabled() const { return m_DSCreateInfo.depthWriteEnable; }
    void DSSetZCmpOp(VkCompareOp zCmpOp) { m_DSCreateInfo.depthCompareOp = zCmpOp; }
    VkCompareOp DSGetZCmpOp() const { m_DSCreateInfo.depthCompareOp; }
    
    void DSEnableStencilTest() { m_DSCreateInfo.stencilTestEnable = VK_TRUE; }
    void DSDisableStenilTest() { m_DSCreateInfo.stencilTestEnable = VK_FALSE; }
    bool DSGetStencilTestEnabled() const { return m_DSCreateInfo.stencilTestEnable; }
    void DSSetStencilOp(uint32_t refVal, uint32_t readMask = std::numeric_limits<uint32_t>::max(),
                        uint32_t writeMask = std::numeric_limits<uint32_t>::max(),
                        VkCompareOp cmp = VK_COMPARE_OP_EQUAL,
                        VkStencilOp passOp = VK_STENCIL_OP_REPLACE,
                        VkStencilOp depthFailOP = VK_STENCIL_OP_KEEP,
                        VkStencilOp failp = VK_STENCIL_OP_KEEP);
    VkStencilOpState DSGetStencilOp() const { return m_DSCreateInfo.front; }

    // frame buffer color blending
    void FBEnableBlend(int attachmentIdx);
    void FBDisableBlend(int attachmentIdx);
    bool FBGetBlendEnabled(int attachmentIdx) const;
    void FBSetColorBlendOp(int attachmentIdx, VkBlendFactor srcFactor, VkBlendFactor dstFactor, VkBlendOp op, VkColorComponentFlags colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT);
    bool FBGetColorBlendOp(int attachmentIdx, VkBlendFactor& srcFactor, VkBlendFactor& dstFactor, VkBlendOp& op, VkColorComponentFlags& colorWriteMask) const;
    void FBSetAlphaBlendOp(int attachmentIdx, VkBlendFactor srcFactor, VkBlendFactor dstFactor, VkBlendOp op);
    bool FBGetAlphaBlendOp(int attachmentIdx, VkBlendFactor& srcFactor, VkBlendFactor& dstFactor, VkBlendOp& op) const;

    // dynamic states
    void EnableDynamicState(VkDynamicState state);
    void DisableDynamicState(VkDynamicState state);;
    bool IsDynamicState(VkDynamicState state) const;
    int DynamicStateCount() const { return m_DynamicStates.size(); }
    const std::vector<VkDynamicState>& GetDynamicStates() const { return m_DynamicStates; }

    // multi sample state
    void MSEnableAlphaToCoverage() { m_MSDesc.alphaToCoverageEnable = true; }
    void MSDisableAlphaToCoverage() { m_MSDesc.alphaToCoverageEnable = false; }
    bool MSAlphaToCoverageEnabled() const { return m_MSDesc.alphaToCoverageEnable; }
    void MSEnableAlphaToOne() { m_MSDesc.alphaToOneEnable = true; }
    void MSDisableAlphaToOne() { m_MSDesc.alphaToOneEnable = false; }
    bool MSGetAlphaToOneEnable() { return m_MSDesc.alphaToOneEnable; }
    void MSSetSampleCount(int sampleCnt);
    int MSGetSampleCount() const;

    bool Apply();
    bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }
    VkPipeline GetHandle() const { return m_Pipeline; }
    VkPipelineLayout GetLayoutHandle() const;
    void Release();

private:
    void ResetDefaultStates();
    static VkPipelineColorBlendAttachmentState GetDefaultBlendState()
    {
        VkPipelineColorBlendAttachmentState bs;
        bs.blendEnable = VK_FALSE;
        bs.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
        bs.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;;
        bs.colorBlendOp = VK_BLEND_OP_ADD;;
        bs.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        bs.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        bs.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        bs.alphaBlendOp = VK_BLEND_OP_ADD;
        return bs;
    }

    static VkPipelineRasterizationStateCreateInfo GetDefaultRasterizationState()
    {
        VkPipelineRasterizationStateCreateInfo rs{};
        rs.cullMode = VK_CULL_MODE_BACK_BIT;
        rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        rs.lineWidth = 1;;
        rs.rasterizerDiscardEnable = false;
        rs.depthBiasEnable = false;
        rs.depthClampEnable = false;
        rs.depthBiasConstantFactor = 0;
        rs.depthBiasSlopeFactor = 0;;
        rs.depthBiasClamp = 0;
        return rs;;
    }

    static VkPipelineDepthStencilStateCreateInfo GetDefaultDepthStencilState()
    {
        VkPipelineDepthStencilStateCreateInfo ds{};
        VkStencilOpState ss{};
        ss.reference = 0;
        ss.compareMask = std::numeric_limits<uint32_t>::max();
        ss.writeMask = std::numeric_limits<uint32_t>::max();
        ss.compareOp = VK_COMPARE_OP_ALWAYS;
        ss.passOp = VK_STENCIL_OP_KEEP;
        ss.failOp = VK_STENCIL_OP_KEEP;
        ss.depthFailOp = VK_STENCIL_OP_KEEP;
         
        ds.depthTestEnable = true;
        ds.depthBoundsTestEnable = false;
        ds.depthWriteEnable = true;
        ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        ds.stencilTestEnable = false;
        ds.front = ss;
        ds.back = ss;

        return ds;
    }

};