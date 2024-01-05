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