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
    for (size_t i = 0; i < QueueType::MaxQueueType; i++)
    {
        m_DeviceQueues[i] = VK_NULL_HANDLE;
        m_DeviceQueueFamilyIndices[i] = -1;
        m_DeviceQueueCmdPools[i*2] = VK_NULL_HANDLE;
        m_DeviceQueueCmdPools[i*2+1] = VK_NULL_HANDLE;
        //m_DeviceQueueCmdBuffers[i] = VK_NULL_HANDLE;
    }
    
}

 bool Device::Create(VkPhysicalDevice phyDevice, uint32_t driverVersion, const DeviceCreation& desc)
 {
    if (IsValid())
        return false; 

    // verify device is support
    // support all queue types
    int queueFamilyIndices[QueueType::MaxQueueType];
    VkQueueFamilyProperties queueProperties[QueueType::MaxQueueType];
    for (size_t i = 0; i < desc.enableQueueCnt; i++)
    {
        int idx = vkutils_queue_type_family_index(phyDevice, desc.enableQueue[i], &queueProperties[desc.enableQueue[i]]);
        if (idx == -1)
            return false;
        queueFamilyIndices[desc.enableQueue[i]] = idx;
    }

    // support all extension
    uint32_t deviceExtendsionCnt{0};
    std::vector<VkExtensionProperties> deviceExtensionProperties{};
    vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &deviceExtendsionCnt, nullptr);
    deviceExtensionProperties.resize(deviceExtendsionCnt);
    vkEnumerateDeviceExtensionProperties(phyDevice, nullptr, &deviceExtendsionCnt, deviceExtensionProperties.data());
    for (size_t j=0; j<desc.enableExtendsionCnt; j++)
    {
        auto pos = std::find_if(deviceExtensionProperties.begin(), deviceExtensionProperties.end(), [&](const VkExtensionProperties& extProp){
            return strcmp(desc.enableExtendsions[j], extProp.extensionName) == 0;
        });

        if (pos == deviceExtensionProperties.end())
        {
            LOGE("-->Extendsion \"{}\" Is Not Supported By Device!", desc.enableExtendsions[j]);
            return false;
        }  
    }

    // support all feature
    VkPhysicalDeviceFeatures deviceFeatures{};
    vkGetPhysicalDeviceFeatures(phyDevice, &deviceFeatures);
    for (size_t i = 0; i < desc.enableFeatureCnt; i++)
    {
        if (!vkutils_fetch_device_feature(deviceFeatures, desc.enableFeatures[i]))
        {
            LOGE("-->Device's feature({}) not support!", desc.enableFeatures[i]);
            return false;
        }
    }

    LOGI("--> Device({}) Queue Infos:", (void *)phyDevice);
    for (size_t i = 0; i < desc.enableQueueCnt; i++)
    {
        LOGI("\tQueue{}: flags({}) queue count({}) -> type({})",
             i,
             vkutils_queue_flags_str(queueProperties[desc.enableQueue[i]].queueFlags),
             queueProperties[desc.enableQueue[i]].queueCount,
             vkutils_queue_type_str(desc.enableQueue[i]));
    }

    LOGI("--> Detecte Device Supported Extendsions: {}", deviceExtendsionCnt); 
    for (const auto& extProp : deviceExtensionProperties)
    {
        LOGI("\t{}\t{}",extProp.extensionName, extProp.specVersion);
    }

    LOGI("--> Enabled Device Extendsions: {}", desc.enableExtendsionCnt);
    for (size_t i=0; i<desc.enableExtendsionCnt; ++i)
    {
        LOGI("\t{}", desc.enableExtendsions[i]);
    }


    // create logical device
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(desc.enableQueueCnt);
    float defaultQueuePriorty = 1;
    for (size_t i=0; i<desc.enableQueueCnt; i++)
    {
        queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfos[i].flags = 0;
        queueCreateInfos[i].pNext = nullptr;
        queueCreateInfos[i].queueFamilyIndex = queueFamilyIndices[desc.enableQueue[i]];
        queueCreateInfos[i].queueCount = 1;
        queueCreateInfos[i].pQueuePriorities = &defaultQueuePriorty;
    }
    VkPhysicalDeviceFeatures enableFeatures{};
    if (desc.enableFeatureCnt > 0)
        enableFeatures = vkutils_populate_physical_device_feature(desc.enableFeatures, desc.enableFeatureCnt);
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.flags = 0;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = nullptr; // depcrecated
    deviceCreateInfo.queueCreateInfoCount = queueCreateInfos.size();
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
    deviceCreateInfo.enabledExtensionCount = desc.enableExtendsionCnt;
    deviceCreateInfo.ppEnabledExtensionNames = desc.enableExtendsions;
    deviceCreateInfo.pEnabledFeatures = desc.enableExtendsionCnt > 0 ? &enableFeatures : nullptr;

    VkDevice createdDevice = VK_NULL_HANDLE;
    VkResult result = vkCreateDevice(phyDevice, &deviceCreateInfo, nullptr, &createdDevice);
    if (result == VK_SUCCESS)
    {
        m_vkDevice = createdDevice;
        m_vkPhyDevice = phyDevice;
        m_vkApiVersion = driverVersion;
        vkGetPhysicalDeviceProperties(phyDevice, &m_PhyDeviceProps);
        vkGetPhysicalDeviceMemoryProperties(m_vkPhyDevice, &m_PhyDeviceMemProps);
        m_PhyDeviceFeatures = deviceFeatures;
        m_DeviceExtendsions.reserve(desc.enableExtendsionCnt);
        for (size_t i = 0; i < desc.enableExtendsionCnt; i++)
            m_DeviceExtendsions.emplace_back(desc.enableExtendsions[i]);
        m_DeviceFeatures.reserve(desc.enableFeatureCnt);
        for (size_t i = 0; i < desc.enableFeatureCnt; i++)
            m_DeviceFeatures.emplace_back(desc.enableFeatures[i]);
        for (size_t i = 0; i < desc.enableQueueCnt; i++)
            m_DeviceQueueFamilyIndices[desc.enableQueue[i]] = queueFamilyIndices[desc.enableQueue[i]];
        for (size_t i = 0; i < desc.enableQueueCnt; i++)
            vkGetDeviceQueue(createdDevice, queueFamilyIndices[desc.enableQueue[i]], 0, &m_DeviceQueues[desc.enableQueue[i]]);
    }
    else 
    {
        LOGE("--> Create vulkan device error: {}", result);
        return false;
    }
        
    result = _create_command_pools();    
    if(result != VK_SUCCESS)
    {
        LOGE("-->vulkan device create cmd pool error: {}", result);
        return false;
    }

    LOGI("--> Create Device success.");

    return true;
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
    for (size_t i=0; i<QueueType::MaxQueueType*2; i++)
    {
        if (m_DeviceQueueCmdPools[i] != VK_NULL_HANDLE) 
        {   
            vkDestroyCommandPool(m_vkDevice, m_DeviceQueueCmdPools[i], nullptr);
            m_DeviceQueueCmdPools[i] = VK_NULL_HANDLE;
        }
    }

    //vkDeviceWaitIdle(m_vkDevice); // ensure all job has finish before destroy any device
    vkDestroyDevice(m_vkDevice, nullptr); // destroy deivce will automatically destroy it's created queues

    for (size_t i = 0; i < QueueType::MaxQueueType; i++)
    {
        m_DeviceQueues[i] = VK_NULL_HANDLE;
        m_DeviceQueueFamilyIndices[i] = -1;
    }
    
    m_vkDevice = VK_NULL_HANDLE;
    m_vkPhyDevice = VK_NULL_HANDLE;
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


VkResult Device::_create_command_pools()
{
    for (size_t i = 0; i < QueueType::MaxQueueType; i++)
    {
        int queueFamIdx = m_DeviceQueueFamilyIndices[i];
        if (queueFamIdx != -1)
        {
            // create 2 command pool for this type queue
            // one for shot time lives
            // one for long time use, can reset to record again
            VkCommandPoolCreateInfo poolCreateInfo{};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
            poolCreateInfo.pNext = nullptr;
            poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
            poolCreateInfo.queueFamilyIndex = queueFamIdx;

            VkCommandPool longLivePool = VK_NULL_HANDLE;
            VkResult result = vkCreateCommandPool(m_vkDevice, &poolCreateInfo, nullptr, &longLivePool);
            if (result != VK_SUCCESS)
                return result;
            m_DeviceQueueCmdPools[i * 2] = longLivePool;

            poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
            VkCommandPool shotTimePool = VK_NULL_HANDLE;
            result = vkCreateCommandPool(m_vkDevice, &poolCreateInfo, nullptr, &shotTimePool);
            if (result != VK_SUCCESS)
                return result;
            m_DeviceQueueCmdPools[i * 2 + 1] = shotTimePool;
        }
    }

    return VK_SUCCESS;
    
}



bool Device::AllocMemory(const VkMemoryRequirements& memReq, VkMemoryPropertyFlags memPropFlag, VkDeviceMemory* pAllocatedMem)
{
    if (!IsValid())
        return false;

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


Buffer* Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memProp,  const char* name)
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
    assert(pBuf->_create(desc, name));
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

    static auto _glslang_dump_log = [=](const char logType, const char *stageStr, const char *filePath, glslang_shader_t *shader, glslang_program_t *program, bool release = false)
    {
        bool ok = false;
        switch (logType)
        {
        case 'e':
            LOGE("GLSLang  {} {} ->\n{}\n{}\n",
                 stageStr,
                 filePath,
                 glslang_shader_get_info_log(shader),
                 glslang_shader_get_info_debug_log(shader));
            ok = true;
            break;
        case 'w':
            LOGW("GLSLang {} {} ->\n{}\n{}\n",
                 stageStr,
                 filePath,
                 glslang_shader_get_info_log(shader),
                 glslang_shader_get_info_debug_log(shader));
            ok = true;
            break;
        case 'i':
             LOGI("GLSLang {} {} ->\n{}\n{}\n",
                stageStr,
                filePath,
                glslang_shader_get_info_log(shader),
                glslang_shader_get_info_debug_log(shader));
            ok = true;
            break;
        default:
            break;
        }

        if (ok && release)
        {
            glslang_shader_delete(shader);
            if (program)
                glslang_program_delete(program);
        }
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

    LOGI("GLSLang compiling {}\n{}", path, glslSrc.bytes);

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
        _glslang_dump_log('e', "preprocess", path, glslangShader, glslangProgram);
        futils_free_file_bytes(glslSrc);
        return false;
    }

    if (!glslang_shader_parse(glslangShader, &glslangInput))
    {
        _glslang_dump_log('e' ,"parse", path, glslangShader, glslangProgram);
        futils_free_file_bytes(glslSrc);
        return false;
    }

    glslangProgram = glslang_program_create();
    glslang_program_add_shader(glslangProgram, glslangShader);
    if (!glslang_program_link(glslangProgram, GLSLANG_MSG_SPV_RULES_BIT | GLSLANG_MSG_VULKAN_RULES_BIT))
    {
        _glslang_dump_log('e', "linking", path, glslangShader, glslangProgram);
        futils_free_file_bytes(glslSrc);
        return false;
    }

    _glslang_dump_log('i', "finish", path, glslangShader, glslangProgram, false);

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


bool Device::SupportPrenset(Window* window) const
{
    if (GetMainQueue() == VK_NULL_HANDLE)
        return false;
    VkBool32 support{false};
    vkGetPhysicalDeviceSurfaceSupportKHR(m_vkPhyDevice, m_DeviceQueueFamilyIndices[QueueType::Main], window->GetVulkanSurface(), &support);
    return support;
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
    return std::find(m_DeviceFeatures.begin(), m_DeviceFeatures.end(), feature) != m_DeviceFeatures.end();
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


QueueType Device::_get_queue_type(VkQueue queue) const
{
    for (size_t i = 0; i < QueueType::MaxQueueType; i++)
    {
        if (m_DeviceQueues[i] == queue)
            return (QueueType)i;
    }
    
    return MaxQueueType;
}


CommandBuffer* Device::_create_command_buffer(VkQueue queue, bool temprary)
{
    if (VKHANDLE_IS_NULL(queue))
        return nullptr;

    QueueType qt = _get_queue_type(queue);
    if (qt == MaxQueueType)
    {
        LOGE("Invalide Queue!");
        return nullptr;
    }
    
    // alloc one resetable command buffer
    VkCommandPool pool = temprary ? m_DeviceQueueCmdPools[qt * 2 + 1] : m_DeviceQueueCmdPools[qt * 2];
    CommandBuffer* pCmdBuf = _CmdBufPool.Get();
    assert(pCmdBuf->_create(pool, queue, temprary));
    return pCmdBuf;
}