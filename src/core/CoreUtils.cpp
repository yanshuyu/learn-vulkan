#include"core\CoreUtils.h"


VkImageLayout vkutils_get_render_pass_attachment_best_input_layout(VkFormat fmt)
{
    if (vkutils_is_stencil_only_format(fmt))
        return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    else if (vkutils_is_depth_only_format(fmt))
        return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    else if (vkutils_is_depth_stencil_format)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}


VkImageLayout vkutils_get_render_pass_attachment_best_output_layout(VkFormat fmt)
{
    if (vkutils_is_stencil_only_format(fmt))
        return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    else if (vkutils_is_depth_only_format(fmt))
        return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    else if (vkutils_is_depth_stencil_format)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

void vkutils_toggle_extendsion_or_layer_name_active(std::vector<std::string> &arr, const char *name, bool enabled)
{
    auto pos = std::find(arr.begin(), arr.end(), name);
    if (!enabled && pos != arr.end())
    {
        arr.erase(pos);
    }
    else if (enabled && pos == arr.end())
    {
        arr.push_back(name);
    }
}

size_t vkutils_queue_flags_str(VkQueueFlags flags, char* strbuf, size_t bufSz)
{

    std::memset(strbuf, NULL, bufSz);

    const VkQueueFlagBits allFlagBits[] = {
        VK_QUEUE_GRAPHICS_BIT,
        VK_QUEUE_COMPUTE_BIT,
        VK_QUEUE_TRANSFER_BIT,
        VK_QUEUE_SPARSE_BINDING_BIT,
        VK_QUEUE_PROTECTED_BIT,
        VK_QUEUE_VIDEO_DECODE_BIT_KHR,
        VK_QUEUE_OPTICAL_FLOW_BIT_NV,
    };

    const char* flagBitStrs[] = {
        "G",
        "C",
        "T",
        "SB",
        "P",
        "VD",
        "OF",
    };

    size_t pos = 0;
    for (size_t i = 0; i < 7; i++)
    {
        if (pos >= bufSz)
            break;

        if (flags & allFlagBits[i])
        {
            if (pos > 0)
            {
                strbuf[pos] = '|';
                pos++;
            }

            const char* str = flagBitStrs[i];
            while (*str != NULL && pos < bufSz)
            {
                strbuf[pos] = *str;
                str++;
                pos++;
            }
 
        }
    }
    
    return pos;
}


bool vkutils_fetch_device_feature(const VkPhysicalDeviceFeatures& featureProps, DeviceFeatures feature)
{
    switch (feature)
    {
    case DeviceFeatures::geometryShader:
        return featureProps.geometryShader;

     case DeviceFeatures::tessellationShader:
        return featureProps.tessellationShader;

    case DeviceFeatures::samplerAnisotropy:
        return featureProps.samplerAnisotropy;

    case DeviceFeatures::textureCompressionETC2:
        return featureProps.textureCompressionETC2;   
    
    default:
        return false;
    }
}

uint32_t vkutils_fetch_device_limit(const VkPhysicalDeviceLimits& limitProps, DeviceLimits limit)
{
    switch (limit)
    {
    case DeviceLimits::maxSamplerAnisotropy:
        return limitProps.maxSamplerAnisotropy;
    
    default:
        0;
    }
}