#pragma once
#include"Application.h"
#include"core\CoreUtils.h"


class CommandBuffer;

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

    VkRenderPass m_RenderPass{VK_NULL_HANDLE};    

public:
    ApiSample(const AppDesc& appDesc);
    ~ApiSample() = default;
    NONE_COPYABLE_NONE_MOVEABLE(ApiSample)

private:
    void Draw();

    void RecordDrawCommands(uint32_t swapChainImageIdx);

public:
    void Step() override { Draw(); };

    bool Setup() override;

    void Release() override;
};