#pragma once
#include<core\PipelineState.h>
//#include<functional>

// boost's hash_combine implementation
void hash_combine(size_t &seed, size_t hash);

size_t hash_fundamentals(const char* types, ...);


template<>
struct std::hash<VkVertexInputAttributeDescription>
{
    size_t operator () (const VkVertexInputAttributeDescription& val)
    {   
        return hash_fundamentals("%ud %ud %ud %ud", val.binding, val.location, val.format, (uint32_t)val.format);
    }
};

template<>
struct std::hash<VkVertexInputBindingDescription>
{
    size_t operator () (const VkVertexInputBindingDescription& val)
    {
        return hash_fundamentals("%ud %ud %ud", val.binding, val.stride, (uint32_t)val.inputRate);
    }
};

template<>
struct std::hash<VkPipelineColorBlendAttachmentState>
{
    size_t operator () (const VkPipelineColorBlendAttachmentState& val)
    {
        return hash_fundamentals("%b %ud %ud %ud %ud %ud %ud %ud", 
                                val.blendEnable,
                                (uint32_t)val.srcColorBlendFactor, 
                                (uint32_t)val.dstColorBlendFactor,
                                (uint32_t)val.colorBlendOp,
                                (uint32_t)val.srcAlphaBlendFactor,
                                (uint32_t)val.dstAlphaBlendFactor,
                                (uint32_t)val.alphaBlendOp,
                                (uint32_t)val.colorWriteMask);
    }
};

template<>
struct std::hash<VkPipelineShaderStageCreateInfo>
{
    size_t operator () (const VkPipelineShaderStageCreateInfo& val)
    {
        return hash_fundamentals("%ud %s %ud %ud", (uint32_t)val.module,
                                    val.pName, 
                                    (uint32_t)val.stage,
                                    (uint32_t)val.flags);
    }
};


template<typename T> 
struct PipelineStateHasher
{
    size_t operator () (const T& t)
    {
        std::hash<T> h{};
        return h(t);
    }
};


template<>
struct PipelineStateHasher<VertexInputState>
{
    size_t operator () (const VertexInputState& vis)
    {
        if (vis.attrsDescs.size() == 0 && vis.bindingDescs.size() == 0)
            return 0;
        
        size_t h{0};
        for (auto &&attr : vis.attrsDescs)
        {
            hash_combine(h, std::hash<VkVertexInputAttributeDescription>{}(attr));
        }

        for (auto &&binding : vis.bindingDescs)
        {
            hash_combine(h, std::hash<VkVertexInputBindingDescription>{}(binding));
        }
        
        return h;
    }
};


template<>
struct PipelineStateHasher<InputAssemblyState>
{
    size_t operator () (const InputAssemblyState& ias)
    {
        return hash_fundamentals("%ud %b", ias.topology, ias.restartEnable);
    }
};

template<>
struct PipelineStateHasher<ViewporScissorState>
{
    size_t operator () (const ViewporScissorState& vss)
    {
        size_t h{0};
        hash_combine(h, hash_fundamentals("%f %f %f %f %f %f", vss.viewport.x, vss.viewport.y, vss.viewport.width, vss.viewport.height, vss.viewport.minDepth, vss.viewport.maxDepth));
        hash_combine(h, hash_fundamentals("%d %d %ud %ud", vss.scissor.offset.x, vss.scissor.offset.y, vss.scissor.extent.width, vss.scissor.extent.height));
        return h;
    }
};


template<>
struct PipelineStateHasher<MultiSampleState>
{
    size_t operator () (const MultiSampleState& mss)
    {
        return hash_fundamentals("%ud %b %b", (uint32_t)mss.rasterizationSamples, mss.alphaToCoverageEnable, mss.alphaToOneEnable);
    }
};


template<>
struct PipelineStateHasher<RasterizationState>
{
    size_t operator () (const RasterizationState& ras)
    {
        return hash_fundamentals("%ud %ud %ud %ud %b %f %f %f", (uint32_t)ras.cullMode
                                , (uint32_t)ras.frontFace
                                , (uint32_t)ras.polygonMode
                                , (uint32_t)ras.lineWidth
                                , ras.depthBiasEnable
                                , ras.depthBiasConstantFactor
                                , ras.depthBiasSlopeFactor
                                , ras.depthBiasClamp);
    }
};



template<>
struct PipelineStateHasher<DepthStencilState>
{
    size_t operator () (const DepthStencilState& dss)
    {
        size_t h{0};
        hash_fundamentals("%b %ud %b", dss.depthTestEnable, (uint32_t)dss.depthCompareOp, dss.depthWriteEnable);
        hash_fundamentals("%b", dss.stencilTestEnable);
        hash_fundamentals("%ud %ud %ud %ud %ud %ud %ud", dss.front.reference,
                          (uint32_t)dss.front.compareOp,
                          (uint32_t)dss.front.passOp,
                          (uint32_t)dss.front.failOp,
                          (uint32_t)dss.front.depthFailOp,
                          dss.front.compareMask,
                          dss.front.writeMask);
        hash_fundamentals("%ud %ud %ud %ud %ud %ud %ud", dss.back.reference,
                          (uint32_t)dss.back.compareOp,
                          (uint32_t)dss.back.passOp,
                          (uint32_t)dss.back.failOp,
                          (uint32_t)dss.back.depthFailOp,
                          dss.back.compareMask,
                          dss.back.writeMask);
        return h;
    }
};


template<>
struct PipelineStateHasher<BlendState>
{
    size_t operator () (const BlendState& fbs)
    {
        size_t h{0};
        for (auto &&abs : fbs.attachmentBlendStates)
        {
            hash_combine(h, std::hash<VkPipelineColorBlendAttachmentState>{}(abs));
        }
        
        return h;
    }
};

template<>
struct PipelineStateHasher<DynamicState>
{
    size_t operator () (const DynamicState& dys)
    {
        size_t h{0};
        for (auto &&dys : dys.states)
        {
            hash_combine(h, std::hash<uint32_t>{}(dys));
        }
        
        return h;
    }
};

template<>
struct PipelineStateHasher<ShaderStageState>
{
    size_t operator () (const ShaderStageState& sss)
    {
        size_t h{0};
        for (auto &&ssi : sss.stageInfo)
        {
            hash_combine(h, std::hash<VkPipelineShaderStageCreateInfo>{}(ssi));
        }
        hash_combine(h, std::hash<uint32_t>{}((uint32_t)sss.resourceLayout));
        
        return h;
    }
};



template<>
struct PipelineStateHasher<RenderPassState>
{
    size_t operator () (const RenderPassState& rps)
    {
        return hash_fundamentals("%ud %ud", (uint32_t)rps.renderPass, rps.subPass);
    }
};


template<typename T>
void hashing_pipeline_state(size_t& seed, const  T& t)
{
    PipelineStateHasher<T> stateHasher{};
    hash_combine(seed, stateHasher(t));
}


template<typename T, typename ... Args>
void hashing_pipeline_state(size_t& seed, const T& t, const Args& ... args)
{
    hashing_pipeline_state(seed, t);
    hashing_pipeline_state(seed, args...);
}


template<>
struct PipelineStateHasher<PipelineState>
{
    size_t operator () (const PipelineState& ps)
    {
        size_t h{0};
        hashing_pipeline_state(h, ps.viState, ps.iaState, ps.vsState, ps.msaaState, ps.raState, ps.dsState, ps.fbState, ps.dyState, ps.shaderState, ps.rpState);
        return h;
    }
};


