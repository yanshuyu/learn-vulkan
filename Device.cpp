#include"Device.h"
#include<iostream>
#include<algorithm>
#include<map>
#include<utility>


Device::Device()
: m_vkInstance(VK_NULL_HANDLE)
, m_vkDebugMsger(VK_NULL_HANDLE)
, m_PresentSurface(VK_NULL_HANDLE)
, m_EnableQueueOperations(VK_QUEUE_GRAPHICS_BIT)
, m_vkPhyDevice(VK_NULL_HANDLE)
, m_vkDevice(VK_NULL_HANDLE)
, m_ApiVersion(0)
, m_DebugEnabled(false)
, m_OffScreenEnable(false)
, m_PresentQueueFamilyIndex(-1)
{
    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT * 2; i++)
    {
        m_DeviceCommandPools[i] = VK_NULL_HANDLE;
    }
    
}

Device::~Device()
{
    Release();
}

 bool Device::Initialize()
 {
    bool ok = CreateInstance();
    std::cout << "-->Create Vulkan Instances: " << ok << std::endl;
    if (!ok)
        return false;
    ok &= FindPyhsicalDevice();
    std::cout << "-->Find Suitable Physical Device: " << ok << std::endl;
    if (!ok)
        return false;
    
    return true;
 }

 bool Device::Create()
 {
    if (!AllHardWareFeatureSupported())
    {
        std::cout << "--> Not All Physical Device Features Are Supported!" << std::endl;
        return false;
    }

    bool ok = CreateLogicalDevice();
    std::cout << "-->Create Device: " << ok << std::endl;
    if (ok)
    {
        vkGetDeviceQueue(m_vkDevice, m_DeviceQueueFamilyIndices.GrapicQueueFamilyIndex(), 0, &m_DeviceGraphicQueue);
        if (!m_OffScreenEnable && m_PresentSurface != VK_NULL_HANDLE)
        {   
            m_PresentQueueFamilyIndex = m_DeviceQueueFamilyIndices.PresentQueueFamilyIndex(m_PresentSurface);
            vkGetDeviceQueue(m_vkDevice, m_PresentQueueFamilyIndex, 0, &m_DevicePresentQueue);
        }
        ok &= CreateCommandPools();
    }

    return ok;
 }


void Device::Release()
{

    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT; i++)
    {
        if (m_DeviceCommandPools[i] != VK_NULL_HANDLE)
        {
            vkDestroyCommandPool(m_vkDevice, m_DeviceCommandPools[i], nullptr);
            m_DeviceCommandPools[i] = VK_NULL_HANDLE;
        }
    }
    

    if (m_vkDevice != VK_NULL_HANDLE)
    {
        vkDeviceWaitIdle(m_vkDevice); // ensure all job has finish before destroy any device
        vkDestroyDevice(m_vkDevice, nullptr);
        m_vkDevice = VK_NULL_HANDLE;
        m_DeviceGraphicQueue = VK_NULL_HANDLE; // destroy deivce will automatically destroy it's created queues
        m_DevicePresentQueue = VK_NULL_HANDLE;
    }

    if (m_vkDebugMsger != VK_NULL_HANDLE)
    {
        auto destroyDebugMsgerFn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
        if (destroyDebugMsgerFn != nullptr)
        {
            std::cout << "--> Destroy Vulkan Debug Messenger" << std::endl;
            (*destroyDebugMsgerFn)(m_vkInstance, m_vkDebugMsger, nullptr);
        }
        else 
        {
            std::cout << "--> Vulkan filed to load function: vkDestroyDebugUtilsMessengerEXT!\n\tDestroy debug messenger fiailed." << std::endl;
        }
        m_vkDebugMsger = VK_NULL_HANDLE;
    }

    if (m_vkInstance != VK_NULL_HANDLE)
    {
        vkDestroyInstance(m_vkInstance, nullptr);
        m_vkInstance = VK_NULL_HANDLE;
    }

    ResetAllHints();
}


void Device::ResetAllHints()
{
    m_InstanceExtendsions.clear();
    m_InstanceLayers.clear();
    m_PresentSurface = VK_NULL_HANDLE;
    m_EnableQueueOperations = VK_QUEUE_GRAPHICS_BIT;
    m_DeviceExtendsions.clear();
    m_EnablePhyDeviceFeatures.clear();
    m_ApiVersion = 0;
    m_DebugEnabled = false;
    m_OffScreenEnable = false;
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


bool Device::CreateInstance()
{
     // verify all extendsions are supported 
    uint32_t instanceSupportExtCnt = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, nullptr);
    std::vector<VkExtensionProperties> instanceSupportExtendsionProps(instanceSupportExtCnt);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, instanceSupportExtendsionProps.data());
    std::cout << "-->Detect Vulkan Instance Supported Extendsions:" << instanceSupportExtCnt << std::endl;
    for (const auto& extProp : instanceSupportExtendsionProps)
    {
        std::cout << "\t" << extProp.extensionName << std::endl; 
    }

        
    std::cout << "-->Enabled Instance Extendsions: " << m_InstanceExtendsions.size() << std::endl;
    for (const auto& ext : m_InstanceExtendsions)
    {
        std::cout << "\t" << ext << std::endl;
    }

    for (const auto &ext : m_InstanceExtendsions)
    {
        auto pos = std::find_if(instanceSupportExtendsionProps.begin(), instanceSupportExtendsionProps.end(),[&ext](const auto& extProp){
            return strcmp(ext.c_str(), extProp.extensionName) == 0;
        });

        if ( pos == instanceSupportExtendsionProps.end())
        {
            std::cout << "-->Vulkan Instance Extendsion " << ext << " Is Not Supported!" << std::endl;
            return false; 
        }
    }

    // verify all layers are supported
    uint32_t instanceSupportLayerCnt = 0;
    vkEnumerateInstanceLayerProperties(&instanceSupportLayerCnt, nullptr);
    std::vector<VkLayerProperties> instanceSupportLayerProps(instanceSupportExtCnt);
    vkEnumerateInstanceLayerProperties(&instanceSupportLayerCnt, instanceSupportLayerProps.data());
    std::cout << "--> Detect vulkan Instance Supported Layers:" << instanceSupportLayerCnt << std::endl;
    for (const auto& layProp : instanceSupportLayerProps)
    {
        std::cout << "\t" << layProp.layerName << std::endl;
        std::cout << "\t\t" << layProp.description << std::endl; 
    }

        
    std::cout << "-->Enabled Instance Layers: " << m_InstanceLayers.size() << std::endl;
    for (const auto& layer : m_InstanceLayers)
    {
        std::cout << "\t" << layer << std::endl;
    }

    for (const auto& lay : m_InstanceLayers)
    {
        auto pos = std::find_if(instanceSupportLayerProps.begin(), instanceSupportLayerProps.end(), [&lay](const auto& layProp){
            return strcmp(lay.c_str(), layProp.layerName) == 0;
        });

        if ( pos == instanceSupportLayerProps.end())
        {
            std::cout << "-->Vulkan Instance Layer " << lay << " Is Not Supported!" << std::endl;
            return false; 
        }
    }
    
    // query supported instance's version
    if (m_ApiVersion == 0)
        vkEnumerateInstanceVersion(&m_ApiVersion);

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pApplicationName = "No Name";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = m_ApiVersion;


    std::vector<const char*> extendsionsNames(m_InstanceExtendsions.size());
    for (size_t i = 0; i < m_InstanceExtendsions.size(); i++)
    {
       extendsionsNames[i] = m_InstanceExtendsions[i].c_str();
    }
    
    std::vector<const char*> layerNames(m_InstanceLayers.size());
    for (size_t i = 0; i < m_InstanceLayers.size(); i++)
    {
        layerNames[i] = m_InstanceLayers[i].c_str();
    }
    VkDebugUtilsMessengerCreateInfoEXT msgerCreateInfo{};
    bool enableDebugMsger = m_DebugEnabled
        && std::find(m_InstanceLayers.begin(), m_InstanceLayers.end(), "VK_LAYER_KHRONOS_validation") != m_InstanceLayers.end()
        && std::find(m_InstanceExtendsions.begin(), m_InstanceExtendsions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != m_InstanceExtendsions.end();
    if (enableDebugMsger)
    {
        msgerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        msgerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        msgerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        msgerCreateInfo.pfnUserCallback = Device::DebugMessengerCallback;
        
    }
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extendsionsNames.size());
    createInfo.ppEnabledExtensionNames = extendsionsNames.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    createInfo.ppEnabledLayerNames = layerNames.data();
    createInfo.pNext = enableDebugMsger ? &msgerCreateInfo : nullptr; // for debug create/destroy vkinstance function
    createInfo.flags = 0;
    VkInstance createdInstance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &createdInstance);

    if (result != VK_SUCCESS)
    { 
        std::cout << "--> Create vulkan instance error: " << result << std::endl;
        return false;
    }
    m_vkInstance = createdInstance;

    // create debug messager if validation layer is enabled and debug messager is enabled
    if(enableDebugMsger)
    {
        // extendsion functions are not loaded by defualt loader
        // we must load manully
        auto messengerCreateFn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(createdInstance, "vkCreateDebugUtilsMessengerEXT");
        if (messengerCreateFn == nullptr)
        {
            std::cout << "--> Vulkan failed to load function vkCreateDebugUtilsMessengerEXT!\n\tCreate debug messenger failed." << std::endl;
        }
        else
        {
            result = (*messengerCreateFn)(createdInstance, &msgerCreateInfo, nullptr, &m_vkDebugMsger);
            std::cout << "--> Create Vulkan Messenger: " << (result == VK_SUCCESS) << std::endl;
        }
        
    }


    return true;
}


bool Device::FindPyhsicalDevice()
 {

    // enumerate physical devices
    uint32_t physicalDeviceCnt = 0;
    vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCnt, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCnt);
    vkEnumeratePhysicalDevices(m_vkInstance, &physicalDeviceCnt, physicalDevices.data());
    std::vector<VkPhysicalDeviceProperties> physicalDeviceProps(physicalDeviceCnt);

    static char* s_PhysicalDeviceTypeNames[4]{
        "Other",
        "Integrated-GPU",
        "Discrete-GPU",
        "CPU",
    };

    std::cout << "-->Detect Physical Devices Install On System: " << physicalDeviceCnt << std::endl;
    for (size_t i=0; i<physicalDeviceCnt; i++ )
    {
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProps[i]);
        std::cout << "index\t" << "Device Name\t\t\t" << "Device Type\t\t\t" << "Driver Version" << std::endl;
        std::cout << i << "\t" << physicalDeviceProps[i].deviceName << "\t" 
        << s_PhysicalDeviceTypeNames[(size_t)physicalDeviceProps[i].deviceType] << "\t"
        << physicalDeviceProps[i].driverVersion << std::endl;
    }

    // pick suitable physcial device    
    size_t suitablePhyDeviceIndex = -1;
    QueueFamilyIndices suitablePhyDeviceQueueFamilyIndices;
    for (size_t i = 0; i < physicalDeviceCnt; i++)
    {
        suitablePhyDeviceQueueFamilyIndices.Query(physicalDevices[i]);
        if ((suitablePhyDeviceQueueFamilyIndices.CombindQueueFamilyFlags() & m_EnableQueueOperations) == m_EnableQueueOperations)
        {
            if (!m_OffScreenEnable && m_PresentSurface != VK_NULL_HANDLE)
            {
                if (suitablePhyDeviceQueueFamilyIndices.IsPresentSupported(m_PresentSurface))
                    suitablePhyDeviceIndex = i;
            }
            else
            {
                suitablePhyDeviceIndex = i;
            }

            if (suitablePhyDeviceIndex != -1)
                break;
        }
    }
    
    if (suitablePhyDeviceIndex == -1)
    {
        std::cout << "-->Failed to find suitable physical device for application" << std::endl;
        return false;
    }  

    m_vkPhyDevice = physicalDevices[suitablePhyDeviceIndex];
    m_DeviceQueueFamilyIndices = suitablePhyDeviceQueueFamilyIndices;
    return true;
 }


bool Device::CreateLogicalDevice()
{
    // Check All Device Extendions Are Supported
    uint32_t deviceSupportedExtendsionCnt = 0;
    vkEnumerateDeviceExtensionProperties(m_vkPhyDevice, nullptr, &deviceSupportedExtendsionCnt, nullptr);
    std::vector<VkExtensionProperties> deviceSupportedExtendsionProps(deviceSupportedExtendsionCnt);
    vkEnumerateDeviceExtensionProperties(m_vkPhyDevice, nullptr, &deviceSupportedExtendsionCnt, deviceSupportedExtendsionProps.data());
    std::cout << "--> Detecte Device Supported Extendsions: " << deviceSupportedExtendsionCnt << std::endl; 
    for (const auto& extProp : deviceSupportedExtendsionProps)
    {
        std::cout << "\t" << extProp.extensionName << "\t" << extProp.specVersion << std::endl;
    }

    std::cout << "--> Enabled Device Extendsions: " << m_DeviceExtendsions.size() << std::endl;
    for (const auto& ext : m_DeviceExtendsions)
    {
        std::cout << "\t" << ext << std::endl;
    }
    
    for (const auto& ext : m_DeviceExtendsions)
    {
        auto pos = std::find_if(deviceSupportedExtendsionProps.begin(), deviceSupportedExtendsionProps.end(), [&](const VkExtensionProperties& extProp){
            return strcmp(ext.c_str(), extProp.extensionName) == 0;
        });

        if (pos == deviceSupportedExtendsionProps.end())
        {
            std::cout << "-->Extendsion " << ext << "Is Not Supported By Device!" << std::endl;
            return false;
        }  
    }

    // create logical device
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
    VkResult result = vkCreateDevice(m_vkPhyDevice, &deviceCreateInfo, nullptr, &createdDevice);
    if (result == VK_SUCCESS)
    {
        m_vkDevice = createdDevice;
    }
    else 
    {
        std::cout <<"--> Create vulkan device error: " << result << std::endl;
    }

    return result == VK_SUCCESS;
}

bool Device::CreateCommandPools()
{
    std::map<uint32_t, std::pair<VkCommandPool, VkCommandPool>> createdQueueFamilyCommandPools{};
    for (size_t i = 0; i < QUEUE_FAMILY_MAX_COUNT; i++)
    {
        uint32_t curQueueFamilyIndex = m_DeviceQueueFamilyIndices.QueueFamilyIndexAtIndex(i);
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
                m_DeviceCommandPools[i*2] = longLivePool;
                
                poolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
                VkCommandPool shotTimePool = VK_NULL_HANDLE;
                result = vkCreateCommandPool(m_vkDevice, &poolCreateInfo, nullptr, &shotTimePool);
                if (result != VK_SUCCESS)
                {
                    std::cout << "--> Create Command shot time Pool For Queue Family Index: " << curQueueFamilyIndex << ", vulkan error: " << result << std::endl;
                    return false;
                }
                m_DeviceCommandPools[i*2+1] = shotTimePool;

                createdQueueFamilyCommandPools[curQueueFamilyIndex] = std::make_pair(longLivePool, shotTimePool);

            }
            else 
            {
                m_DeviceCommandPools[i*2] = createdQueueFamilyCommandPools[curQueueFamilyIndex].first;
                m_DeviceCommandPools[i*2+1] = createdQueueFamilyCommandPools[curQueueFamilyIndex].second;   
            }
        }
    }

    return true;
    
}

void Device::ApplyExtendionOrLayerHint(std::vector<std::string>& arr, const char* name, bool enabled)
{
auto pos = std::find(arr.begin(), arr.end(), name);
if (!enabled && pos != arr.end())
{
    m_InstanceExtendsions.erase(pos);

}
else if (enabled && pos == arr.end())
{
    arr.push_back(name);
}
}


bool Device::AllHardWareFeatureSupported() const
{
VkPhysicalDeviceFeatures phyDevicefeatures;
vkGetPhysicalDeviceFeatures(m_vkPhyDevice, &phyDevicefeatures);
for (size_t i = 0; i < m_EnablePhyDeviceFeatures.size(); i++)
{
    HardwareFeature feature = m_EnablePhyDeviceFeatures[i];
    switch (feature)
    {
    case HardwareFeature::geometryShader:
        if (!phyDevicefeatures.geometryShader) return false; 
        break;
    case HardwareFeature::tessellationShader:
        if (!phyDevicefeatures.tessellationShader) return false;
        break;
    case HardwareFeature::samplerAnisotropy:
        if (!phyDevicefeatures.samplerAnisotropy) return false ;
        break;            
    case HardwareFeature::textureCompressionETC2:
        if(!phyDevicefeatures.textureCompressionETC2) return false;
        break;           
    
    default:
        break;
    }
}

return true ;
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


 VkCommandPool Device::GetCommandPool(JobOperation op, bool temprary)
 {
    int cmdPoolIdx = -1;
    switch (op)
    {
    case grapic:
        cmdPoolIdx = QUEUE_FAMILY_GRAPICS_INDEX;
        break;
    case compute:
        cmdPoolIdx = QUEUE_FAMILY_COMPUTE_INDEX;
    case transfer:
        cmdPoolIdx = QUEUE_FAMILY_TRANSFER_INDEX;
    case present:
        if (!m_OffScreenEnable && m_DevicePresentQueue != VK_NULL_HANDLE)
            cmdPoolIdx = m_PresentQueueFamilyIndex;
        break;

    default:
        break;
    }

    if (cmdPoolIdx == -1)
        return VK_NULL_HANDLE;
    
    if (temprary)
        cmdPoolIdx++;

    return m_DeviceCommandPools[cmdPoolIdx];
 }

VKAPI_ATTR VkBool32 VKAPI_CALL Device::DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT msgType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData)
{
    if (msgSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
        return false;

    static const char* vkValidationMsg = "[Vulkan Validation Msg]\t";
    char* msgSeverityStr = "Unknow";
    char* msgTypeStr = "Unknow";
    switch (msgSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        msgSeverityStr = "Verbose";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        msgSeverityStr = "Info";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        msgSeverityStr = "Warning";
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        msgSeverityStr = "Error";
        break;
    default:
        break;
    }

    switch (msgType)
    {
    case VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT:
        msgTypeStr = "General";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT:
        msgTypeStr = "Validation";
        break;
    case VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT:
        msgTypeStr = "Performance";
        break;
    default:
        break;
    }

    std::cout << "-->\n"
                << vkValidationMsg <<"severity:\t" << msgSeverityStr  << std::endl
                << vkValidationMsg << "type:\t" << msgTypeStr << std::endl
                << vkValidationMsg << "detail:\t" << pCallbackData->pMessage << std::endl
                << "<--\n";
    return VK_FALSE;
}