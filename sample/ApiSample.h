#pragma once
#include<app\Application.h>
#include<core\CoreUtils.h>
#include<rendering\GameTimer.h>
#include<rendering\OrbitCamera.h>
#include<memory>
#include<vector>


class Buffer;
class Mesh;
class ShaderProgram;
class GraphicPipeline;
class CommandBuffer;
class RenderPass;
class Texture2D;
class TextureCube;
struct PerFrameData;
struct PerCameraData;
struct PerObjectData;
class Material;
class FrameBuffer;


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
    std::vector<std::unique_ptr<FrameBuffer>> m_SwapChainFrameBuffers{};
    
    // quad
    std::unique_ptr<Mesh> _quad{};
    ShaderProgram* _vertColorProgram{nullptr};
    GraphicPipeline* _quadPipeline{};
    //VkDescriptorSet _quadSet{VK_NULL_HANDLE};
    std::unique_ptr<Material> _quad_mat{};
    std::unique_ptr<Texture2D> _vkLogoTex{};
    std::unique_ptr<PerObjectData> _quadInstanceData{};
    glm::vec4 _quadMainColor{1};

    // skybox
    std::unique_ptr<Mesh> _cube{};
    ShaderProgram* _skyboxProgram{nullptr};
    GraphicPipeline* _skyboxPipeline{};
    //VkDescriptorSet _skyboxSet{VK_NULL_HANDLE};
    std::unique_ptr<Material> _skyboxMat{};
    std::unique_ptr<TextureCube> _skyboxTex{};
    std::unique_ptr<PerObjectData> _skyboxInstanceData{};

    std::unique_ptr<PerFrameData> _perFrameData{};
    std::unique_ptr<PerCameraData> _perCameraData{};

    OrbitCamera _camera{};

public:
    ApiSample(const AppDesc& appDesc);
    ~ApiSample() = default;
    NONE_COPYABLE_NONE_MOVEABLE(ApiSample)

private:

    bool CreateSwapChainFrameBuffers();
    void DestroySwapChainFrameBuffers();
    
    void _set_up_quad();
    void _clean_up_quad();
    void _set_up_sky_box();
    void _clean_up_sky_box();

    void Update();

    void Draw();

    void RecordDrawCommands(uint32_t swapChainImageIdx);

public:
    void Step() override;

    bool Setup() override;

    void Release() override;
};