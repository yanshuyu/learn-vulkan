#include"core\QueueFamilyIndices.h"	
#include<algorithm>

QueueFamilyIndices::QueueFamilyIndices()
{
    std::fill_n(m_QueueFamilyIndices.begin(), MAX_INDEX, -1);
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
    std::fill_n(m_QueueFamilyIndices.begin(), MAX_INDEX, -1);
    m_QueueFamilyProperties.clear();
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
    m_QueueFamilyProperties.resize(queueFamilyCnt);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCnt, m_QueueFamilyProperties.data());
    
    for (int i = 0; i < queueFamilyCnt; i++)
    {
        if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && m_QueueFamilyIndices[GRAPICS_INDEX] == -1)
            m_QueueFamilyIndices[GRAPICS_INDEX] = i;
        if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT && m_QueueFamilyIndices[COMPUTE_INDEX] == -1)
            m_QueueFamilyIndices[COMPUTE_INDEX] = i;
        if (m_QueueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT && m_QueueFamilyIndices[TRANSFER_INDEX] == -1)
            m_QueueFamilyIndices[TRANSFER_INDEX] = i;
    }
    

    m_pPhysicalDevice = device != nullptr ? device : m_pPhysicalDevice;
    m_IsQueryed = true;
}


int QueueFamilyIndices::PresentQueueFamilyIndex(VkSurfaceKHR surface) const
{
    if (surface == nullptr || !m_IsQueryed)
        return -1;

    for (size_t i=0; i<QueueFamilyCount(); i++)
    {
        VkBool32 result = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(m_pPhysicalDevice, i, surface, &result);
        if (result)
            return i;
    }
    
    return -1;
}


VkQueueFlags QueueFamilyIndices::CombindQueueFamilyFlags() const 
{
    VkQueueFlags result = 0;
    if (m_QueueFamilyIndices[GRAPICS_INDEX] != -1)
        result |= VK_QUEUE_GRAPHICS_BIT;
    if (m_QueueFamilyIndices[COMPUTE_INDEX] != -1)
        result |= VK_QUEUE_COMPUTE_BIT;
    if (m_QueueFamilyIndices[TRANSFER_INDEX] != -1)
        result |= VK_QUEUE_TRANSFER_BIT;

    return result;
}


std::set<int> QueueFamilyIndices::UniqueQueueFamilyIndices() const
{

    std::set<int> uniqueIndices{};
    for (size_t i = 0; i < MAX_INDEX; i++)
    {
        if (m_QueueFamilyIndices[i] != -1)
            uniqueIndices.insert(uniqueIndices.end(), m_QueueFamilyIndices[i]);
    }

    return uniqueIndices;
    
}