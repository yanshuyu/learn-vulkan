#include<core\PipelineState.h>
#include<algorithm>

PipelineState::PipelineState(const PipelineState& other)
{
    *this = other;
}

PipelineState::PipelineState(PipelineState&& other)
{
    *this = std::move(other);
}

PipelineState& PipelineState::operator = (const PipelineState& other)
{
    if (this == &other)
        return *this;
    
    // vertex input
    viState.bindingDescs.resize(other.viState.bindingDescs.size());
    viState.attrsDescs.resize(other.viState.attrsDescs.size());
    std::copy(other.viState.bindingDescs.cbegin(), other.viState.bindingDescs.cend(), viState.bindingDescs.begin());
    std::copy(other.viState.attrsDescs.cbegin(), other.viState.attrsDescs.cend(), viState.attrsDescs.begin());
    // input assembler
    iaState = other.iaState;
    // viewport scissor
    vsState = other.vsState;
    // msaa
    msaaState = other.msaaState;
    // rasterization
    raState = other.raState;
    // depth stencil 
    dsState = other.dsState;
    // blend
    fbState.attachmentBlendStates.resize(other.fbState.attachmentBlendStates.size());
    std::copy(other.fbState.attachmentBlendStates.cbegin(), other.fbState.attachmentBlendStates.cend(), fbState.attachmentBlendStates.begin());
    // dynamic state
    dyState.states.resize(other.dyState.states.size());
    std::copy(other.dyState.states.cbegin(), other.dyState.states.cend(), dyState.states.begin());
    // shader stage
    shaderState.resourceLayout = other.shaderState.resourceLayout;
    shaderState.stageInfo.resize(other.shaderState.stageInfo.size());
    std::copy(other.shaderState.stageInfo.cbegin(), other.shaderState.stageInfo.cend(), shaderState.stageInfo.begin());
    // render pass
    rpState.renderPass = other.rpState.renderPass;
    rpState.subPass = other.rpState.subPass;

    return *this;
}

PipelineState& PipelineState::operator = (PipelineState&& other)
{
    if (this == &other)
        return *this;
    // vertex input
    viState.bindingDescs.swap(other.viState.bindingDescs);
    viState.attrsDescs.swap(other.viState.attrsDescs);
    // input assembler
    iaState = other.iaState;
    // viewport scissor
    vsState = other.vsState;
    // msaa
    msaaState = other.msaaState;
    // rasterization
    raState = other.raState;
    // depth stencil 
    dsState = other.dsState;
    // blend
    fbState.attachmentBlendStates.swap(other.fbState.attachmentBlendStates);
    // dynamic state
    dyState.states.swap(other.dyState.states);
    // shader stage
    shaderState.resourceLayout = other.shaderState.resourceLayout;
    shaderState.stageInfo.swap(other.shaderState.stageInfo);
    // render pass
    rpState.renderPass = other.rpState.renderPass;
    rpState.subPass = other.rpState.subPass;

    return *this;    
}