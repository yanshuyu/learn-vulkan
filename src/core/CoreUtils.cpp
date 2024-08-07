#include"core\CoreUtils.h"
#include"core\Device.h"
#include<stb\stb_image.h>
#include<fstream>

int vkutils_queue_type_family_index(VkPhysicalDevice phyDevice, QueueType queue, VkQueueFamilyProperties* queueProperties)
{
    uint32_t deviceQueueCnt{0};
    std::vector<VkQueueFamilyProperties> deviceQueueProperties{};
    vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &deviceQueueCnt, nullptr);
    deviceQueueProperties.resize(deviceQueueCnt);
    vkGetPhysicalDeviceQueueFamilyProperties(phyDevice, &deviceQueueCnt, deviceQueueProperties.data());
    VkQueueFlags queueIncludeBits[] = {
        VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
        VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT,
        VK_QUEUE_TRANSFER_BIT,
    };

    VkQueueFlags queueExcludeBits[] = {
        0,
        VK_QUEUE_GRAPHICS_BIT,
        VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT,
    };


    for (int i = 0; i < deviceQueueCnt; i++)
    {
        if (((deviceQueueProperties[i].queueFlags & queueIncludeBits[queue]) == queueIncludeBits[queue])
            && ((deviceQueueProperties[i].queueFlags & queueExcludeBits[queue]) == 0))
        {   
            if (queueProperties)
                *queueProperties = deviceQueueProperties[i]; 
            return i;
        }
    }

    return -1;
}

const char* vkutils_queue_type_str(QueueType queue)
{
    static char* type_queue_str[] = {
        "Main",
        "Compute",
        "Transfer",
        "MaxEnum",
    };

    return type_queue_str[queue];
}

const char* vkutils_device_feature_str(DeviceFeatures feature)
{
    static char* feature_strs[] = {
        "Feature Begin",
        "geometry shader",
        "tessellation shader",
        "sampler anisotropy",
        "texture compression ETC2",
        "Feature End",
    };

    return feature_strs[feature];
}



VkPhysicalDeviceFeatures vkutils_populate_physical_device_feature(const DeviceFeatures* fetrues, size_t featureCnt)
{
    VkPhysicalDeviceFeatures phyDeviceFeatures{};
    for (size_t i = 0; i < featureCnt; i++)
    {
        DeviceFeatures feature = fetrues[i];
        switch (feature)
        {
        case DeviceFeatures::geometryShader:
            phyDeviceFeatures.geometryShader = VK_TRUE;
            break;
        case DeviceFeatures::tessellationShader:
            phyDeviceFeatures.tessellationShader = VK_TRUE;
            break;
        case DeviceFeatures::samplerAnisotropy:
            phyDeviceFeatures.samplerAnisotropy = VK_TRUE;
            break;
        case DeviceFeatures::textureCompressionETC2:
            phyDeviceFeatures.textureCompressionETC2 = VK_TRUE;
            break;

        default:
            break;
        }
    }

    return phyDeviceFeatures;
}


VkImageAspectFlags vkutils_get_image_input_asepect_mask(VkFormat fmt)
{
    if (vkutils_is_color_format(fmt))
        return VK_IMAGE_ASPECT_COLOR_BIT;
    if (vkutils_is_depth_only_format(fmt))
        return VK_IMAGE_ASPECT_DEPTH_BIT;
    if (vkutils_is_stencil_only_format(fmt))
        return VK_IMAGE_ASPECT_STENCIL_BIT;

    return VK_IMAGE_ASPECT_DEPTH_BIT;
}


VkImageAspectFlags vkutils_get_image_output_asepect_mask(VkFormat fmt)
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
    uint32_t x =  std::max({extents.width, extents.height, extents.depth});
    x = std::log2(x);
    return x + 1;
}


VkImageUsageFlags vkutils_get_render_texture_usage_flags(VkFormat fmt, bool sampled, bool persistable)
{
    VkImageUsageFlags flags{0};
    if (vkutils_is_depth_or_stencil_format(fmt))
        flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    else 
        flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    
    if (sampled)
        flags |= VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    
    if (persistable)
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    
    return flags;
}


VkImageViewType vkutils_get_image_view_type(VkExtent3D extents, size_t layers, VkImageCreateFlags flags)
{
    if (flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT && layers % 6 == 0)
    {
        return layers / 6 > 1 ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_CUBE;
    }

    VkImageViewType allViewTypes[] = {
        VK_IMAGE_VIEW_TYPE_1D,
        VK_IMAGE_VIEW_TYPE_1D_ARRAY,
        VK_IMAGE_VIEW_TYPE_2D,
        VK_IMAGE_VIEW_TYPE_2D_ARRAY,
        VK_IMAGE_VIEW_TYPE_3D,
        VK_IMAGE_VIEW_TYPE_MAX_ENUM,
    };

    size_t idx = 0;
    if (extents.height > 1)
        idx++;
    if (extents.depth > 1)
        idx++;
    
    idx *= 2;

    if (layers > 1)
        idx++;

    return allViewTypes[idx];
}


VkSampleCountFlagBits vkutils_sample_count_to_flag_bit(uint32_t sampleCnt)
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


size_t vkutils_sample_flag_bit_to_count(VkSampleCountFlagBits sample)
{
    int cnt{0};
    while (sample & ( 1 << cnt++) == 0)
    return cnt;
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

const char* vkutils_queue_flags_str(VkQueueFlags flags)
{
    constexpr int str_buf_sz = 256;
    static char str_buf[str_buf_sz]{};

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

    memset(str_buf, NULL, str_buf_sz);
    size_t pos = 0;
    for (size_t i = 0; i < 7; i++)
    {
        if (pos >= str_buf_sz)
            break;

        if (flags & allFlagBits[i])
        {
            if (pos > 0)
            {
                str_buf[pos] = '|';
                pos++;
            }

            const char* str = flagBitStrs[i];
            while (*str != NULL && pos < str_buf_sz)
            {
                str_buf[pos] = *str;
                str++;
                pos++;
            }
 
        }
    }
    
    if (pos < str_buf_sz)
        pos++;
    
    str_buf[pos] = NULL;

    return str_buf;
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
    
    case DeviceLimits::maxMipLodBias:
        return limitProps.maxSamplerLodBias;

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

VkShaderStageFlagBits vkutils_get_shader_stage_bit_from_file_extendsion(const char* ext)
{
    const char* extNames[] = {
        ".vert",
        ".frag",
        ".geom",
        ".tesc",
        ".tese",
        ".comp",
    };


    VkShaderStageFlagBits shaderStageBits[] = {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
        VK_SHADER_STAGE_GEOMETRY_BIT,
        VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        VK_SHADER_STAGE_COMPUTE_BIT,
    };

    size_t idx = 0;
    for (auto &&_ext : extNames)
    {
        if (strcmp(_ext, ext))
            return shaderStageBits[idx];
        idx++;
    }
    
    return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
    
}


float vkutils_remap_anisotropy_level(float anisotropy01, const Device* pdevice)
{
    if (!pdevice->IsFeatureEnabled(DeviceFeatures::samplerAnisotropy))
        return 0;
    
    float maxAnisotropy = pdevice->GetDeviceLimit(DeviceLimits::maxSamplerAnisotropy);
    return std::clamp(anisotropy01, 0.f, 1.f) * maxAnisotropy;
}


uint8_t* vkutils_stb_load_texture(Device* pdevice, const char* srcFile, bool srgb, bool readWriteEnable, int* w, int* h, VkFormat* fmt, int* dataSz)
{
    std::string fullPath(ASSETS_DIR);
    fullPath += srcFile;
    int _w, _h, c;
    if (!stbi_info(fullPath.c_str(), &_w, &_h, &c))
        return nullptr;

    //     channels     components
    //       1           grey
    //       2           grey, alpha (vulkan uncompatitble, force to load at x-x-x-1 fmt)
    //       3           red, green, blue (vulkan unsupport, force to load at r-g-b-1 fmt)
    //       4           red, green, blue, alpha

    VkFormat fmts[] = {
        VK_FORMAT_R8_UNORM,
        VK_FORMAT_R8_SRGB,
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_FORMAT_R8G8B8A8_SRGB,
    };

    if (c > 1 && c < 4)
        c = 4;

    int fmtIdx = (int)(c / 2.f) + srgb;
    if (!pdevice->IsFormatFeatureSupport(fmts[fmtIdx], VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT, readWriteEnable))
    {
        LOGE("load texture({}) of fmt({}) is not support!", srcFile, fmts[fmtIdx]);
        return nullptr;
    }

    int _c;
    uint8_t* pdata = stbi_load(fullPath.c_str(), &_w, &_h, &_c, c);
    assert(pdata);

    *w = _w;
    *h = _h;
    *fmt = fmts[fmtIdx];
    *dataSz = _w * _h * c;
    return pdata;
}

file_bytes futils_read_file_bytes(const char *filePath)
{
    file_bytes result{};
    std::ifstream fs(filePath, std::ios_base::ate | std::ios_base::binary);
    if (!fs.is_open())
        return result;

    size_t srcSz = fs.tellg();
    result.byteCnt = srcSz;
    result.bytes = (char*)malloc(srcSz+1);
    memset(result.bytes, NULL, srcSz+1);

    fs.seekg(std::ios_base::beg);
    fs.read(result.bytes, srcSz);
    fs.close();

    return result;
}

void futils_free_file_bytes(file_bytes& fb)
{
    if (fb.bytes && fb.byteCnt > 0)
    {
        free(fb.bytes);
        fb.bytes = nullptr;
        fb.byteCnt = 0;
    }
}

void futils_dump_file_bytes(const file_bytes& fb)
{
    for (size_t i = 0; i < fb.byteCnt; i++)
    {
        LOGI("{}", fb.bytes[i]);
    }
    
}