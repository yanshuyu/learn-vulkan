#include"core\Image.h"
#include"core\Device.h"

 Image::Image(Device* pDevice): VKDeviceResource(pDevice), IMapAccessMemory()
{

}

bool Image::Create(const ImageDesc &desc)
{
    VkImageCreateInfo createInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    createInfo.format = desc.format;
    createInfo.imageType = vkutils_get_image_type_form_extents(desc.extents);
    createInfo.extent = desc.extents;
    createInfo.arrayLayers = desc.layers;
    createInfo.samples = vkutils_get_sample_count_flag_bit(desc.sampleCount);
    createInfo.mipLevels = desc.linearTiling ? 1 : desc.mipLeves; // linear tiling image force limit to single mipmap for simplity
    createInfo.usage = desc.usageFlags;
    createInfo.tiling = desc.linearTiling ? VK_IMAGE_TILING_LINEAR : VK_IMAGE_TILING_OPTIMAL;
    createInfo.initialLayout = desc.linearTiling  ? VK_IMAGE_LAYOUT_PREINITIALIZED : VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.flags = desc.flags;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    VkImage createdImage{VK_NULL_HANDLE};
    VkResult result = vkCreateImage(_pDevice->GetHandle(), &createInfo, nullptr, &createdImage);
    if (VKCALL_FAILED(result))
    {
        LOGE("Device({}) create image error({})!", (void *)_pDevice, result);
        return false;
    }

    VkMemoryPropertyFlags memProps = desc.linearTiling ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
                                                       : VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT; // only linear tiling image can access by host directly
    VkMemoryRequirements memReq{};
    vkGetImageMemoryRequirements(_pDevice->GetHandle(), createdImage, &memReq);
    VkDeviceMemory allocedMem{VK_NULL_HANDLE};
    if (!_pDevice->AllocMemory(memReq, memProps, &allocedMem))
    {
        vkDestroyImage(_pDevice->GetHandle(), createdImage, nullptr);
        return false;
    }

    result = vkBindImageMemory(_pDevice->GetHandle(), createdImage, allocedMem, 0);
    if (VKCALL_FAILED(result))
    {
        vkDestroyImage(_pDevice->GetHandle(), createdImage, nullptr);
        _pDevice->FreeMemory(allocedMem);
        LOGE("Image({}) memory({}) bind error({})!", (void *)createdImage, (void *)allocedMem, result);
        return false;
    }

    m_vkImage = createdImage;
    m_ImageMem = allocedMem;
    m_Desc = desc;
    m_Desc.mipLeves = createInfo.mipLevels;
    m_MemSz = memReq.size;
    m_MemProps = memProps;
    m_View = CreateView(desc);
    m_Layout = createInfo.initialLayout;

    return true;
}


VkImageView Image::CreateView(const ImageDesc& desc)
{
    VkImageViewCreateInfo viewInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    viewInfo.viewType = vkutils_get_image_view_type(desc.extents, desc.layers, desc.flags);
    viewInfo.format = desc.format;
    viewInfo.image = m_vkImage;
    viewInfo.subresourceRange.aspectMask = vkutils_get_image_input_asepect_mask(desc.format);
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = desc.layers;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = desc.mipLeves;
    VkImageView createdView{VK_NULL_HANDLE};
    if(VKCALL_FAILED(vkCreateImageView(_pDevice->GetHandle(), &viewInfo, nullptr, &createdView)))
        LOGE("Image failed to create view!");

    return createdView;
}


void Image::SetPixels(const uint8_t* rawData, size_t dataSz, size_t layer)
{
    if (!IsCreate())
        return;

    if (m_Desc.linearTiling)
    {
        Map(Write);
        SetData(rawData, dataSz, dataSz * (layer-1));
        UnMap();
        VkImageSubresourceRange imgLayerRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, layer, 1};
        CommandBuffer* cmd = _pDevice->CreateTempraryCommandBuffer(_pDevice->GetGrapicQueue());
        CommandBuffer::TransitionLayout(cmd, m_vkImage, imgLayerRange, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        _pDevice->DestroyCommandBuffer(cmd);
    }
    else
    {
        // transffer raw pixel data from cpu to a staging buffer
        Buffer* stagingBuffer =  _pDevice->CreateBuffer(dataSz, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        stagingBuffer->Map(Write);
        stagingBuffer->SetData(rawData, dataSz, 0);
        stagingBuffer->UnMap();

        // transffer image at current layer's 0 mip level layout for buffer image copy
        VkImageSubresourceRange imgMipLevelRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, layer, 1};
        CommandBuffer* cmd = _pDevice->CreateTempraryCommandBuffer(_pDevice->GetGrapicQueue());
        cmd->Begin();
        cmd->TransitionLayout(m_vkImage, imgMipLevelRange, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

        // copy pixel to image layer's mip level 0
        VkImageSubresourceLayers imgMipLevelLayer{VK_IMAGE_ASPECT_COLOR_BIT, 0, layer, 1};
        VkBufferImageCopy bufImgcopyInfo;
        bufImgcopyInfo.bufferOffset = 0;
        bufImgcopyInfo.imageExtent = m_Desc.extents;
        bufImgcopyInfo.imageOffset = {0, 0, 0};
        bufImgcopyInfo.imageSubresource = imgMipLevelLayer;
        vkCmdCopyBufferToImage(cmd->GetHandle(), stagingBuffer->GetHandle(), m_vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufImgcopyInfo);
        
        // generate sub mip map levels
        if (m_Desc.mipLeves > 1)
        {
            VkOffset3D srcMipLevelExtent = {m_Desc.extents.width, m_Desc.extents.height, m_Desc.extents.depth};
            for (size_t i = 1; i < m_Desc.mipLeves; i++)
            {
                // transform prev mip level's layout from transfer write to tansfer read
                cmd->TransitionLayout(m_vkImage, imgMipLevelRange, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
               
                // transform cur mip level's layout to tansfer write
                imgMipLevelRange.baseMipLevel = i; // move to cur mip level
                cmd->TransitionLayout(m_vkImage, imgMipLevelRange, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
               
                // blit prev mip level to cur mip level
                VkOffset3D dstMipLevelExtent{};
                dstMipLevelExtent.x = std::max(srcMipLevelExtent.x / 2, 1);
                dstMipLevelExtent.y = std::max(srcMipLevelExtent.y / 2, 1);
                dstMipLevelExtent.z = std::max(srcMipLevelExtent.z / 2, 1);
                VkImageBlit imgBlitInfo{};
                imgBlitInfo.srcSubresource = imgMipLevelLayer;
                imgBlitInfo.srcOffsets[1] = srcMipLevelExtent;
                imgMipLevelLayer.mipLevel = i; // move to cur mip level
                imgBlitInfo.dstSubresource =  imgMipLevelLayer;
                imgBlitInfo.dstOffsets[1] = dstMipLevelExtent;        
                vkCmdBlitImage(cmd->GetHandle(), m_vkImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_vkImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imgBlitInfo, m_Desc.mipLevelFilter);

                srcMipLevelExtent = dstMipLevelExtent;
            }

            // transfer last mip level's layout to transfer src, so that the whole image resource in same layout
            cmd->TransitionLayout(m_vkImage, imgMipLevelRange, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        }

        // transffer image layout for shader sample
        VkImageSubresourceRange imgLayerRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, m_Desc.mipLeves, layer, 1};
        cmd->TransitionLayout(m_vkImage, imgLayerRange, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        cmd->End();
        cmd->ExecuteSync();

        _pDevice->DestroyBuffer(stagingBuffer);
        _pDevice->DestroyCommandBuffer(cmd);
    }

    m_Layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
}


void Image::Release()
{
    if (!IsValid())
        return;

    // release views
    if (VKHANDLE_IS_NOT_NULL(m_View))
    {
        vkDestroyImageView(_pDevice->GetHandle(), m_View, nullptr);
        VKHANDLE_SET_NULL(m_View);
    }

    if (VKHANDLE_IS_NOT_NULL(m_vkImage))
    { 
        vkDestroyImage(_pDevice->GetHandle(), m_vkImage, nullptr);
        VKHANDLE_SET_NULL(m_vkImage);
    }

    if (VKHANDLE_IS_NOT_NULL(m_ImageMem))
    {
        _pDevice->FreeMemory(m_ImageMem);
        VKHANDLE_SET_NULL(m_ImageMem);
        m_MemSz = 0;
    }
    m_Desc = {};
}



VkDevice Image::GetDeviceHandle() const
{
    return _pDevice->GetHandle();
}