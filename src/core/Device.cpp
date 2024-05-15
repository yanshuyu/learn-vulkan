#include"core\Device.h"
#include<iostream>
#include<algorithm>
#include<map>
#include<utility>
#include<set>
#include<glslang\glslang\Include\glslang_c_interface.h>
#include<glslang\glslang\Public\resource_limits_c.h>

Device::Device()
    : _BufferPool(
          nullptr,
          [](Buffer *pBuffer)
          { assert(!pBuffer->IsValid()); },
          [this]()
          { return new Buffer(this); }),
      _CmdBufPool(
          nullptr,
          [](CommandBuffer *pCmdBuf)
          { assert(!pCmdBuf->IsValid()); },
          [this]()
          { return new CommandBuffer(this); }),
      _FencePool(
          nullptr,
          [](Fence *pFence)
          { assert(!pFence->IsValid()); },
          [this]()
          { return new Fence(this); })
{
    for (size_t i = 0; i < QueueFamilyIndices::MAX_INDEX; i++)
    {
        m_DeviceQueues[i] = VK_NULL_HANDLE;
        m_DeviceQueueCmdPools[i*2] = VK_NULL_HANDLE;
        m_DeviceQueueCmdPools[i*2+1] = VK_NULL_HANDLE;
        //m_DeviceQueueCmdBuffers[i] = VK_NULL_HANDLE;
    }
    
}

 bool Device::Initailze(VkPhysicalDevice phyDevice, uint32_t driverVersion)
 {
    if (IsValid())
        return true;

    // init device's properties
    vkGetPhysicalDeviceProperties(phyDevice, &m_PhyDeviceProps);
    
    // init device's features
    vkGetPhysicalDeviceFeatures(phyDevice, &m_PhyDeviceFeatures);

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

    m_vkApiVersion = driverVersion;

    return ok;
 }


void Device::Release()
{
    if (!IsValid())
        return;

    WaitIdle();

    _CmdBufPool.CleanUp();
    _BufferPool.CleanUp();
    _FencePool.CleanUp();

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
    if (pCmdBuf->GetDevice() != this)
    {
        LOGW("Try to release a command buffer({}) create by device({}) with diffrence device({})!", (void*)pCmdBuf, (void*)pCmdBuf->GetDevice(), (void*)this);
        return false;
    }
    pCmdBuf->Release();
    _CmdBufPool.Return(pCmdBuf);

    return true;
}


void Device::ResetAllHints()
{
    m_DeviceExtendsions.clear();
    m_EnablePhyDeviceFeatures.clear();
}

void Device::SetDeviceFeatureHint(DeviceFeatures feature, bool enabled)
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
    //VkPhysicalDeviceFeatures phyDevicefeatures;
    //vkGetPhysicalDeviceFeatures(phyDevice, &phyDevicefeatures);
    for (size_t i = 0; i < m_EnablePhyDeviceFeatures.size(); i++)
    {
        DeviceFeatures feature = m_EnablePhyDeviceFeatures[i];
        if (!vkutils_fetch_device_feature(m_PhyDeviceFeatures, feature))
        {
            LOGE("-->Device's feature({}) not support!", feature);
            return false;
        }
    }

    return true;
}

bool Device::AllocMemory(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags memPropFlag, VkDeviceMemory* pAllocatedMem)
{
    if (!IsValid())
        return false;

    if (m_PhyDeviceMemProps.memoryHeapCount == 0 || m_PhyDeviceMemProps.memoryTypeCount == 0)
        QueryDeviceMemoryProperties();

    size_t memTypeIdx = -1;
    for (size_t i = 0; i < m_PhyDeviceMemProps.memoryTypeCount; i++)
    {
        if (memReq.memoryTypeBits & (1 << i)  > 0 // avaliable memory type index
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
    allocInfo.allocationSize = memReq.size;

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
    desc.size = size;
    desc.usage = usage;
    desc.memFlags = memProp;
    Buffer *pBuf = _BufferPool.Get();
    assert(pBuf->_create(desc));
    return pBuf;
}

bool Device::DestroyBuffer(Buffer* pBuffer)
{
    if (pBuffer->GetDevice() != this)
    {
        LOGW("Try to release a Buffer({}) create by device({}) with diffrence device({})!", (void*)pBuffer, (void*)pBuffer->GetDevice(), (void*)this);
        return false;
    }
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

    Fence* fence = _FencePool.Get();
    assert(fence->_create(signaled));
    return fence;
}

bool Device::DestroyFence(Fence* pFence)
{
    if (pFence->GetDevice() != this)
    {
        LOGW("Try to release a Fence({}) create by device({}) with diffrence device({})!", (void*)pFence, (void*)pFence->GetDevice(), (void*)this);
        return false;
    }
    pFence->Release();
    _FencePool.Return(pFence);

    return true;
}

bool Device::CompileShader(const char *path, VkShaderStageFlagBits stage, std::vector<char> &spvCodes) const
{
    static std::unordered_map<VkShaderStageFlagBits, glslang_stage_t> _vkstageToGlslangStage = {
        {VK_SHADER_STAGE_VERTEX_BIT, GLSLANG_STAGE_VERTEX},
        {VK_SHADER_STAGE_FRAGMENT_BIT, GLSLANG_STAGE_FRAGMENT},
    };

    static auto _glslang_dump_error = [=](const char *stageStr, const char *filePath, glslang_shader_t *shader, glslang_program_t *program)
    {
        LOGE("GLSLang {} {} Error!\n{}\n{}\n",
             stageStr,
             filePath,
             glslang_shader_get_info_log(shader),
             glslang_shader_get_info_debug_log(shader));

        glslang_shader_delete(shader);
        if (program) glslang_program_delete(program);
    };

    static auto _glslang_local_include_resolve = [](void *ctx, const char *header_name, const char *includer_name, size_t include_depth) -> glsl_include_result_t *
    {
        std::string incPath = (const char *)ctx;
        size_t pos = incPath.find_last_of('/');
        if (pos == std::string::npos)
            pos = incPath.find_last_of('\\');

        pos++;
        incPath.replace(pos, incPath.size() - pos, header_name);

        file_bytes headerSrc = futils_read_file_bytes(incPath.c_str());
        assert(headerSrc.byteCnt > 0);

        glsl_include_result_t *result = new glsl_include_result_t();
        result->header_name = header_name;
        result->header_data = headerSrc.bytes;
        result->header_length = headerSrc.byteCnt;

        return result;
    };

    static auto _glslang_include_free = [](void *ctx, glsl_include_result_t *result) -> int
    {
        file_bytes fileResult{(char *)result->header_data, result->header_length};
        futils_free_file_bytes(fileResult);
        delete result;
        return 0;
    };

    static auto _vkapi_version_to_glslang_vkclient_version = [](uint32_t apiVersion) -> glslang_target_client_version_t 
    {
        int major = VK_API_VERSION_MAJOR(apiVersion);
        int minor = VK_API_VERSION_MINOR(apiVersion);
        int clientVerion = (major << 22) | (minor << 12);
        return (glslang_target_client_version_t)clientVerion;
    };

    file_bytes glslSrc = futils_read_file_bytes(path);
    glslang_input_t glslangInput{};
    glslang_shader_t *glslangShader{nullptr};
    glslang_program_t *glslangProgram{nullptr};

    LOGI("GLSLang compile {}\n{}", path, glslSrc.bytes);

    glslang_initialize_process();

    glslangInput.language = GLSLANG_SOURCE_GLSL;
    glslangInput.stage = _vkstageToGlslangStage[stage];
    glslangInput.client = GLSLANG_CLIENT_VULKAN;
    glslangInput.client_version = _vkapi_version_to_glslang_vkclient_version(m_vkApiVersion);
    glslangInput.target_language = GLSLANG_TARGET_SPV;
    glslangInput.target_language_version = GLSLANG_TARGET_SPV_1_5,
    glslangInput.code = glslSrc.bytes;
    glslangInput.default_version = 100;
    glslangInput.default_profile = GLSLANG_NO_PROFILE;
    glslangInput.force_default_version_and_profile = false;
    glslangInput.forward_compatible = false;
    glslangInput.messages = GLSLANG_MSG_DEFAULT_BIT;
    glslangInput.resource = glslang_default_resource();
    glslangInput.callbacks_ctx = (void*)path;
    glslangInput.callbacks.include_local = _glslang_local_include_resolve;
    glslangInput.callbacks.free_include_result = _glslang_include_free;

    glslangShader = glslang_shader_create(&glslangInput);

    if (!glslang_shader_preprocess(glslangShader, &glslangInput))
    {
        _glslang_dump_error("preprocess", path, glslangShader, glslangProgram);
        futils_free_file_bytes(glslSrc);
        return false;
    }

    if (!glslang_shader_parse(glslangShader, &glslangInput))
    {
        _glslang_dump_error("parse", path, glslangShader, glslangProgram);
        futils_free_file_bytes(glslSrc);
        return false;
    }

    glslangProgram = glslang_program_create();
    glslang_program_add_shader(glslangProgram, glslangShader);
    if (!glslang_program_link(glslangProgram, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        _glslang_dump_error("linking", path, glslangShader, glslangProgram);
        futils_free_file_bytes(glslSrc);
        return false;
    }

    glslang_program_SPIRV_generate(glslangProgram, glslangInput.stage);
    size_t spvWordSz = glslang_program_SPIRV_get_size(glslangProgram);
    spvCodes.resize(spvWordSz * sizeof(uint32_t));
    glslang_program_SPIRV_get(glslangProgram, (uint32_t *)spvCodes.data());

    glslang_program_delete(glslangProgram);
    glslang_shader_delete(glslangShader);
    glslang_finalize_process();
    futils_free_file_bytes(glslSrc);
    return true;
}

VkPhysicalDeviceFeatures Device::HardwareFeaturesToVkPhysicalDeviceFeatures() const
{
    VkPhysicalDeviceFeatures phyDeviceFeatures{};
    for (size_t i = 0; i < m_EnablePhyDeviceFeatures.size(); i++)
    {
        DeviceFeatures feature = m_EnablePhyDeviceFeatures[i];
        switch (feature)
        {
        case DeviceFeatures::geometryShader:
            phyDeviceFeatures.geometryShader = VK_TRUE;
            break;
        case DeviceFeatures::tessellationShader:
            phyDeviceFeatures.tessellationShader = VK_TRUE;
            break;
        case DeviceFeatures::samplerAnisotropy:
            phyDeviceFeatures.samplerAnisotropy = VK_TRUE;
            break;
        case DeviceFeatures::textureCompressionETC2:
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
    CommandBuffer* pCmdBuf = _CmdBufPool.Get();
    assert(pCmdBuf->_create(pool, queue, temprary));
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


bool Device::IsFeatureSupport(DeviceFeatures feature) const
{
    if (IsValid())
        return false;

    return vkutils_fetch_device_feature(m_PhyDeviceFeatures, feature);
}

bool Device::IsFeatureEnabled(DeviceFeatures feature) const
{
    return std::find(m_EnablePhyDeviceFeatures.begin(), m_EnablePhyDeviceFeatures.end(), feature) != m_EnablePhyDeviceFeatures.end();
}

uint32_t Device::GetDeviceLimit(DeviceLimits limit) const
{
    if (IsValid())
        return 0;
    
    return vkutils_fetch_device_limit(m_PhyDeviceProps.limits, limit);
}


bool Device::IsFormatFeatureSupport(VkFormat fmt, VkFormatFeatureFlagBits fmtFeature, bool linearTiling) const
{
    if (!IsValid())
        return false;

    VkFormatProperties fmtProps{};
    vkGetPhysicalDeviceFormatProperties(m_vkPhyDevice, fmt, &fmtProps);
    VkFormatFeatureFlags deviceFmtFeatures = linearTiling ? fmtProps.linearTilingFeatures : fmtProps.optimalTilingFeatures;
    return deviceFmtFeatures & fmtFeature;
}