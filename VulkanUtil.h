#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include<string>

using std::vector;
using std::string;

class VulkanUtil
{

public:
    static bool CreateInstance(const vector<string>& enableExtendsions, const vector<string>& enableLayers, VkInstance* pCreatedInstance);

};