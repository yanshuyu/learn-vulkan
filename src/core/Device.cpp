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
#include"rendering\Window.h"


Device::Device()
{
    for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX; i++)
    {
        m_DeviceQueues[i] = VK_NULL_HANDLE;
        m_DeviceQueueCmdPools[i*2] = VK_NULL_HANDLE;
        m_DeviceQueueCmdPools[i*2+1] = VK_NULL_HANDLE;
        //m_DeviceQueueCmdBuffers[i] = VK_NULL_HANDLE;
    }
    
}

 bool Device::Initailze(VkPhysicalDevice phyDevice)
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
        for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX; i++)
        {
            int queueFamIdx = m_DeviceQueueFamilyIndices.QueueFamilyIndex((QueueFamilyIndices::Key)i);
            if (queueFamIdx != -1)
                vkGetDeviceQueue(m_vkDevice, queueFamIdx, 0, &m_DeviceQueues[i]);
        }
        
        QueryDeviceMemoryProperties();
        
        ok &= CreateCommandPools();
        //ok &= CreateCommandBuffers();
    }

    return ok;
 }


void Device::Release()
{
    if (!IsValid())
        return;

    WaitIdle();

    // destory command pool will destroy command buffers allocate from it
    std::set<VkCommandPool> compactCmdPools;
    for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX*2; i++)
    {
        if (m_DeviceQueueCmdPools[i] != VK_NULL_HANDLE)
            compactCmdPools.insert(m_DeviceQueueCmdPools[i]);
    }
    for (auto &&pool : compactCmdPools)
    {
        vkDestroyCommandPool(m_vkDevice, pool, nullptr);
    }

    //vkDeviceWaitIdle(m_vkDevice); // ensure all job has finish before destroy any device
    vkDestroyDevice(m_vkDevice, nullptr); // destroy deivce will automatically destroy it's created queues
    m_vkDevice = VK_NULL_HANDLE;

    for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX; i++)
    {
        m_DeviceQueues[i] = VK_NULL_HANDLE;
        m_DeviceQueueCmdPools[i*2] = VK_NULL_HANDLE;
        m_DeviceQueueCmdPools[i*2+1] = VK_NULL_HANDLE;
       // m_DeviceQueueCmdBuffers[i] = VK_NULL_HANDLE;
    }

    m_vkPhyDevice = VK_NULL_HANDLE;
    m_DeviceQueueFamilyIndices.Reset();

    ResetAllHints();
}


void Device::WaitIdle() const
{
    if (IsValid())
        vkDeviceWaitIdle(m_vkDevice);
}




bool Device::DestroyCommandBuffer(CommandBuffer* pCmdBuf)
{
    if (pCmdBuf == nullptr)
        return false;
    
    if (pCmdBuf->IsVaild())
        return false;

    if (pCmdBuf->GetDevice() != this)
    {
        LOGW("Try to release a command buffer({}) create by device({}) with diffrence device({})!", (void*)pCmdBuf, (void*)pCmdBuf->GetDevice(), (void*)this);
        return false;
    }

    VkCommandBuffer cmdbuf = pCmdBuf->GetHandle();
    vkFreeCommandBuffers(m_vkDevice, pCmdBuf->GetPoolHandle(), 1, &cmdbuf);
    pCmdBuf->ClenUp();
    _CmdBufPool.Return(pCmdBuf);

    return true;
}


void Device::ResetAllHints()
{
    m_DeviceExtendsions.clear();
    m_EnablePhyDeviceFeatures.clear();
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
    for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX; i++)
    {
        int curQueueFamilyIndex = m_DeviceQueueFamilyIndices.QueueFamilyIndex((QueueFamilyIndices::Key)i);
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
                m_DeviceQueueCmdPools[i*2] = longLivePool;
                
                poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                VkCommandPool shotTimePool = VK_NULL_HANDLE;
                result = vkCreateCommandPool(m_vkDevice, &poolCreateInfo, nullptr, &shotTimePool);
                if (result != VK_SUCCESS)
                {
                    std::cout << "--> Create Command shot time Pool For Queue Family Index: " << curQueueFamilyIndex << ", vulkan error: " << result << std::endl;
                    return false;
                }
                m_DeviceQueueCmdPools[i*2+1] = shotTimePool;

                createdQueueFamilyCommandPools[curQueueFamilyIndex] = std::make_pair(longLivePool, shotTimePool);

            }
            else 
            {
                m_DeviceQueueCmdPools[i*2] = createdQueueFamilyCommandPools[curQueueFamilyIndex].first;
                m_DeviceQueueCmdPools[i*2+1] = createdQueueFamilyCommandPools[curQueueFamilyIndex].second;   
            }
        }
    }

    return true;
    
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

    BufferDesc desc{};
    desc.device = this;
    desc.size = size;
    desc.usage = usage;
    desc.memFlags = memProp;
    Buffer *newBuffer = _BufferPool.Get();
    if (!newBuffer->Create(desc))
    {
        newBuffer->Reset();
        _BufferPool.Return(newBuffer);
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
    _BufferPool.Return(pBuffer);

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

QueueFamilyIndices::Key Device::GetQueueKey(VkQueue queue) const
{
    assert(IsValid());

    for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX; i++)
    {
        if (m_DeviceQueues[i] == queue)
            return (QueueFamilyIndices::Key)i;
    }
    
    return QueueFamilyIndices::MAX_INDEX;
}


CommandBuffer* Device::CreateCommandBufferImp(VkQueue queue, bool temprary)
{
    if (VKHANDLE_IS_NULL(queue))
        return nullptr;

    int keyIdx = GetQueueKey(queue);
    assert(keyIdx != QueueFamilyIndices::MAX_INDEX);
    
    // alloc one resetable command buffer
    VkCommandPool pool = temprary ? m_DeviceQueueCmdPools[keyIdx * 2 + 1] : m_DeviceQueueCmdPools[keyIdx * 2];
    VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = pool;
    allocInfo.commandBufferCount = 1;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VkCommandBuffer cmdBuffer{VK_NULL_HANDLE};
    VkResult result = vkAllocateCommandBuffers(m_vkDevice, &allocInfo, &cmdBuffer);
    if (result != VK_SUCCESS)
    {
        char str[256];
        sprintf(str, "Alloc command buffer error: %d", result);
        throw std::runtime_error(str);
    }

    CommandBuffer* pCmdBuf = _CmdBufPool.Get();
    pCmdBuf->SetUp(this, pool, queue, cmdBuffer, temprary);
   
    return pCmdBuf;
}


VkQueue Device::GetPresentQueue(const Window* window) const
{
    VkSurfaceKHR surface = window->GetVulkanSurface();
    if (VKHANDLE_IS_NULL(surface))
        return VK_NULL_HANDLE;
    
    if (!IsValid())
        return VK_NULL_HANDLE;

    int presentQueueFamIdx = m_DeviceQueueFamilyIndices.PresentQueueFamilyIndex(surface);
    if (presentQueueFamIdx == -1)
        return VK_NULL_HANDLE;

    for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX; i++)
    {
        int queueFamIdx = m_DeviceQueueFamilyIndices.QueueFamilyIndex((QueueFamilyIndices::Key)i);
        if (queueFamIdx == presentQueueFamIdx)
            return m_DeviceQueues[i];
    }
    
    return VK_NULL_HANDLE;
}



std::vector<VkSurfaceFormatKHR> Device::GetSupportedPresentFormats(Window* window) const
{
    std::vector<VkSurfaceFormatKHR> supportedFmts{};
    VkSurfaceKHR surface = window->GetVulkanSurface();
    if (!IsValid() || VKHANDLE_IS_NULL(surface))
        return std::move(supportedFmts);

    uint32_t fmtCnt{0};
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhyDevice, surface, &fmtCnt, nullptr);
    if (fmtCnt <= 0)
        return std::move(supportedFmts);
    
    supportedFmts.resize(fmtCnt);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_vkPhyDevice, surface, &fmtCnt, supportedFmts.data());

    return std::move(supportedFmts);
}


std::vector<VkPresentModeKHR> Device::GetSupportedPresentModes(Window* window) const
{
    std::vector<VkPresentModeKHR> supportedModes{};
    VkSurfaceKHR surface = window->GetVulkanSurface();
    if (!IsValid() || VKHANDLE_IS_NULL(surface))
        return std::move(supportedModes);
    
    uint32_t modeCnt{0};
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhyDevice, surface, &modeCnt, nullptr);
    if (modeCnt <= 0)
        return std::move(supportedModes);
    
    supportedModes.resize(modeCnt);
    vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkPhyDevice, surface, &modeCnt, supportedModes.data());

    return std::move(supportedModes);
}


bool Device::GetSurfaceCapabilities(Window* window, VkSurfaceCapabilitiesKHR* pResult) const
{
    if (pResult == nullptr)
        return false;

    VkSurfaceCapabilitiesKHR surfaceCaps{};
    VkSurfaceKHR surface = window->GetVulkanSurface();
    if (!IsValid() || VKHANDLE_IS_NULL(surface))
        return false;
    
    return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vkPhyDevice, surface, pResult) == VK_SUCCESS;
}