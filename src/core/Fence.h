#pragma once
#include<vulkan\vulkan.h>
#include<numeric>
#include"core\CoreUtils.h"



class Device;

class Fence
{
private:
    VkFence _vkFence{VK_NULL_HANDLE};
    Device* _pDevice{nullptr};

public:
    Fence(Device* pDevice, bool signaled = false);
    ~Fence() { Release(); };

    NONE_COPYABLE(Fence)

    Fence( Fence&& rval);
    Fence& operator = (Fence&& rval);


    VkFence GetHandle() const { return _vkFence; }
    Device* GetDevice() const { return _pDevice; } 

    void Reset() const;
    void Wait(uint64_t timeOut = std::numeric_limits<uint64_t>::max()) const;
    bool IsSignaled() const;

    bool IsValid() const { return _pDevice != nullptr && VKHANDLE_IS_NOT_NULL(_vkFence); }
    void Release();

};


