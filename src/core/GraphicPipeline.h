#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include<unordered_map>
#include<numeric>
#include<core\CoreUtils.h>
#include<core\PipelineState.h>

class Device;


class GraphicPipeline 
{

private:
    const Device* m_pDevice;
    PipelineState m_PipelineState;
    VkPipeline m_Pipeline;


public:
    GraphicPipeline(const Device* pDevice, const PipelineState& pipelineState);
    GraphicPipeline(const Device* pDevice, PipelineState&& pipelineState);
    ~GraphicPipeline();

    NONE_COPYABLE_NONE_MOVEABLE(GraphicPipeline)

    bool IsValid() const { return m_Pipeline != VK_NULL_HANDLE; }
    const PipelineState& GetPipelineState() const { return m_PipelineState; }
    VkPipeline GetHandle() const { return m_Pipeline; }
    VkPipelineLayout GetLayoutHandle() const;
    void Release();

private:
    VkResult _create();

};