#pragma once
#include<core\CoreUtils.h> 

struct VertexInputState
{
    std::vector<VkVertexInputBindingDescription> bindingDescs{};
    std::vector<VkVertexInputAttributeDescription> attrsDescs{};
};

struct InputAssemblyState
{
    VkPrimitiveTopology  topology{VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST};
    bool restartEnable{false};
};

struct ViewporScissorState
{
    VkViewport viewport{};
    VkRect2D scissor{};
};

struct RasterizationState
{
    VkPolygonMode polygonMode{VK_POLYGON_MODE_FILL};
    VkCullModeFlags cullMode{VK_CULL_MODE_BACK_BIT};
    VkFrontFace frontFace{VK_FRONT_FACE_CLOCKWISE};
    bool depthBiasEnable{false};
    float depthBiasConstantFactor{0};
    float depthBiasClamp{0};
    float depthBiasSlopeFactor{0};
    float lineWidth{1.f};
};

struct MultiSampleState
{
    VkSampleCountFlagBits rasterizationSamples {VK_SAMPLE_COUNT_1_BIT};
    bool alphaToCoverageEnable{false};
    bool alphaToOneEnable{false};
};

struct DepthStencilState
{
    bool depthTestEnable{true};
    bool depthWriteEnable{true};
    VkCompareOp depthCompareOp{VK_COMPARE_OP_LESS_OR_EQUAL};
    bool stencilTestEnable{false};
    VkStencilOpState front{};
    VkStencilOpState back{};
};

struct BlendState
{
   std::vector<VkPipelineColorBlendAttachmentState> attachmentBlendStates{};
};

struct DynamicState
{
    std::vector<VkDynamicState> states{};
};


struct ShaderStageState
{
    std::vector<VkPipelineShaderStageCreateInfo> stageInfo{};
    VkPipelineLayout resourceLayout{VK_NULL_HANDLE};
};

struct RenderPassState
{
    VkRenderPass renderPass{VK_NULL_HANDLE};
    uint32_t subPass{0};
};


struct PipelineState
{
    VertexInputState viState{};
    InputAssemblyState iaState{};
    ViewporScissorState vsState{};
    RasterizationState raState{};
    MultiSampleState msaaState{};
    DepthStencilState dsState{};
    BlendState fbState{};
    DynamicState dyState{};
    ShaderStageState shaderState{};
    RenderPassState rpState;

    PipelineState() = default;
    PipelineState(const PipelineState& other);
    PipelineState(PipelineState&& other);

    PipelineState& operator = (const PipelineState& other);
    PipelineState& operator = (PipelineState&& other);
};