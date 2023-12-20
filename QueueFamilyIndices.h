#pragma once
#include<vulkan\vulkan.h>
#include<array>
#include<set>

#define QUEUE_FAMILY_MAX_COUNT 3
#define QUEUE_FAMILY_GRAPICS_INDEX 0
#define QUEUE_FAMILY_COMPUTE_INDEX 1
#define QUEUE_FAMILY_TRANSFER_INDEX 2

struct QueueFamilyIndices
{
private:
    VkPhysicalDevice m_pPhysicalDevice;
    std::array<int, QUEUE_FAMILY_MAX_COUNT> m_QueueFamilyIndices;
    bool m_IsQueryed;

 public:
    QueueFamilyIndices();
    QueueFamilyIndices(VkPhysicalDevice device);
    void Query(VkPhysicalDevice device = nullptr);
    void Reset();
    VkQueueFlags CombindQueueFamilyFlags() const;
    std::set<int> UniqueQueueFamilyIndices() const;

    int GrapicQueueFamilyIndex() const 
    {
        return m_QueueFamilyIndices[QUEUE_FAMILY_GRAPICS_INDEX];
    }

    int ComputeQueueFamilyIndex() const
    {
        return m_QueueFamilyIndices[QUEUE_FAMILY_COMPUTE_INDEX];
    }

    int TransferQueueFamilyIndex() const 
    {
        return m_QueueFamilyIndices[QUEUE_FAMILY_TRANSFER_INDEX];
    }

    int PresentQueueFamilyIndex(VkSurfaceKHR surface) const;

    bool IsPresentSupported(VkSurfaceKHR surface) const 
    {
        return PresentQueueFamilyIndex(surface) != -1;
    }


    bool IsQueryed() const
    {
        return m_IsQueryed;
    }

    VkPhysicalDevice QueryedDevice() const 
    {
        return m_pPhysicalDevice;
    }

};
