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