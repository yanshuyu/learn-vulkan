#pragma once
#include<vulkan\vulkan.h>
#include<array>
#include<set>
#include<vector>

class QueueFamilyIndices
{
public:
    enum Key 
    {
        GRAPICS_INDEX,
        COMPUTE_INDEX,
        TRANSFER_INDEX,
        MAX_INDEX,
    };

private:
    VkPhysicalDevice m_pPhysicalDevice{VK_NULL_HANDLE};
    std::array<int, Key::MAX_INDEX> m_QueueFamilyIndices{};
    std::vector<VkQueueFamilyProperties> m_QueueFamilyProperties{};
    bool m_IsQueryed{false};

 public:
    QueueFamilyIndices();
    QueueFamilyIndices(VkPhysicalDevice device);
    ~QueueFamilyIndices() = default;

    void Query(VkPhysicalDevice device = nullptr);
    void Reset();
    VkQueueFlags CombindQueueFamilyFlags() const;
    std::set<int> UniqueQueueFamilyIndices() const;

    int GrapicQueueFamilyIndex() const 
    {
        return m_QueueFamilyIndices[GRAPICS_INDEX];
    }

    int ComputeQueueFamilyIndex() const
    {
        return m_QueueFamilyIndices[COMPUTE_INDEX];
    }

    int TransferQueueFamilyIndex() const 
    {
        return m_QueueFamilyIndices[TRANSFER_INDEX];
    }

    int QueueFamilyIndex(Key key) const { return m_QueueFamilyIndices[key]; }

    int PresentQueueFamilyIndex(VkSurfaceKHR surface) const;

    bool IsPresentSupported(VkSurfaceKHR surface) const 
    {
        return PresentQueueFamilyIndex(surface) != -1;
    }

    int QueueFamilyCount() const { return m_QueueFamilyProperties.size(); }

    VkQueueFamilyProperties QueueFamilyProperties(int queueFamilyIndex) const { return m_QueueFamilyProperties[queueFamilyIndex]; }

    bool IsQueryed() const
    {
        return m_IsQueryed;
    }

    VkPhysicalDevice QueryedDevice() const 
    {
        return m_pPhysicalDevice;
    }

};
