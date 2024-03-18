#include"core\CoreUtils.h"

VkImageAspectFlags vkutils_get_image_asepect_mask(VkFormat fmt)
{
    if (vkutils_is_color_format(fmt))
        return VK_IMAGE_ASPECT_COLOR_BIT;
    if (vkutils_is_depth_only_format(fmt))
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkutils_is_stencil_only_format(fmt))
        return VK_IMAGE_ASPECT_STENCIL_BIT;

    return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
}

VkImageLayout vkutils_get_render_pass_attachment_best_input_layout(VkFormat fmt)
{
    if (vkutils_is_stencil_only_format(fmt))
        return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
    else if (vkutils_is_depth_only_format(fmt))
        return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
    else if (vkutils_is_depth_and_stencil_format)
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}


VkImageLayout vkutils_get_render_pass_attachment_best_output_layout(VkFormat fmt)
{
    if (vkutils_is_stencil_only_format(fmt))
        return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
    else if (vkutils_is_depth_only_format(fmt))
        return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    else if (vkutils_is_depth_and_stencil_format(fmt))
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
}

VkImageType vkutils_get_image_type_form_extents(VkExtent3D extents)
{
    if (extents.depth > 1)
        return VK_IMAGE_TYPE_3D;
    
    if (extents.height > 1)
        return VK_IMAGE_TYPE_2D;
    
    return VK_IMAGE_TYPE_1D;
}


uint32_t vkutils_get_mip_level_count_from_extents(VkExtent3D extents)
{
    uint32_t x = std::max(extents.width, extents.height);
    x = std::log2(x);
    return x + 1;
}


VkSampleCountFlagBits vkutils_get_sample_count_flag_bit(uint32_t sampleCnt)
{
    VkSampleCountFlagBits sampleCntBits[] = {
        VK_SAMPLE_COUNT_1_BIT,
        VK_SAMPLE_COUNT_2_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_64_BIT,
    };

    return sampleCntBits[(uint32_t)std::log2(sampleCnt)];
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
    
    case DeviceLimits::maxFrameBufferColorSampleCount:
        return vkutils_fetch_max_sample_count(limitProps.framebufferColorSampleCounts);
    
    case DeviceLimits::maxFrameBufferDepthSampleCount:
        return vkutils_fetch_max_sample_count(limitProps.framebufferDepthSampleCounts);
    
    default:
        return 0;
    }
}


uint32_t vkutils_fetch_max_sample_count(VkSampleCountFlags sampleCountFlags)
{
    VkSampleCountFlagBits sampleCountBits[] = {
        VK_SAMPLE_COUNT_1_BIT,
        VK_SAMPLE_COUNT_2_BIT,
        VK_SAMPLE_COUNT_4_BIT,
        VK_SAMPLE_COUNT_8_BIT,
        VK_SAMPLE_COUNT_16_BIT,
        VK_SAMPLE_COUNT_32_BIT,
        VK_SAMPLE_COUNT_64_BIT,
    };

    for (size_t i = 6; i >= 0; i--)
    {
        if (sampleCountFlags & sampleCountBits[i])
            return std::pow(2, i);
    }
    
    return 1;
}