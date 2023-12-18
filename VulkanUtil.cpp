#include"VulkanUtil.h"
#include<iostream>
#include<algorithm>

bool VulkanUtil::CreateInstance(const vector<string>& enableExtendsions, const vector<string>& enableLayers, VkInstance* pCreatedInstance)
{
    // verify all extendsions are supported 
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
    

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extendsionsNames.size());
    createInfo.ppEnabledExtensionNames = extendsionsNames.data();
    createInfo.enabledLayerCount = static_cast<uint32_t>(layerNames.size());
    createInfo.ppEnabledLayerNames = layerNames.data();
    createInfo.flags = 0;
    result = vkCreateInstance(&createInfo, nullptr, pCreatedInstance);

    return result  == VK_SUCCESS;
}


void VulkanUtil::DestoryInstance(VkInstance* pInstance)
{
    if (pInstance == nullptr)
        return;
    
    vkDestroyInstance(*pInstance, nullptr);
    pInstance = nullptr;

}