#pragma once
#include<core\CoreUtils.h>
#include<vector>

class Image;

class PhysicalDevice
{
private:
    VkPhysicalDevice _handle{VK_NULL_HANDLE};
    VkPhysicalDeviceFeatures _features{};
    VkPhysicalDeviceProperties _generalProps{};
    VkPhysicalDeviceMemoryProperties _memoryProps{};
    std::vector<VkQueueFamilyProperties> _queueFamilyProps{};
    

public:
    PhysicalDevice(VkPhysicalDevice deviceHanle);
    ~PhysicalDevice();

    NONE_COPYABLE_NONE_MOVEABLE(PhysicalDevice)
    VkPhysicalDevice GetHandle() const { return _handle; }
    const VkPhysicalDeviceFeatures& GetFeatures() const { return _features; }
    const VkPhysicalDeviceProperties& GetGeneralProperty() const { return _generalProps; }
    const VkPhysicalDeviceMemoryProperties& GetMemotyProperty() const { return _memoryProps; }
    size_t GetQueueFamilyCount() const { return _queueFamilyProps.size(); }
    const VkQueueFamilyProperties& GetQueueFamilyProperty(size_t queueFamilyIdx) const { return _queueFamilyProps[queueFamilyIdx]; }

    bool GetFormatProperty(VkFormat fmt, VkFormatProperties* result) const;
    bool GetImageFormatProperty(VkFormat fmt, const Image* image, VkImageFormatProperties* result) const;
    bool GetSurfaceCapability(VkSurfaceKHR surface, VkSurfaceCapabilitiesKHR* result) const;
    bool GetSurfaceFormats(VkSurfaceKHR surface, std::vector<VkSurfaceFormatKHR>& result) const;
    bool GetSurfacePresentModes(VkSurfaceKHR surface, std::vector<VkPresentModeKHR> result) const;
};
