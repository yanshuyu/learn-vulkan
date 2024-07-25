#include"FrameBuffer.h"
#include"RenderPass.h"
#include"Device.h"


RenderTarget::RenderTarget(VkImageView rt, VkFormat fmt, uint32_t w, uint32_t h, uint32_t sampleCnt, bool clearRT)
{
    attachment = rt;
    format = fmt;
    width = w;
    height = h;
    sampleCount = sampleCnt;
    if (clearRT)
    {
        if (vkutils_is_color_format(fmt))
        {
            clear.color.float32[0] = 0.f;
            clear.color.float32[1] = 0.f;
            clear.color.float32[2] = 0.f;
            clear.color.float32[3] = 1.f;
        }
        else
        {
            clear.depthStencil.depth = 1.f;
            clear.depthStencil.stencil = 0;
        }
    } 
}


RenderTarget::RenderTarget(VkImageView rt, VkFormat fmt, uint32_t w, uint32_t h, float clearColor[4], float clearDepth, int clearStencil, uint32_t sampleCnt)
{
    attachment = rt;
    format = fmt;
    width = w;
    height = h;
    sampleCount = sampleCnt;
    if (vkutils_is_color_format(fmt))
    {
        clear.color.float32[0] = clearColor[0];
        clear.color.float32[0] = clearColor[0];
        clear.color.float32[0] = clearColor[0];
        clear.color.float32[0] = clearColor[0];
    }
    else 
    {
        clear.depthStencil.depth = clearDepth;
        clear.depthStencil.stencil = clearStencil;
    }
}

FrameBuffer::FrameBuffer(uint32_t width, uint32_t height)
: _width(width)
, _height(height)
{

}

void FrameBuffer::SetRenderTarget(const RenderTarget& color)
{
    _renderTargets[0] = color;
    _targetClears[0] = color.clear;
    _numTarget = 1;
}

void FrameBuffer::SetRenderTarget(const RenderTarget& color, const RenderTarget& depth)
{
    _renderTargets[0] = color;
    _renderTargets[1] = depth;
    _targetClears[0] = color.clear;
    _targetClears[1] = depth.clear;
    _numTarget = 2;
}

void FrameBuffer::SetRenderTarget(const std::vector<RenderTarget> colors)
{
    for (size_t i = 0; i < colors.size(); i++)
    {
        _renderTargets[i] = colors[i];
        _targetClears[i] = colors[i].clear;
    }
    _numTarget = colors.size();
}

void FrameBuffer::SetRenderTarget(const std::vector<RenderTarget> colors, const RenderTarget& depth)
{
    for (size_t i = 0; i < colors.size(); i++)
    {
        _renderTargets[i] = colors[i];
        _targetClears[i] = colors[i].clear;
    }
    
    _renderTargets[colors.size()] = depth;
    _targetClears[colors.size()] = depth.clear;

    _numTarget = colors.size() + 1;
}

bool FrameBuffer::Validation(const RenderPass* renderPass)
{
    if (renderPass->GetAttachmentCount() != _numTarget)
        return false;
    
    for (size_t i = 0; i < _numTarget; i++)
    {
        if (_renderTargets[i].width != _width
            || _renderTargets[i].height != _height
            || _renderTargets[i].sampleCount != renderPass->GetAttachment(i).samples
            || _renderTargets[i].format != renderPass->GetAttachment(i).format
            )
            return false;
    }
    
    return true;
}

bool FrameBuffer::Create(const RenderPass* renderPass)
{
#ifdef _DEBUG
    assert(Validation(renderPass));
#endif
    if (!Validation(renderPass))
        return false;

    Release();

    VkImageView attachments[FRAME_BUFFER_MAX_ATTACHMENT]{};
    for (size_t i = 0; i < _numTarget; i++)
    {
        attachments[i] = _renderTargets[i].attachment;
    }
    
    VkFramebufferCreateInfo createInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
    createInfo.width = _width;
    createInfo.height = _height;
    createInfo.attachmentCount = _numTarget;
    createInfo.pAttachments = attachments;
    createInfo.layers = 1;
    createInfo.renderPass = renderPass->GetHandle();

    if (vkCreateFramebuffer(renderPass->GetDevice()->GetHandle(), &createInfo, nullptr, &_frameBufferHandle) == VK_SUCCESS)
    {
        _renderPass = const_cast<RenderPass*>(renderPass);
        return true;
    }

    return false;
}
    

void FrameBuffer::Release()
{
    if (VKHANDLE_IS_NOT_NULL(_frameBufferHandle))
    {
        vkDestroyFramebuffer(_renderPass->GetDevice()->GetHandle(), _frameBufferHandle, nullptr);
        VKHANDLE_SET_NULL(_frameBufferHandle);
    }
}