#pragma once
#include<vulkan\vulkan.h>
#include<numeric>
#include"core\CoreUtils.h"
#include"core\VKDeviceResource.h"



class Device;

class Fence : public VKDeviceResource
{
    friend class Device;

private:
    VkFence _vkFence{VK_NULL_HANDLE};

    bool _create(bool signaled);
    void Release() override;

public:
    Fence(Device* pDevice = nullptr);
    ~Fence() { assert(!IsValid()); };

    NONE_COPYABLE_NONE_MOVEABLE(Fence)

    VkFence GetHandle() const { return _vkFence; }
    void Reset() const;
    void Wait(uint64_t timeOut = std::numeric_limits<uint64_t>::max()) const;
    bool IsSignaled() const;
    bool IsValid() const override { return  VKHANDLE_IS_NOT_NULL(_vkFence); }
};


