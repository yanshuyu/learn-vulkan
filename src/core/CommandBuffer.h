#pragma once
#include<vulkan\vulkan.h>
#include"core\CoreUtils.h"

class Device;

class CommandBuffer
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
    Device* _pDevice {nullptr};
    VkCommandPool _vkCmdPool{VK_NULL_HANDLE};
    VkQueue _vkQueue{VK_NULL_HANDLE};
    VkCommandBuffer _vkCmdBuffer{VK_NULL_HANDLE};
    bool _temprary{true};
    State _state { State::Invalid};

    void SetUp(Device* pDevice, VkCommandPool cmdPool, VkQueue exeQueue, VkCommandBuffer cmdBuf, bool isTemp);
    void ClenUp();

public:
    CommandBuffer(){};
    ~CommandBuffer();

    NONE_COPYABLE_NONE_MOVEABLE(CommandBuffer)

    Device* GetDevice() const { return _pDevice; }
    VkCommandPool GetPoolHandle() const { return _vkCmdPool; }
    VkCommandBuffer GetHandle() const { return _vkCmdBuffer; }
    VkQueue GetQueueHandle() const { return _vkQueue; }
    
    bool IsTemprary() const { return _temprary; }
    bool IsVaild() const;
    bool CanExecute() const { return IsVaild() && !_temprary && _state == State::Executable; }
    bool Begin();
    bool End();
    bool Execute();
    bool Reset();
};


