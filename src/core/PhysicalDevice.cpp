#include<core\PhysicalDevice.h>
#include<stdexcept>
#include<core\Image.h>
#include<core\CoreUtils.h>

PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle)
: _handle(handle)
{
    if (VKHANDLE_IS_NULL(handle))
        throw std::invalid_argument("Create Physical device with invalid handle!");

    vkGetPhysicalDeviceFeatures(handle, &_features);
    vkGetPhysicalDeviceProperties(handle, &_generalProps);
    vkGetPhysicalDeviceMemoryProperties(handle, &_memoryProps);
    uint32_t queueFamilyCnt = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCnt, nullptr);
    _queueFamilyProps.resize(queueFamilyCnt);
    vkGetPhysicalDeviceQueueFamilyProperties(handle, &queueFamilyCnt, _queueFamilyProps.data());

}



bool PhysicalDevice::GetFormatProperty(VkFormat fmt, VkFormatProperties* result) const
{
   vkGetPhysicalDeviceFormatProperties(_handle, fmt, result);
   return true;
}


bool PhysicalDevice::GetImageFormatProperty(VkFormat fmt, const Image* image, VkImageFormatProperties* result) const
{
    ImageDesc imgDesc = image->GetDesc();
    return VKCALL_SUCCESS(vkGetPhysicalDeviceImageFormatProperties(_handle, fmt,  
            image->GetType(),
            imgDesc.linearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL,
            imgDesc.usageFlags,
            imgDesc.flags,
            result));
}


 bool PhysicalDevice::GetSurfaceCapability(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* result) const
 {
    return VKCALL_SUCCESS(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_handle, surface, result));
 }


 bool PhysicalDevice::GetSurfaceFormats(VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>& result) const
 {
    result.clear();
    uint32_t fmtCnt = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(_handle, surface, &fmtCnt, nullptr);
    result.resize(fmtCnt);
    return VKCALL_SUCCESS(vkGetPhysicalDeviceSurfaceFormatsKHR(_handle, surface, &fmtCnt, result.data()));
 }


 bool PhysicalDevice::GetSurfacePresentModes(VkSurfaceKHR surface, std::vector<VkPresentModeKHR> result) const
 {
    result.clear();
    uint32_t presentModeCnt = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(_handle, surface, &presentModeCnt, nullptr);
    result.resize(presentModeCnt);
    return VKCALL_SUCCESS(vkGetPhysicalDeviceSurfacePresentModesKHR(_handle, surface, &presentModeCnt, result.data()));
 }