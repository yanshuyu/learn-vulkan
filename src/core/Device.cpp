#include"core\Device.h"
#include<iostream>
#include<algorithm>
#include<map>
#include<utility>
#include<set>
#include"core\CoreUtils.h"
#include"core\CommandBuffer.h"
#include"core\Buffer.h"
#include"core\Fence.h"

Device* Device::sActive = nullptr;

Device::Device()
{
    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT; i++)
    {
        m_DeviceQueueCommandPools[i*2] = VK_NULL_HANDLE;
        m_DeviceQueueCommandPools[i*2+1] = VK_NULL_HANDLE;
        m_DeviceQueueCmdBuffers[i] = VK_NULL_HANDLE;
    }
    
}

 bool Device::Initailze(VkPhysicalDevice phyDevice, VkSurfaceKHR presentSurface)
 {
    if (IsValid())
        return true;

    if (!AllHardWareFeatureSupported(phyDevice))
    {
        LOGE("--> Not All Physical Device Features Are Supported!");
        return false;
    }

    bool ok = CreateLogicalDevice(phyDevice);

    LOGI("-->Create Device: {}", ok);
    if (ok)
    {
        QueryDeviceMemoryProperties();
        vkGetDeviceQueue(m_vkDevice, m_DeviceQueueFamilyIndices.GrapicQueueFamilyIndex(), 0, &m_DeviceGraphicQueue);
        vkGetDeviceQueue(m_vkDevice, m_DeviceQueueFamilyIndices.TransferQueueFamilyIndex(), 0, &m_DeviceTransferQueue);
        if (m_DeviceQueueFamilyIndices.ComputeQueueFamilyIndex() != -1)
            vkGetDeviceQueue(m_vkDevice, m_DeviceQueueFamilyIndices.ComputeQueueFamilyIndex(), 0, &m_DeviceComputeQueue);
        if (presentSurface != VK_NULL_HANDLE)
        {   
            m_PresentQueueFamilyIndex = m_DeviceQueueFamilyIndices.PresentQueueFamilyIndex(presentSurface);
            vkGetDeviceQueue(m_vkDevice, m_PresentQueueFamilyIndex, 0, &m_DevicePresentQueue);
        }
        ok &= CreateCommandPools();
        ok &= CreateCommandBuffers();
    }

    return ok;
 }

void Device::Release()
{
    // destory command pool will destroy command buffers allocate from it
    std::set<VkCommandPool> compactCmdPools;
    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT*2; i++)
    {
        if (m_DeviceQueueCommandPools[i] != VK_NULL_HANDLE)
            compactCmdPools.insert(m_DeviceQueueCommandPools[i]);
    }
    for (auto &&pool : compactCmdPools)
    {
        vkDestroyCommandPool(m_vkDevice, pool, nullptr);
    }


    if (m_vkDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_vkDevice); // ensure all job has finish before destroy any device
        vkDestroyDevice(m_vkDevice, nullptr);
        m_vkDevice = VK_NULL_HANDLE;
        m_DeviceGraphicQueue = VK_NULL_HANDLE; // destroy deivce will automatically destroy it's created queues
        m_DevicePresentQueue = VK_NULL_HANDLE;
        m_DeviceComputeQueue = VK_NULL_HANDLE;
        m_DeviceTransferQueue = VK_NULL_HANDLE;
        m_PresentQueueFamilyIndex = -1;

        if (sActive == this)
            sActive = nullptr;
    }


    ResetAllHints();
}


void Device::WaitIdle() const
{
    if (IsValid())
        vkDeviceWaitIdle(m_vkDevice);
}

CommandBuffer Device::GetCommandBuffer(DeviceJobOperation op)
{
    int idx = GetOperationQueueFamilyIndex(op);
    if (idx == -1)
        return std::move(CommandBuffer(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, true));
    
    return std::move(CommandBuffer(m_vkDevice, m_DeviceQueueCommandPools[idx * 2], m_DeviceQueueCmdBuffers[idx], false));
}


CommandBuffer Device::GetTempraryCommandBuffer(DeviceJobOperation op)
{
    int idx = GetOperationQueueFamilyIndex(op);
    if (idx == -1)
        return std::move(CommandBuffer(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, true));

    // alloc one temprary command buffer
    VkCommandBufferAllocateInfo cmdBufAllocInfo{};
    cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdBufAllocInfo.pNext = nullptr;
    cmdBufAllocInfo.commandPool = m_DeviceQueueCommandPools[idx * 2 + 1];
    cmdBufAllocInfo.commandBufferCount = 1;
    cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VkCommandBuffer allocedCmdBuffer = VK_NULL_HANDLE;
    VkResult result = vkAllocateCommandBuffers(m_vkDevice, &cmdBufAllocInfo, &allocedCmdBuffer);
    if (result != VK_SUCCESS)
    {
        std::cout << "--> Allocate Temprary Command Buffer Failed! vulkan error: " << result << std::endl;
        return std::move(CommandBuffer(VK_NULL_HANDLE, VK_NULL_HANDLE, VK_NULL_HANDLE, true));
    }

    return std::move(CommandBuffer(m_vkDevice, m_DeviceQueueCommandPools[idx * 2 + 1], allocedCmdBuffer, true));
}

void Device::ResetAllHints()
{
    m_DeviceExtendsions.clear();
    m_EnablePhyDeviceFeatures.clear();
    m_PresentQueueFamilyIndex = -1;
}

void Device::SetDeviceFeatureHint(HardwareFeature feature, bool enabled)
{
    auto pos = std::find(m_EnablePhyDeviceFeatures.begin(), m_EnablePhyDeviceFeatures.end(), feature);
    if (!enabled && pos != m_EnablePhyDeviceFeatures.end())
    {
        m_EnablePhyDeviceFeatures.erase(pos);
    }
    else if (enabled && pos == m_EnablePhyDeviceFeatures.end())
    {
        m_EnablePhyDeviceFeatures.push_back(feature);
    }
}


bool Device::CreateLogicalDevice(VkPhysicalDevice phyDevice)
{
    // Check All Device Extendions Are Supported
    uint32_t deviceSupportedExtendsionCnt = 0;
    vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &deviceSupportedExtendsionCnt, nullptr);
    std::vector<VkExtensionProperties> deviceSupportedExtendsionProps(deviceSupportedExtendsionCnt);
    vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &deviceSupportedExtendsionCnt, deviceSupportedExtendsionProps.data());
    LOGI("--> Detecte Device Supported Extendsions: {}", deviceSupportedExtendsionCnt); 
    for (const auto& extProp : deviceSupportedExtendsionProps)
    {
        LOGI("\t{}\t{}",extProp.extensionName, extProp.specVersion);
    }

    LOGI("--> Enabled Device Extendsions: {}", m_DeviceExtendsions.size());
    for (const auto& ext : m_DeviceExtendsions)
    {
        LOGI("\t{}", ext);
    }
    
    for (const auto& ext : m_DeviceExtendsions)
    {
        auto pos = std::find_if(deviceSupportedExtendsionProps.begin(), deviceSupportedExtendsionProps.end(), [&](const VkExtensionProperties& extProp){
            return strcmp(ext.c_str(), extProp.extensionName) == 0;
        });

        if (pos == deviceSupportedExtendsionProps.end())
        {
            LOGE("-->Extendsion \"{}\" Is Not Supported By Device!", ext);
            return false;
        }  
    }

    // create logical device
    m_DeviceQueueFamilyIndices.Query(phyDevice);
    auto compactQueueFamilyIndices = m_DeviceQueueFamilyIndices.UniqueQueueFamilyIndices();
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(compactQueueFamilyIndices.size());
    float defaultQueuePriorty = 1;
    size_t i = 0;
    for (auto& idx : compactQueueFamilyIndices)
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = nullptr;
        queueCreateInfos[i].queueFamilyIndex = idx;
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &defaultQueuePriorty;
        i++;
    }

    std::vector<const char*> enableExtendsionNames(m_DeviceExtendsions.size());
    for (size_t i = 0; i < m_DeviceExtendsions.size(); i++)
    {
        enableExtendsionNames[i] = m_DeviceExtendsions[i].c_str();
    }
    
    VkPhysicalDeviceFeatures enableFeatures{};
    if (m_EnablePhyDeviceFeatures.size() > 0)
        enableFeatures = HardwareFeaturesToVkPhysicalDeviceFeatures();
    
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr; // depcrecated
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = enableExtendsionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = enableExtendsionNames.data();
    deviceCreateInfo.pEnabledFeatures = m_EnablePhyDeviceFeatures.size() > 0 ? &enableFeatures : nullptr;

    VkDevice createdDevice = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(phyDevice, &deviceCreateInfo, nullptr, &createdDevice);
    if (result == VK_SUCCESS)
    {
        m_vkDevice = createdDevice;
        m_vkPhyDevice = phyDevice;
    }
    else 
    {
        LOGE("--> Create vulkan device error: {}", result);
    }

    return result == VK_SUCCESS;
}

bool Device::CreateCommandPools()
{
    std::map<int, std::pair<VkCommandPool, VkCommandPool>> createdQueueFamilyCommandPools{};
    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT; i++)
    {
        int curQueueFamilyIndex = m_DeviceQueueFamilyIndices.QueueFamilyIndexAtIndex(i);
        if (curQueueFamilyIndex != -1) 
        {
            auto pos = createdQueueFamilyCommandPools.find(curQueueFamilyIndex);
            // create 2 command pool for this type of queue family (grapics / compute / transfer)
            // one for shot time lives
            // one for long time use, can reset to record again
            if (pos == createdQueueFamilyCommandPools.end())
            {
                VkCommandPoolCreateInfo poolCreateInfo{};
                poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
                poolCreateInfo.pNext = nullptr;
                poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
                poolCreateInfo.queueFamilyIndex = curQueueFamilyIndex;

                VkCommandPool longLivePool = VK_NULL_HANDLE;
                VkResult result = vkCreateCommandPool(m_vkDevice, &poolCreateInfo, nullptr, &longLivePool);
                if (result != VK_SUCCESS)
                {
                    std::cout << "--> Create Command long live Pool For Queue Family Index: " << curQueueFamilyIndex << ", vulkan error: " << result << std::endl;
                    return false;
                }
                m_DeviceQueueCommandPools[i*2] = longLivePool;
                
                poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                VkCommandPool shotTimePool = VK_NULL_HANDLE;
                result = vkCreateCommandPool(m_vkDevice, &poolCreateInfo, nullptr, &shotTimePool);
                if (result != VK_SUCCESS)
                {
                    std::cout << "--> Create Command shot time Pool For Queue Family Index: " << curQueueFamilyIndex << ", vulkan error: " << result << std::endl;
                    return false;
                }
                m_DeviceQueueCommandPools[i*2+1] = shotTimePool;

                createdQueueFamilyCommandPools[curQueueFamilyIndex] = std::make_pair(longLivePool, shotTimePool);

            }
            else 
            {
                m_DeviceQueueCommandPools[i*2] = createdQueueFamilyCommandPools[curQueueFamilyIndex].first;
                m_DeviceQueueCommandPools[i*2+1] = createdQueueFamilyCommandPools[curQueueFamilyIndex].second;   
            }
        }
    }

    return true;
    
}

bool Device::CreateCommandBuffers()
{
    std::map<int, VkCommandBuffer> createdQueueCommanBuffers{};
    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT; i++)
    {
        int curQueueFamilyIndex = m_DeviceQueueFamilyIndices.QueueFamilyIndexAtIndex(i);
        if ( curQueueFamilyIndex != -1)
        {
            auto pos = createdQueueCommanBuffers.find(curQueueFamilyIndex);
            if (pos == createdQueueCommanBuffers.end())
            {
                VkCommandBufferAllocateInfo cmdBufferAllocInfo{};
                cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
                cmdBufferAllocInfo.pNext = nullptr;
                cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
                cmdBufferAllocInfo.commandBufferCount = 1;
                cmdBufferAllocInfo.commandPool = m_DeviceQueueCommandPools[i * 2];
                VkCommandBuffer createdCmdBuffer = VK_NULL_HANDLE;
                VkResult result = vkAllocateCommandBuffers(m_vkDevice, &cmdBufferAllocInfo, &createdCmdBuffer);
                if (result != VK_SUCCESS)
                {
                    std::cout << "--> Alloc Command Buffer Failed for queue family: " << curQueueFamilyIndex << ", vulkan error: " << result << std::endl;
                    return false;
                }

                m_DeviceQueueCmdBuffers[i] = createdCmdBuffer;
                createdQueueCommanBuffers[curQueueFamilyIndex] = createdCmdBuffer;
            }
            else
            {
                m_DeviceQueueCmdBuffers[i] = pos->second;
            }
        }
    }
    
}

void Device::QueryDeviceMemoryProperties()
{
    vkGetPhysicalDeviceMemoryProperties(m_vkPhyDevice, &m_PhyDeviceMemProps);
}

bool Device::AllHardWareFeatureSupported(VkPhysicalDevice phyDevice) const
{
    VkPhysicalDeviceFeatures phyDevicefeatures;
    vkGetPhysicalDeviceFeatures(phyDevice, &phyDevicefeatures);
    for (size_t i = 0; i < m_EnablePhyDeviceFeatures.size(); i++)
    {
        HardwareFeature feature = m_EnablePhyDeviceFeatures[i];
        switch (feature)
        {
        case HardwareFeature::geometryShader:
            if (!phyDevicefeatures.geometryShader)
                return false;
            break;
        case HardwareFeature::tessellationShader:
            if (!phyDevicefeatures.tessellationShader)
                return false;
            break;
        case HardwareFeature::samplerAnisotropy:
            if (!phyDevicefeatures.samplerAnisotropy)
                return false;
            break;
        case HardwareFeature::textureCompressionETC2:
            if (!phyDevicefeatures.textureCompressionETC2)
                return false;
            break;

        default:
            break;
        }
    }

    return true;
}

bool Device::AllocMemory(uint32_t memTypeBits, VkMemoryPropertyFlags memPropFlag, VkDeviceSize size, VkDeviceMemory* pAllocatedMem)
{
    if (!IsValid())
        return false;

    if (m_PhyDeviceMemProps.memoryHeapCount == 0 || m_PhyDeviceMemProps.memoryTypeCount == 0)
        QueryDeviceMemoryProperties();

    size_t memTypeIdx = -1;
    for (size_t i = 0; i < m_PhyDeviceMemProps.memoryTypeCount; i++)
    {
        if (memTypeBits & (1 << i)  > 0 // avaliable memory type index
            && (m_PhyDeviceMemProps.memoryTypes[i].propertyFlags & memPropFlag) == memPropFlag) // avaliable memory properties
        {
            memTypeIdx = i;
            break;
        } 
    }

    if (memTypeIdx == -1)
        return false;

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = memTypeIdx;
    allocInfo.allocationSize = size;

    VkResult result = vkAllocateMemory(m_vkDevice, &allocInfo, nullptr, pAllocatedMem);
    if (result != VK_SUCCESS)
        LOGE("Vulkan Alloc Memory error: {}", result);
    
    return result == VK_SUCCESS;
}

void Device::FreeMemory(VkDeviceMemory mem)
{
    if (!IsValid())
        return;

    vkFreeMemory(m_vkDevice, mem, nullptr);
}


Buffer* Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp)
{
    if (!IsValid())
    {
        LOGW("Try to create Buffer with invalid Device({})!", (void *)this);
        return nullptr;
    }

    Buffer *newBuffer = new Buffer{};
    if (!newBuffer->Initailize(this, size, usage, memProp))
    {
        delete newBuffer;
        return nullptr;
    }

    _BuffersRes.push_back(newBuffer);

    return newBuffer;
}

bool Device::DestroyBuffer(Buffer* pBuffer)
{
    auto pos = std::find(_BuffersRes.begin(), _BuffersRes.end(), pBuffer);
    if (pos == _BuffersRes.end())
        return false;
    
    _BuffersRes.erase(pos);
    pBuffer->Release();
    delete pBuffer;

    return true;
}

Fence* Device::CreateFence(bool signaled)
{
    if (!IsValid())
    {
        LOGW("Try to create Fence with invalid Device({})!", (void *)this);
        return nullptr;
    }

    Fence* pFence = new Fence(this, signaled);
    _Fences.push_back(pFence);
    return pFence;
}

bool Device::DestroyFence(Fence* pFence)
{
    auto pos = std::find(_Fences.begin(), _Fences.end(), pFence);
    if (pos == _Fences.end())
        return false;
    
    _Fences.erase(pos);
    delete pFence;

    return true;
}

VkPhysicalDeviceFeatures Device::HardwareFeaturesToVkPhysicalDeviceFeatures() const
{
    VkPhysicalDeviceFeatures phyDeviceFeatures{};
    for (size_t i = 0; i < m_EnablePhyDeviceFeatures.size(); i++)
    {
        HardwareFeature feature = m_EnablePhyDeviceFeatures[i];
        switch (feature)
        {
        case HardwareFeature::geometryShader:
            phyDeviceFeatures.geometryShader = VK_TRUE;
            break;
        case HardwareFeature::tessellationShader:
            phyDeviceFeatures.tessellationShader = VK_TRUE;
            break;
        case HardwareFeature::samplerAnisotropy:
            phyDeviceFeatures.samplerAnisotropy = VK_TRUE;
            break;
        case HardwareFeature::textureCompressionETC2:
            phyDeviceFeatures.textureCompressionETC2 = VK_TRUE;
            break;

        default:
            break;
        }
    }

    return phyDeviceFeatures;
}

 int Device::GetOperationQueueFamilyIndex(DeviceJobOperation op)
 {
    int idx = -1;
    switch (op)
    {
    case grapic:
        idx = QUEUE_FAMILY_GRAPICS_INDEX;
        break;
    case compute:
        idx = QUEUE_FAMILY_COMPUTE_INDEX;
    case transfer:
        idx = QUEUE_FAMILY_TRANSFER_INDEX;
    case present:
        if (m_DevicePresentQueue != VK_NULL_HANDLE)
            idx = m_PresentQueueFamilyIndex;
        break;

    default:
        break;
    }

    return idx;
 }
