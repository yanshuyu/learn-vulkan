#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include"spdlog\spdlog.h"
#include<stdexcept>


// Macros
#define NONE_COPYABLE(ClassName) ClassName(const ClassName &) = delete; \
ClassName & operator = (const ClassName &) = delete;

#define NONE_MOVEABLE(ClassName) ClassName(ClassName&&) = delete; \
ClassName operator = (ClassName&&) = delete;

#define NONE_COPYABLE_NONE_MOVEABLE(ClassName) ClassName(const ClassName &) = delete; \
ClassName & operator = (const ClassName &) = delete; \
ClassName(ClassName&&) = delete; \
ClassName operator = (ClassName&&) = delete;

#define VKHANDLE_IS_NULL(vkHandle) (vkHandle == VK_NULL_HANDLE)
#define VKHANDLE_IS_NOT_NULL(vkHandle) (vkHandle != VK_NULL_HANDLE)
#define VKHANDLE_SET_NULL(vkHandle) (vkHandle = VK_NULL_HANDLE)
#define VKCALL_SUCCESS(vkResult) (vkResult == VK_SUCCESS)
#define VKCALL_FAILED(vkResult) (vkResult != VK_SUCCESS)
#define VKCALL_THROW_IF_FAILED(vkResult, msg) if (vkResult != VK_SUCCESS) throw std::runtime_error(msg)
#define THROW_IF(boolean, msg) if(boolean) throw std::runtime_error(msg)
#define THROW_IF_NOT(boolean, msg) if(!boolean) throw std::runtime_error(msg)


// loging
#define ROOT_PATH_SIZE 64
#define __FILENAME__ (static_cast<const char *>(__FILE__) + ROOT_PATH_SIZE)

#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) spdlog::error("[{}:{}] {}", __FILENAME__, __LINE__, fmt::format(__VA_ARGS__));
#define LOGD(...) spdlog::debug(__VA_ARGS__);

// Types
enum QueueType
{
    Main,  // grapic | compute | transfer
    Compute, // compute | transfer 
    Transfer, // transfer 
    MaxQueueType,
};

enum DeviceFeatures
{
    FeatureBegin,
    geometryShader,
    tessellationShader,
    samplerAnisotropy,
    textureCompressionETC2,
    // will add more
    FeatureEnd,
};

enum DeviceLimits
{
    LimitsBegin,
    maxSamplerAnisotropy,
    maxMipLodBias,
    maxFrameBufferColorSampleCount,
    maxFrameBufferDepthSampleCount,
    // will add more
    LimitsEnd,
};

enum VertexAttribute
{
    Position,
    Normal,
    Tangent,
    Color,
    UV0,
    UV1,
    MaxAttribute,
};

typedef uint32_t index_t;

enum BlendMode
{
    None,
    Alpha,
    Additive,
    Multiply,
    MaxBlendMode,
};

enum ColorMask
{
    R,
    RG,
    RGB,
    RGBA,
    A,
};

struct Rect
{
    float x;
    float y;
    float width;
    float height;
};


struct TextureImportSetting
{
    bool genMipMap;
    bool srgbImage;
    bool readWriteEnable;
    float anisotropyFactor;
    float mipLodBias;
    VkFilter mipLevelFilterMode;
    VkFilter filterMode;
    VkSamplerMipmapMode mipMapMode;
    VkSamplerAddressMode uAddressMode;
    VkSamplerAddressMode vAddressMode;
    VkSamplerAddressMode wAddressMode;
    VkBorderColor borderColor;
};

struct file_bytes
{
    char* bytes{nullptr};
    size_t byteCnt{0};
};


enum SetIndices
{
    PerFrame,
    PerCamera,
    PerMaterial,
    PerObject,
    MaxSetIndex,
};



int vkutils_queue_type_family_index(VkPhysicalDevice phyDevice, QueueType queue, VkQueueFamilyProperties* queueProperties = nullptr);

const char* vkutils_queue_type_str(QueueType queue);

const char* vkutils_device_feature_str(DeviceFeatures feature);

VkPhysicalDeviceFeatures vkutils_populate_physical_device_feature(const DeviceFeatures* fetrues, size_t featureCnt);

inline bool vkutils_is_depth_and_stencil_format(VkFormat fmt)
{
    return fmt == VK_FORMAT_D16_UNORM_S8_UINT  // DS
            || fmt == VK_FORMAT_D24_UNORM_S8_UINT
            || fmt == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

inline bool vkutils_is_depth_only_format(VkFormat fmt)
{
    return fmt == VK_FORMAT_D16_UNORM // D
            || fmt == VK_FORMAT_D32_SFLOAT
            || fmt == VK_FORMAT_X8_D24_UNORM_PACK32;
}


inline bool vkutils_is_depth_format(VkFormat fmt)
{
    return vkutils_is_depth_and_stencil_format(fmt) // DS or D
            || vkutils_is_depth_only_format(fmt);
}

inline bool vkutils_is_stencil_only_format(VkFormat fmt)
{
    return fmt == VK_FORMAT_S8_UINT; // S
}

inline bool vkutils_is_stencil_format(VkFormat fmt)
{
    return vkutils_is_depth_and_stencil_format(fmt) // DS or S
            || vkutils_is_stencil_only_format(fmt); 
}

inline bool vkutils_is_depth_or_stencil_format(VkFormat fmt)
{
    return vkutils_is_depth_format(fmt) || vkutils_is_stencil_format(fmt);
}

inline bool vkutils_is_depth_and_stencil_layout(VkImageLayout layout)
{
    return layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL
        || layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL
        || layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL
        || layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
}

inline bool vkutils_is_color_format(VkFormat fmt)
{
    return !vkutils_is_depth_or_stencil_format(fmt);
}

inline bool vkutils_is_depth_only_layout(VkImageLayout layout)
{
    return layout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL 
        || layout == VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
}

inline bool vkutils_is_stencil_only_layout(VkImageLayout layout)
{
    return layout == VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL
        || layout == VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
}

inline bool vkutils_is_depth_layout(VkImageLayout layout)
{
    return vkutils_is_depth_only_layout(layout) || vkutils_is_depth_and_stencil_layout(layout);
}

inline bool vkutils_is_stencil_layout(VkImageLayout layout)
{
    return vkutils_is_stencil_only_layout(layout) || vkutils_is_depth_and_stencil_layout(layout);
}

inline bool vkutils_is_depth_or_stencil_layout(VkImageLayout layout)
{
    return vkutils_is_depth_layout(layout) || vkutils_is_stencil_layout(layout);
}

inline bool vkutils_is_initailable_layout(VkImageLayout layout)
{
    return layout == VK_IMAGE_LAYOUT_UNDEFINED 
        || layout == VK_IMAGE_LAYOUT_PREINITIALIZED;
}

VkImageAspectFlags vkutils_get_image_input_asepect_mask(VkFormat fmt);

VkImageAspectFlags vkutils_get_image_output_asepect_mask(VkFormat fmt);

VkImageLayout vkutils_get_render_pass_attachment_best_input_layout(VkFormat fmt);


VkImageLayout vkutils_get_render_pass_attachment_best_output_layout(VkFormat fmt);

VkImageType vkutils_get_image_type_form_extents(VkExtent3D extents);

uint32_t vkutils_get_mip_level_count_from_extents(VkExtent3D extents);

VkImageViewType vkutils_get_image_view_type(VkExtent3D extents, size_t layers = 1, VkImageCreateFlags flags = 0);

VkSampleCountFlagBits vkutils_get_sample_count_flag_bit(uint32_t sampleCnt);

void vkutils_toggle_extendsion_or_layer_name_active(std::vector<std::string>& arr, const char* name, bool enabled);


const char* vkutils_queue_flags_str(VkQueueFlags flags);

bool vkutils_fetch_device_feature(const VkPhysicalDeviceFeatures& featureProps, DeviceFeatures feature);

uint32_t vkutils_fetch_device_limit(const VkPhysicalDeviceLimits& limitProps, DeviceLimits limit);

uint32_t vkutils_fetch_max_sample_count(VkSampleCountFlags sampleCountFlags);

VkShaderStageFlagBits vkutils_get_shader_stage_bit_from_file_extendsion(const char* ext);

class Device;
float vkutils_remap_anisotropy_level(float anisotropy01, const Device* pdevice);

uint8_t* vkutils_stb_load_texture(Device* pdevice, const char* srcFile, bool srgb, bool readWriteEnable, int* w, int* h, VkFormat* fmt, int* dataSz);


file_bytes futils_read_file_bytes(const char* filePath);

void futils_free_file_bytes(file_bytes& fb);

void futils_dump_file_bytes(const file_bytes& fb);