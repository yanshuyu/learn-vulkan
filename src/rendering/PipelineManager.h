#pragma once
#include<core\CoreUtils.h>
#include<core\GraphicPipeline.h> 
#include<unordered_map>
#include<memory>


class Material;
class Mesh;
class RenderPass;

class PipelineManager
{
private:
    static std::unordered_map<size_t, std::unique_ptr<GraphicPipeline>> s_GraphicPipelines;


    static void hash_combine(size_t &seed, size_t hash)
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}


    static VkPipelineColorBlendAttachmentState _get_default_blend_state();
    static VkPipelineColorBlendAttachmentState _make_color_blend_state(const Material* material);

    PipelineManager() = delete;
    ~PipelineManager() = delete;
    NONE_COPYABLE_NONE_MOVEABLE(PipelineManager)

public:
    static GraphicPipeline* Request(const Mesh* mesh, const Material* material, const RenderPass* renderPass, size_t subPassIdx=0);
    static PipelineState MakePipelineState(const Mesh* mesh, const Material* material, const RenderPass* renderPass, size_t subPassIdx=0);
   
};