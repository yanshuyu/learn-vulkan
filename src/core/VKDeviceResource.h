#pragma once
#include"core\CoreUtils.h" 

class Device;

class VKDeviceResource
{
protected:
    Device* _pDevice;


public:
    VKDeviceResource(Device* pDevice);
    virtual ~VKDeviceResource() {};


    virtual void Release() = 0;
    virtual bool IsValid() const = 0;

    Device* GetDevice() const { return _pDevice; }
};

