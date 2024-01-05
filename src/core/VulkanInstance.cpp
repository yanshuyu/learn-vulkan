#include"core\VulkanInstance.h"
#include"core\QueueFamilyIndices.h"

VulkanInstance* VulkanInstance::sActive = nullptr;


static VKAPI_ATTR VkBool32 VKAPI_CALL __DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT msgType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData)
{

    switch (msgSeverity)
    {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
        //LOGI("Vulkan Validation Layer[V]--> {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
        LOGI("Vulkan Validation Layer[I]-->\n {}", pCallbackData->pMessage);
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
        LOGW("Vulkan Validation Layer[W]-->\n {}", pCallbackData->pMessage)
        break;
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
        LOGE("Vulkan Validation Layer[E]-->\n {}", pCallbackData->pMessage)
        break;
    default:
        break;
    }
    return VK_FALSE;
}



void VulkanInstance::ResetAllHints()
{
    m_ApiVersion = 0;
    m_DebugEnabled = false;
    m_InstanceExtendsions.clear();
    m_InstanceLayers.clear();
}


bool VulkanInstance::Initailize()
{

    if (m_DebugEnabled)
    {
        SetInstanceLayerHint("VK_LAYER_KHRONOS_validation", true);
        SetInstanceExtendsionHint(VK_EXT_DEBUG_UTILS_EXTENSION_NAME, true); //roung validation layer's debug msg to our callback fundtion
    }

      // verify all extendsions are supported 
    uint32_t instanceSupportExtCnt = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, nullptr);
    std::vector<VkExtensionProperties> instanceSupportExtendsionProps(instanceSupportExtCnt);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, instanceSupportExtendsionProps.data());
    LOGI("-->Detect Vulkan Instance Supported Extendsions: {}", instanceSupportExtCnt);
    for (const auto& extProp : instanceSupportExtendsionProps)
    {
        LOGI(extProp.extensionName); 
    }

        
    LOGI("-->Enabled Instance Extendsions: {}", m_InstanceExtendsions.size());
    for (const auto& ext : m_InstanceExtendsions)
    {
        LOGI(ext);
    }

    for (const auto &ext : m_InstanceExtendsions)
    {
        auto pos = std::find_if(instanceSupportExtendsionProps.begin(), instanceSupportExtendsionProps.end(),[&ext](const auto& extProp){
            return strcmp(ext.c_str(), extProp.extensionName) == 0;
        });

        if ( pos == instanceSupportExtendsionProps.end())
        {
            LOGE("-->Vulkan Instance Extendsion \"{}\" Is Not Supported!", ext);
            return false; 
        }
    }

    // verify all layers are supported
    uint32_t instanceSupportLayerCnt = 0;
    vkEnumerateInstanceLayerProperties(&instanceSupportLayerCnt, nullptr);
    std::vector<VkLayerProperties> instanceSupportLayerProps(instanceSupportExtCnt);
    vkEnumerateInstanceLayerProperties(&instanceSupportLayerCnt, instanceSupportLayerProps.data());
    LOGI("--> Detect vulkan Instance Supported Layers: {}",instanceSupportLayerCnt);
    for (const auto& layProp : instanceSupportLayerProps)
    {
        LOGI("\t {}\n\t\t {}", layProp.layerName, layProp.description); 
    }

        
    LOGI("-->Enabled Instance Layers: {}" , m_InstanceLayers.size());
    for (const auto& layer : m_InstanceLayers)
    {
       LOGI("\t{}\n",layer);
    }

    for (const auto& lay : m_InstanceLayers)
    {
        auto pos = std::find_if(instanceSupportLayerProps.begin(), instanceSupportLayerProps.end(), [&lay](const auto& layProp){
            return strcmp(lay.c_str(), layProp.layerName) == 0;
        });

        if ( pos == instanceSupportLayerProps.end())
        {
            LOGE("-->Vulkan Instance Layer \"{}\" Is Not Supported!", lay);
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
    VkDebugUtilsMessengerCreateInfoEXT msgerCreateInfo{VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
    if (m_DebugEnabled)
    {
        msgerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        msgerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        msgerCreateInfo.pfnUserCallback = __DebugMessengerCallback;
        
    }
    
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extendsionsNames.size());
    createInfo.ppEnabledExtensionNames = extendsionsNames.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    createInfo.ppEnabledLayerNames = layerNames.data();
    createInfo.pNext = m_DebugEnabled ? &msgerCreateInfo : nullptr; // for debug create/destroy vkinstance function
    createInfo.flags = 0;
    VkInstance createdInstance = VK_NULL_HANDLE;
    VkResult result = vkCreateInstance(&createInfo, nullptr, &createdInstance);

    if (result != VK_SUCCESS)
    { 
        LOGE("--> Create vulkan instance error: {}", result);
        return false;
    }
    m_vkInstance = createdInstance;

    // create debug messager if validation layer is enabled and debug messager is enabled
    if(m_DebugEnabled)
    {
        // extendsion functions are not loaded by defualt loader
        // we must load manully
        auto messengerCreateFn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(createdInstance, "vkCreateDebugUtilsMessengerEXT");
        if (messengerCreateFn == nullptr)
        {
            LOGW("--> Vulkan failed to load function vkCreateDebugUtilsMessengerEXT!\n\tCreate debug messenger failed.");
        }
        else
        {
            result = (*messengerCreateFn)(createdInstance, &msgerCreateInfo, nullptr, &m_vkDebugMsger);
            LOGI("--> Create Vulkan Messenger: {}", result == VK_SUCCESS);
        }
        
    }


    return true;
}


VkPhysicalDevice VulkanInstance::RequestPhysicalDevice(VkQueueFlags queueOperation, VkSurfaceKHR presentSurface)
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

    LOGI("-->Detect Physical Devices Install On System: {}", physicalDeviceCnt);
    for (size_t i=0; i<physicalDeviceCnt; i++ )
    {
        vkGetPhysicalDeviceProperties(physicalDevices[i], &physicalDeviceProps[i]);
        LOGI("index\tDevice Name\t\t\tDevice Type\t\t\tDriver Version");
        LOGI("{}\t{}\t{}\t{}",
             i,
             physicalDeviceProps[i].deviceName,
             s_PhysicalDeviceTypeNames[(size_t)physicalDeviceProps[i].deviceType],
             physicalDeviceProps[i].driverVersion);
    }

    // pick suitable physcial device    
    size_t suitablePhyDeviceIndex = -1;
    QueueFamilyIndices suitablePhyDeviceQueueFamilyIndices;
    for (size_t i = 0; i < physicalDeviceCnt; i++)
    {
        suitablePhyDeviceQueueFamilyIndices.Query(physicalDevices[i]);
        if ((suitablePhyDeviceQueueFamilyIndices.CombindQueueFamilyFlags() & queueOperation) == queueOperation)
        {
            if (presentSurface != VK_NULL_HANDLE)
            {
                if (suitablePhyDeviceQueueFamilyIndices.IsPresentSupported(presentSurface))
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
        LOGE("-->Failed to find suitable physical device for application");
        return VK_NULL_HANDLE;
    }  

    return physicalDevices[suitablePhyDeviceIndex];

}


void VulkanInstance::Release()
{
    if (IsValid())
    {
        if (m_vkDebugMsger != VK_NULL_HANDLE)
        {
            auto destroyDebugMsgerFn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_vkInstance, "vkDestroyDebugUtilsMessengerEXT");
            if (destroyDebugMsgerFn != nullptr)
            {
                LOGI("--> Destroy Vulkan Debug Messenger");
                (*destroyDebugMsgerFn)(m_vkInstance, m_vkDebugMsger, nullptr);
            }
            else
            {
                LOGW("--> Vulkan filed to load function: vkDestroyDebugUtilsMessengerEXT!\n\tDestroy debug messenger fiailed.");
            }
            m_vkDebugMsger = VK_NULL_HANDLE;
        }

        if (m_vkInstance != VK_NULL_HANDLE)
        {
            vkDestroyInstance(m_vkInstance, nullptr);
            m_vkInstance = VK_NULL_HANDLE;
            
            if (sActive == this)
                sActive = nullptr;
        }
        
        ResetAllHints();
    }
}