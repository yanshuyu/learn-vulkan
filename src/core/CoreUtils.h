#pragma once
#include<vulkan\vulkan.h>
#include<vector>
#include"spdlog\spdlog.h"

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

// loging
#define ROOT_PATH_SIZE 64
#define __FILENAME__ (static_cast<const char *>(__FILE__) + ROOT_PATH_SIZE)

#define LOGI(...) spdlog::info(__VA_ARGS__);
#define LOGW(...) spdlog::warn(__VA_ARGS__);
#define LOGE(...) spdlog::error("[{}:{}] {}", __FILENAME__, __LINE__, fmt::format(__VA_ARGS__));
#define LOGD(...) spdlog::debug(__VA_ARGS__);

// Types

enum HardwareFeature
{
    none,
    geometryShader,
    tessellationShader,
    samplerAnisotropy,
    textureCompressionETC2,
    // will add more
};

enum DeviceJobOperation
{
    grapic,
    compute,
    transfer,
    present,
};


struct AttachmentDesc
{
    VkFormat format{VK_FORMAT_UNDEFINED};
    VkSampleCountFlagBits sampleCount{VK_SAMPLE_COUNT_1_BIT};
};

struct AttachmentLoadStoreAction
{
    VkAttachmentLoadOp colorDepthLoadAction{VK_ATTACHMENT_LOAD_OP_CLEAR};
    VkAttachmentStoreOp colorDepthStoreAction{VK_ATTACHMENT_STORE_OP_STORE};
    VkAttachmentLoadOp stencilLoadAction{VK_ATTACHMENT_LOAD_OP_CLEAR};
    VkAttachmentStoreOp stencilStoreAction{VK_ATTACHMENT_STORE_OP_STORE};
};

struct SubPassInputOutPutDesc
{
    std::vector<int> inputAttachmentIndices{};
    std::vector<int> outputAttachmentIndices{};

};




// Functions
inline bool vkutils_is_depth_stencil_format(VkFormat fmt)
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
    return vkutils_is_depth_stencil_format(fmt) // DS or D
            || vkutils_is_depth_only_format(fmt);
}

inline bool vkutils_is_stencil_only_format(VkFormat fmt)
{
    return fmt == VK_FORMAT_S8_UINT; // S
}

inline bool vkutils_is_stencil_format(VkFormat fmt)
{
    return vkutils_is_depth_stencil_format(fmt) // DS or S
            || vkutils_is_stencil_only_format(fmt); 
}

inline bool vkutils_is_depth_or_stencil_format(VkFormat fmt)
{
    return vkutils_is_depth_format(fmt) || vkutils_is_stencil_format(fmt);
}

VkImageLayout vkutils_get_render_pass_attachment_best_input_layout(VkFormat fmt);


VkImageLayout vkutils_get_render_pass_attachment_best_output_layout(VkFormat fmt);


void vkutils_toggle_extendsion_or_layer_name_active(std::vector<std::string>& arr, const char* name, bool enabled);
