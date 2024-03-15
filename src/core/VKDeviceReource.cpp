#include"VKDeviceResource.h"
#include"Device.h"


VKDeviceResource::VKDeviceResource(Device* pDevice)
: _pDevice(pDevice)
{
    assert(_pDevice && _pDevice->IsValid());
}
