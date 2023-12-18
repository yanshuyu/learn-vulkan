#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include<string>

using std::vector;
using std::string;

class VulkanUtil
{
private:
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugMessengerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT msgSeverity,
                                                                VkDebugUtilsMessageTypeFlagsEXT msgType,
                                                                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                                void* pUserData); 

public:
    static const char* KHRONOS_STANDARD_VALIDATIONLAYER_NAME;

    static bool CreateInstance(const vector<string>& enableExtendsions, 
                                const vector<string>& enableLayers,
                                 VkInstance* pCreatedInstance, 
                                 VkDebugUtilsMessengerEXT* pCreatedDebugMsger);

    static void DestoryInstance(VkInstance* pInstance, VkDebugUtilsMessengerEXT* pDebugMsger);

};