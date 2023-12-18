#include"VulkanUtil.h"
#include<iostream>
#include<algorithm>

const char* VulkanUtil::KHRONOS_STANDARD_VALIDATIONLAYER_NAME = "VK_LAYER_KHRONOS_validation";

bool VulkanUtil::CreateInstance(const vector<string>& enableExtendsions, 
                                const vector<string>& enableLayers,
                                VkInstance* pCreatedInstance, 
                                VkDebugUtilsMessengerEXT* pCreatedDebugMsger)
{

    // verify all extendsions are supported 
    uint32_t instanceSupportExtCnt = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, nullptr);
    vector<VkExtensionProperties> instanceSupportExtendsionProps(instanceSupportExtCnt);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, instanceSupportExtendsionProps.data());
    std::cout << "-->Detect Vulkan Instance Supported Extendsions:" << instanceSupportExtCnt << std::endl;
    for (const auto& extProp : instanceSupportExtendsionProps)
    {
        std::cout << "\t" << extProp.extensionName << std::endl; 
    }

    for (const auto &ext : enableExtendsions)
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
    vector<VkLayerProperties> instanceSupportLayerProps(instanceSupportExtCnt);
    vkEnumerateInstanceLayerProperties(&instanceSupportLayerCnt, instanceSupportLayerProps.data());
    std::cout << "--> Detect vulkan Instance Supported Layers:" << instanceSupportLayerCnt << std::endl;
    for (const auto& layProp : instanceSupportLayerProps)
    {
        std::cout << "\t" << layProp.layerName << std::endl;
        std::cout << "\t\t" << layProp.description << std::endl; 
    }

    for (const auto& lay : enableLayers)
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
    uint32_t apiVeriosn = 0;
    VkResult result = vkEnumerateInstanceVersion(&apiVeriosn);
    if (result != VK_SUCCESS)
        return false;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.pApplicationName = "No Name";
    appInfo.applicationVersion = VK_MAKE_VERSION(1,0,0);
    appInfo.apiVersion = apiVeriosn;


    vector<const char*> extendsionsNames(enableExtendsions.size());
    for (size_t i = 0; i < enableExtendsions.size(); i++)
    {
       extendsionsNames[i] = enableExtendsions[i].c_str();
    }
    
    vector<const char*> layerNames(enableLayers.size());
    for (size_t i = 0; i < enableLayers.size(); i++)
    {
        layerNames[i] = enableLayers[i].c_str();
    }

    VkDebugUtilsMessengerCreateInfoEXT msgerCreateInfo{};
    bool enableDebugMsger =  pCreatedDebugMsger != nullptr
        && std::find(enableLayers.begin(), enableLayers.end(), KHRONOS_STANDARD_VALIDATIONLAYER_NAME) != enableLayers.end()
        && std::find(enableExtendsions.begin(), enableExtendsions.end(), VK_EXT_DEBUG_UTILS_EXTENSION_NAME) != enableExtendsions.end();
    if (enableDebugMsger)
    {
        msgerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        msgerCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                                        | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        msgerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                                    | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        msgerCreateInfo.pfnUserCallback = VulkanUtil::DebugMessengerCallback;
        
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
    result = vkCreateInstance(&createInfo, nullptr, pCreatedInstance);

    if (result != VK_SUCCESS)
        return false;

    // create debug messager if validation layer is enabled and debug messager is enabled
    if(enableDebugMsger)
    {
        // extendsion functions are not loaded by defualt loader
        // we must load manully
        auto messengerCreateFn = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*pCreatedInstance, "vkCreateDebugUtilsMessengerEXT");
        if (messengerCreateFn == nullptr)
        {
            std::cout << "--> Vulkan failed to load function vkCreateDebugUtilsMessengerEXT!\n\tCreate debug messenger failed." << std::endl;
            return false;
        }
        
        result = (*messengerCreateFn)(*pCreatedInstance, &msgerCreateInfo, nullptr, pCreatedDebugMsger);
        std::cout << "--> Create Vulkan Messenger: " << (result == VK_SUCCESS) << std::endl;
    }


    return result  == VK_SUCCESS;
}


void VulkanUtil::DestoryInstance(VkInstance* pInstance,  VkDebugUtilsMessengerEXT* pDebugMsger)
{
    if (pInstance == nullptr)
        return;

    if (pDebugMsger != nullptr)
    {
        auto destroyDebugMsgerFn = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(*pInstance, "vkDestroyDebugUtilsMessengerEXT");
        if (destroyDebugMsgerFn != nullptr)
        {
            std::cout << "--> Destroy Vulkan Debug Messenger" << std::endl;
            (*destroyDebugMsgerFn)(*pInstance, *pDebugMsger, nullptr);
        }
        else 
        {
            std::cout << "--> Vulkan filed to load function: vkDestroyDebugUtilsMessengerEXT!\n\tDestroy debug messenger fiailed." << std::endl;
        }
    }
    
    vkDestroyInstance(*pInstance, nullptr);
    pDebugMsger = nullptr;
    pInstance = nullptr;

}


VKAPI_ATTR VkBool32 VKAPI_CALL VulkanUtil::DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT msgType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData)
{
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