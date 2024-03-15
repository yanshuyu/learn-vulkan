#pragma once
#include<vulkan\vulkan.h>
#include"core\CoreUtils.h"
#include"core\VKDeviceResource.h"


class Buffer;
class Fence;

class CommandBuffer : public VKDeviceResource
{
    friend class Device;
public:
    enum State 
    {
        Invalid,
        Initial,
        Recording,
        Executable,
    };

private:
    VkCommandPool _vkCmdPool{VK_NULL_HANDLE};
    VkQueue _vkQueue{VK_NULL_HANDLE};
    VkCommandBuffer _vkCmdBuffer{VK_NULL_HANDLE};
    bool _temprary{true};
    State _state { State::Invalid};

    bool _create(VkCommandPool cmdPool, VkQueue exeQueue, bool isTemp);
    bool _execute(Fence* fence);
    void Release() override;

public:
    CommandBuffer(Device* pDevice = nullptr);
    ~CommandBuffer() { assert(!IsValid()); }

    NONE_COPYABLE_NONE_MOVEABLE(CommandBuffer)

    VkCommandPool GetPoolHandle() const { return _vkCmdPool; }
    VkCommandBuffer GetHandle() const { return _vkCmdBuffer; }
    VkQueue GetQueueHandle() const { return _vkQueue; }
    
    bool IsTemprary() const { return _temprary; }
    bool IsValid() const override;
    bool CanExecute() const { return IsValid() && !_temprary && _state == State::Executable; }
    bool Begin();
    bool End();
    bool ExecuteSync() { return _execute(nullptr); }
    bool ExecuteAsync(Fence* fence) { return _execute(fence); }
    bool Reset();
    bool CopyBuffer(const Buffer* src,
                      size_t srcOffset,
                      Buffer* dst,
                      size_t dstOffset,
                      size_t dataSz,
                      VkPipelineStageFlags waitStageMask,
                      VkAccessFlags waitAccessMask,
                      VkPipelineStageFlags signalStageMask,
                      VkAccessFlags signalAccessMask);
};


