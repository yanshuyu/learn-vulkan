#pragma once
#include<core\CoreUtils.h>
#include<core\GraphicPipeline.h>
#include<rendering\PipelineStateHasher.h> 
#include<unordered_map>
#include<memory>


class Material;
class Mesh;
class RenderPass;

class PipelineManager
{
private:
    static Device* s_pDevice;

    static std::unordered_map<size_t, std::unique_ptr<GraphicPipeline>> s_GraphicPipelines;
    static std::unordered_map<size_t, size_t> s_RefrenceCounter;
    static std::unordered_map<GraphicPipeline*, size_t> s_PipelineHashes;

    static VkPipelineColorBlendAttachmentState _get_default_blend_state();
    static VkPipelineColorBlendAttachmentState _make_color_blend_state(const Material* material);

    PipelineManager() = delete;
    ~PipelineManager() = delete;
    NONE_COPYABLE_NONE_MOVEABLE(PipelineManager)

public:
    static void Initailize(Device* pdevice);
    static void DeInitailize();
    static GraphicPipeline* Request(const Mesh* mesh, const Material* material, const RenderPass* renderPass, size_t subPassIdx=0);
    static void Release(GraphicPipeline* pipeline);
    static PipelineState MakePipelineState(const Mesh* mesh, const Material* material, const RenderPass* renderPass, size_t subPassIdx=0);
   
};