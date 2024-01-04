#include"core\QueueFamilyIndices.h"	
#include<vector>
#include<algorithm>

QueueFamilyIndices::QueueFamilyIndices()
{
    m_pPhysicalDevice =nullptr;
    m_IsQueryed = false;
    std::fill_n(m_QueueFamilyIndices.begin(), QUEUE_FAMILY_MAX_COUNT, -1);
}


QueueFamilyIndices::QueueFamilyIndices(VkPhysicalDevice device)
: QueueFamilyIndices()
{
    Query(device);
}


void QueueFamilyIndices::Reset()
{
    m_pPhysicalDevice = nullptr;
    m_IsQueryed = false;
    std::fill_n(m_QueueFamilyIndices.begin(), QUEUE_FAMILY_MAX_COUNT, -1);
}


void QueueFamilyIndices::Query(VkPhysicalDevice device)
{
    if (device == m_pPhysicalDevice && m_IsQueryed)
        return;

    if (device == nullptr && !m_IsQueryed) // use stored device
        device = m_pPhysicalDevice;

    if (device == nullptr)
        return;
    
    uint32_t queueFamilyCnt = 0; 
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCnt, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCnt);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCnt, queueFamilyProps.data());
    
    for (int i = 0; i < queueFamilyCnt; i++)
    {
        if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && m_QueueFamilyIndices[QUEUE_FAMILY_GRAPICS_INDEX] == -1)
            m_QueueFamilyIndices[QUEUE_FAMILY_GRAPICS_INDEX] = i;
        if (queueFamilyProps[i].queueFlags & VK_QUEUE_COMPUTE_BIT && m_QueueFamilyIndices[QUEUE_FAMILY_COMPUTE_INDEX] == -1)
            m_QueueFamilyIndices[QUEUE_FAMILY_COMPUTE_INDEX] = i;
        if (queueFamilyProps[i].queueFlags & VK_QUEUE_TRANSFER_BIT && m_QueueFamilyIndices[QUEUE_FAMILY_TRANSFER_INDEX] == -1)
            m_QueueFamilyIndices[QUEUE_FAMILY_TRANSFER_INDEX] = i;
    }
    

    m_pPhysicalDevice = device != nullptr ? device : m_pPhysicalDevice;
    m_IsQueryed = true;
}


int QueueFamilyIndices::PresentQueueFamilyIndex(VkSurfaceKHR surface) const
{
    if (surface == nullptr || !m_IsQueryed)
        return -1;

    for (auto idx : m_QueueFamilyIndices)
    {
        if (idx == -1)
            continue;
        
        VkBool32 result = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_pPhysicalDevice, idx, surface, &result);
        if (result)
            return idx;
    }
    
    return -1;
}


VkQueueFlags QueueFamilyIndices::CombindQueueFamilyFlags() const 
{
    VkQueueFlags result = 0;
    if (m_QueueFamilyIndices[QUEUE_FAMILY_GRAPICS_INDEX] != -1)
        result |= VK_QUEUE_GRAPHICS_BIT;
    if (m_QueueFamilyIndices[QUEUE_FAMILY_COMPUTE_INDEX] != -1)
        result |= VK_QUEUE_COMPUTE_BIT;
    if (m_QueueFamilyIndices[QUEUE_FAMILY_TRANSFER_INDEX] != -1)
        result |= VK_QUEUE_TRANSFER_BIT;

    return result;
}


std::set<int> QueueFamilyIndices::UniqueQueueFamilyIndices() const
{

    std::set<int> uniqueIndices{};
    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT; i++)
    {
        if (m_QueueFamilyIndices[i] != -1)
            uniqueIndices.insert(uniqueIndices.end(), m_QueueFamilyIndices[i]);
    }

    return uniqueIndices;
    
}