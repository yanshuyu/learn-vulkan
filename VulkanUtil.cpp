#include"VulkanUtil.h"
#include<iostream>
#include<algorithm>

bool VulkanUtil::CreateInstance(const vector<string>& enableExtendsions, const vector<string>& enableLayers, VkInstance* pCreatedInstance)
{
    uint32_t instanceSupportExtCnt = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, nullptr);
    vector<VkExtensionProperties> instanceSupportExtendsionProps(instanceSupportExtCnt);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceSupportExtCnt, instanceSupportExtendsionProps.data());
    std::cout << "-->Detect Vulkan Instance Supported Extendsions: " << instanceSupportExtCnt << std::endl;
    for (const auto& extProp : instanceSupportExtendsionProps)
    {
        std::cout << extProp.extensionName << std::endl; 
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
    

    uint32_t apiVeriosn = 0;
    VkResult result = vkEnumerateInstanceVersion(&apiVeriosn);
    if (result != VK_SUCCESS)
        return false;

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.pApplicationName = "No Name";
    appInfo.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;


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
    

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 0;
    createInfo.ppEnabledExtensionNames = nullptr;
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;
    createInfo.flags = 0;
    
    result = vkCreateInstance(&createInfo, nullptr, pCreatedInstance);

    return result  == VK_SUCCESS;
}