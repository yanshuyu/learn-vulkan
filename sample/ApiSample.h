#pragma once
#include"Application.h"
#include"core\CoreUtils.h"
#include<memory>
#include<vector>

class Buffer;
class Mesh;
class ShaderProgram;
class GraphicPipeline;
class CommandBuffer;
class RenderPass;

class ApiSample : public Application
{
private:
    VkSemaphore m_SwapChainImageAvalible{VK_NULL_HANDLE};     
    VkSemaphore m_PresentAvalible{VK_NULL_HANDLE};
    VkFence m_CmdBufferAvalible{VK_NULL_HANDLE};

    CommandBuffer* m_pCmdBuffer{nullptr};
    VkCommandBuffer m_CmdBuffer{VK_NULL_HANDLE};
    VkQueue m_GraphicQueue{VK_NULL_HANDLE};
    VkQueue m_PresentQueue{VK_NULL_HANDLE};

    std::unique_ptr<RenderPass> _renderPass{};

    VkImage m_DepthBuffer{VK_NULL_HANDLE};
    VkImageView m_DepthBufferView{VK_NULL_HANDLE};
    VkDeviceMemory m_DepthBufferMemory{VK_NULL_HANDLE};
    std::vector<VkFramebuffer> m_SwapChainFrameBuffers{};
    
    // triangle
    std::unique_ptr<Mesh> _triangleMesh{};
    ShaderProgram* _vertColorProgram{nullptr};
    std::unique_ptr<GraphicPipeline> _trianglePipeline{};
    Buffer* _triangleTransformUBO{nullptr};
    std::vector<VkDescriptorSet> _triangleDescriotorSets{};


public:
    ApiSample(const AppDesc& appDesc);
    ~ApiSample() = default;
    NONE_COPYABLE_NONE_MOVEABLE(ApiSample)

private:

    bool CreateSwapChainFrameBuffers();
    void DestroySwapChainFrameBuffers();


    void Draw();

    void RecordDrawCommands(uint32_t swapChainImageIdx);

public:
    void Step() override { Draw(); };

    bool Setup() override;

    void Release() override;
};