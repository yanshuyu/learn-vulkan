#pragma once
#include"CoreUtils.h"


#define FRAME_BUFFER_MAX_ATTACHMENT 8

class RenderPass;


struct RenderTarget
{
    VkImageView attachment{VK_NULL_HANDLE};
    VkFormat format{VK_FORMAT_MAX_ENUM};
    VkClearValue clear{0};
    uint32_t width{0};
    uint32_t height{0};
    uint32_t sampleCount{1};

    RenderTarget() {}
    RenderTarget(VkImageView rt, VkFormat fmt, uint32_t w, uint32_t h, uint32_t sampleCnt = 1, bool clearRT = true);
    RenderTarget(VkImageView rt, VkFormat fmt, uint32_t w, uint32_t h, float clearColor[4], float clearDepth = 1.f, int clearStencil = 0, uint32_t sampleCnt = 1); 
};


class FrameBuffer
{
private:
    RenderTarget _renderTargets[FRAME_BUFFER_MAX_ATTACHMENT]{};
    VkClearValue _targetClears[FRAME_BUFFER_MAX_ATTACHMENT]{};
    RenderPass* _renderPass{nullptr};
    VkFramebuffer _frameBufferHandle{VK_NULL_HANDLE};
    uint32_t _width{0};
    uint32_t _height{0};
    size_t _numTarget{0};
public:
    FrameBuffer(uint32_t width, uint32_t height);
    ~FrameBuffer() { Release(); }

    NONE_COPYABLE_NONE_MOVEABLE(FrameBuffer)

    void SetRenderTarget(const RenderTarget& color);
    void SetRenderTarget(const RenderTarget& color, const RenderTarget& depth);
    void SetRenderTarget(const std::vector<RenderTarget> colors);
    void SetRenderTarget(const std::vector<RenderTarget> colors, const RenderTarget& depth);

    bool Validation(const RenderPass* RenderPass);
    bool Create(const RenderPass* renderPass);
    bool IsVaild() const {return _width > 0 && _height > 0 && VKHANDLE_IS_NOT_NULL(_frameBufferHandle); }
    void Release();

    VkFramebuffer GetHandle() const { return _frameBufferHandle; }
    uint32_t GetWidth() const { return _width; }
    uint32_t GetHeight() const { return _height; }
    size_t GetRenderTargetCount() const { return _numTarget; }
    const VkClearValue* GetRenderTargetClears() const { return _targetClears; }
    const RenderPass* GetRenderPass() const { return _renderPass; }

};