#include"ApiSample.h"
#include"core\VulkanInstance.h"
#include"core\Device.h"
#include"core\SwapChain.h"
#include"core\CommandBuffer.h"
#include"rendering\Window.h"
#include"rendering\Mesh.h"

ApiSample::ApiSample(const AppDesc& appDesc)
: Application(appDesc)
{

}


bool ApiSample::Setup()
{
    VkSemaphoreCreateInfo semahoreCreateInfo{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    if (VKCALL_FAILED(vkCreateSemaphore(m_pDevice->GetHandle(), &semahoreCreateInfo, nullptr, &m_SwapChainImageAvalible))
        || VKCALL_FAILED(vkCreateSemaphore(m_pDevice->GetHandle(), &semahoreCreateInfo, nullptr, &m_PresentAvalible)))
    {
        LOGE("-->SampleAPI set up: Failed to create semahore!");
        return false;
    }

    VkFenceCreateInfo fenceCreateInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (VKCALL_FAILED(vkCreateFence(m_pDevice->GetHandle(), &fenceCreateInfo, nullptr, &m_CmdBufferAvalible)))
    {
        LOGE("-->SampleAPI set up: Failed to create fence!");
        return false;
    }

    m_GraphicQueue = m_pDevice->GetGrapicQueue();
    m_PresentQueue = m_pDevice->GetPresentQueue(m_window);
    assert(VKHANDLE_IS_NOT_NULL(m_GraphicQueue) && VKHANDLE_IS_NOT_NULL(m_PresentQueue));

    m_pCmdBuffer = m_pDevice->CreateCommandBuffer(m_GraphicQueue);
    assert(m_pCmdBuffer->IsVaild());
    m_CmdBuffer = m_pCmdBuffer->GetHandle();

    // render pass
    if (!CreateRenderPass())
    {
        LOGE("-->ApiSample: failed to create render pass!");
        return false; 
    }
    
    // swap chain frame buffers
    if (!CreateSwapChainFrameBuffers())
    {
        LOGE("-->ApiSample: failed to create frame buffers!");
        return false; 
    }

    // mesh
    glm::vec3 triVerts[3] = {
        {-1, 0, 0},
        {0, 1, 0},
        {1, 0, 0},
    };

    glm::vec4 triColors[3] = {
        {1, 0, 0, 1},
        {0, 1, 0, 1},
        {0, 0, 1, 1},
    };

    Mesh::index_t triIndices[3] = {0, 1, 2};

    _triangleMesh.reset(new Mesh(m_pDevice.get()));
    _triangleMesh->SetVertices(triVerts, 3);
    _triangleMesh->SetColors(triColors, 3);
    _triangleMesh->SetIndices(triIndices, 3);
    _triangleMesh->SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    assert(_triangleMesh->Apply());

    return true;
}


void ApiSample::Release()
{
    _triangleMesh->Release();

    DestroySwapChainFrameBuffers();

    DestroyRenderPass();

    if (VKHANDLE_IS_NOT_NULL(m_SwapChainImageAvalible))
    {
        vkDestroySemaphore(m_pDevice->GetHandle(), m_SwapChainImageAvalible, nullptr);
        VKHANDLE_SET_NULL(m_SwapChainImageAvalible);
    }
    if (VKHANDLE_IS_NOT_NULL(m_PresentAvalible))
    {
        vkDestroySemaphore(m_pDevice->GetHandle(), m_PresentAvalible, nullptr);
        VKHANDLE_SET_NULL(m_PresentAvalible);
    }
    if (VKHANDLE_IS_NOT_NULL(m_CmdBufferAvalible))
    {
        vkDestroyFence(m_pDevice->GetHandle(), m_CmdBufferAvalible, nullptr);
        VKHANDLE_SET_NULL(m_CmdBufferAvalible);
    }

    if (m_pCmdBuffer)
    {
        m_pDevice->DestroyCommandBuffer(m_pCmdBuffer);
        m_pCmdBuffer = nullptr;
    }
}


void ApiSample::Draw()
{
    // aquire an image in swapchain to render for current frame 
    uint32_t imageIdx = 0;
    THROW_IF_NOT(m_pSwapChain->AquireNextBuffer(m_SwapChainImageAvalible, &imageIdx), 
        "-->SampleAPI Draw: Failed to get swapchain's next image index!");
    
    // wait prev command buffer finish executing
    VKCALL_THROW_IF_FAILED(vkWaitForFences(m_pDevice->GetHandle(), 1, &m_CmdBufferAvalible, true, std::numeric_limits<uint64_t>::max()),
        "-->SampleAPI Draw: Failed to wait command buffer to finish executing!");
    vkResetFences(m_pDevice->GetHandle(), 1, &m_CmdBufferAvalible); // reset to unsignal state
    VKCALL_THROW_IF_FAILED(vkResetCommandBuffer(m_CmdBuffer, 0),"-->SampleAPI Draw: Failed to reset command buffer!");
    
    RecordDrawCommands(imageIdx);

    // submit commands to queue
    VkPipelineStageFlags waitStageMaks[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_CmdBuffer;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &m_SwapChainImageAvalible;
    submitInfo.pWaitDstStageMask = waitStageMaks;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &m_PresentAvalible;
    VKCALL_THROW_IF_FAILED(vkQueueSubmit(m_GraphicQueue, 1, &submitInfo, m_CmdBufferAvalible),"-->SampleAPI Draw: Failed to submit command buffer!");

    //swapchain presentation
    VkSwapchainKHR swapChainHandle = m_pSwapChain->GetHandle();
    VkPresentInfoKHR presentInfo{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChainHandle;
    presentInfo.pImageIndices = &imageIdx;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &m_PresentAvalible;
    presentInfo.pResults = nullptr;
    VKCALL_THROW_IF_FAILED(vkQueuePresentKHR(m_PresentQueue, &presentInfo),"-->SampleAPI Draw: Failed to present!");

}



void ApiSample::RecordDrawCommands(uint32_t swapChainImageIdx)
{
    VkCommandBufferBeginInfo cmdBufBegInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    VKCALL_THROW_IF_FAILED(vkBeginCommandBuffer(m_CmdBuffer, &cmdBufBegInfo), "-->ApiSample Draw: failed to begin command buffer!");

    VkRect2D renderAera{};
    renderAera.extent = m_pSwapChain->GetBufferSize();
    renderAera.offset = {0, 0};

    VkClearColorValue clearColor;
    clearColor.float32[0] = m_appDesc.backBufferClearColor[0];
    clearColor.float32[1] = m_appDesc.backBufferClearColor[1];
    clearColor.float32[2] = m_appDesc.backBufferClearColor[2];
    clearColor.float32[3] = m_appDesc.backBufferClearColor[3];

    VkClearDepthStencilValue clearDepthStencil;
    clearDepthStencil.depth = 0;
    clearDepthStencil.stencil = 0;

    VkClearValue clearSettings[2];
    clearSettings[0].color = clearColor;
    clearSettings[1].depthStencil = clearDepthStencil;

    VkRenderPassBeginInfo renderPassBegInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassBegInfo.framebuffer = m_SwapChainFrameBuffers[swapChainImageIdx];
    renderPassBegInfo.renderPass = m_RenderPass;
    renderPassBegInfo.renderArea = renderAera;
    renderPassBegInfo.clearValueCount = 2;
    renderPassBegInfo.pClearValues = clearSettings;
    vkCmdBeginRenderPass(m_CmdBuffer, &renderPassBegInfo, VK_SUBPASS_CONTENTS_INLINE);
   
    // ***************************** draw call cmds **************************************

    // ***********************************************************************************

    vkCmdEndRenderPass(m_CmdBuffer);

    VKCALL_THROW_IF_FAILED(vkEndCommandBuffer(m_CmdBuffer), "-->ApiSample Draw: failed to end command buffer!");
}


bool ApiSample::CreateRenderPass()
{
    std::vector<VkAttachmentDescription> colorDepthAttachmentsDesc(2);
    colorDepthAttachmentsDesc[0].format = m_pSwapChain->GetBufferFormat().format;
    colorDepthAttachmentsDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
    colorDepthAttachmentsDesc[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorDepthAttachmentsDesc[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    colorDepthAttachmentsDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorDepthAttachmentsDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorDepthAttachmentsDesc[1].format = VK_FORMAT_D24_UNORM_S8_UINT;
    colorDepthAttachmentsDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
    colorDepthAttachmentsDesc[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorDepthAttachmentsDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    colorDepthAttachmentsDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorDepthAttachmentsDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorDepthAttachmentsDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorDepthAttachmentsDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

    VkSubpassDescription subpassDesc{};
    VkAttachmentReference colorAttachmentRef{};
    VkAttachmentReference depthAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    subpassDesc.inputAttachmentCount = 0;
    subpassDesc.pInputAttachments = nullptr;
    subpassDesc.colorAttachmentCount = 1;
    subpassDesc.pColorAttachments = &colorAttachmentRef;
    subpassDesc.pDepthStencilAttachment = &depthAttachmentRef;
    subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDesc.preserveAttachmentCount = 0;
    subpassDesc.pPreserveAttachments = nullptr;
    subpassDesc.pResolveAttachments = nullptr;

    VkRenderPassCreateInfo renderPassCreateInfo{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    renderPassCreateInfo.attachmentCount = 2;
    renderPassCreateInfo.pAttachments = colorDepthAttachmentsDesc.data();
    renderPassCreateInfo.subpassCount = 1;
    renderPassCreateInfo.pSubpasses = &subpassDesc;
    renderPassCreateInfo.dependencyCount = 0;
    renderPassCreateInfo.pDependencies = nullptr;

    return vkCreateRenderPass(m_pDevice->GetHandle(), &renderPassCreateInfo, nullptr, &m_RenderPass) == VK_SUCCESS;
}


void ApiSample::DestroyRenderPass()
{
    if (VKHANDLE_IS_NOT_NULL(m_RenderPass))
    {
        vkDestroyRenderPass(m_pDevice->GetHandle(), m_RenderPass, nullptr);
        VKHANDLE_SET_NULL(m_RenderPass);
    }
}


bool ApiSample::CreateSwapChainFrameBuffers()
{
    // depth buffer
    VkImageCreateInfo depthBufferCreateInfo{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    depthBufferCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    depthBufferCreateInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depthBufferCreateInfo.extent = {m_pSwapChain->GetBufferSize().width, m_pSwapChain->GetBufferSize().height, 1};
    depthBufferCreateInfo.arrayLayers = 1;
    depthBufferCreateInfo.mipLevels = 1;
    depthBufferCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    depthBufferCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthBufferCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthBufferCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depthBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    if( VKCALL_FAILED(vkCreateImage(m_pDevice->GetHandle(), &depthBufferCreateInfo, nullptr, &m_DepthBuffer)))
    {
        LOGE("-->ApiSample: failed to create depth buffer!");
        return false;
    }

    VkMemoryRequirements depthBufferMemReq{};
    vkGetImageMemoryRequirements(m_pDevice->GetHandle(), m_DepthBuffer, &depthBufferMemReq);
    if (!m_pDevice->AllocMemory(depthBufferMemReq, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &m_DepthBufferMemory))
    {
        LOGE("-->ApiSample: failed to alloc device({}) memory!", (void*)m_pDevice.get());
        return false;
    }

    if(VKCALL_FAILED(vkBindImageMemory(m_pDevice->GetHandle(), m_DepthBuffer, m_DepthBufferMemory,  0 /*depthBufferMemReq.alignment*/)))
    {
        LOGE("-->ApiSample: failed to bind device({}) memory!", (void*)m_pDevice.get());
        return false;
    }

    VkImageViewCreateInfo depthBufferViewCreateInfo{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    
    depthBufferViewCreateInfo.image = m_DepthBuffer;
    depthBufferViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthBufferViewCreateInfo.format = VK_FORMAT_D24_UNORM_S8_UINT;
    depthBufferViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    depthBufferViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    depthBufferViewCreateInfo.subresourceRange.layerCount = 1;
    depthBufferViewCreateInfo.subresourceRange.baseMipLevel = 0;
    depthBufferViewCreateInfo.subresourceRange.levelCount = 1;
    if (VKCALL_FAILED(vkCreateImageView(m_pDevice->GetHandle(), &depthBufferViewCreateInfo, nullptr, &m_DepthBufferView)))
    {
        LOGE("-->ApiSample: failed to create depth buffer view!", (void*)m_pDevice.get());
        return false;
    }

    // frame buffer
    m_SwapChainFrameBuffers.resize(m_pSwapChain->GetBufferCount(), VK_NULL_HANDLE);
    for (size_t i = 0; i < m_SwapChainFrameBuffers.size(); i++)
    {
        VkFramebufferCreateInfo fbCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        VkImageView fbAttachments[2]{m_pSwapChain->GetBufferView(i), m_DepthBufferView};
        fbCreateInfo.renderPass = m_RenderPass;
        fbCreateInfo.width = m_pSwapChain->GetBufferSize().width;
        fbCreateInfo.height = m_pSwapChain->GetBufferSize().height;
        fbCreateInfo.layers = 1;
        fbCreateInfo.attachmentCount = 2;
        fbCreateInfo.pAttachments = fbAttachments;
        if (VKCALL_FAILED(vkCreateFramebuffer(m_pDevice->GetHandle(), &fbCreateInfo, nullptr, &m_SwapChainFrameBuffers[i])))
        {   
            LOGE("-->ApiSample: failed to create frame buffer at idx: {}!", i);
            return false;
        }
    }

    return true;
}

void ApiSample::DestroySwapChainFrameBuffers()
{
    for (size_t i = 0; i < m_SwapChainFrameBuffers.size(); i++)
    {
        if (VKHANDLE_IS_NOT_NULL(m_SwapChainFrameBuffers[i]))
        {
            vkDestroyFramebuffer(m_pDevice->GetHandle(), m_SwapChainFrameBuffers[i], nullptr);
            m_SwapChainFrameBuffers[i] = VK_NULL_HANDLE;
        }
    }
    m_SwapChainFrameBuffers.clear();

    if ( VKHANDLE_IS_NOT_NULL(m_DepthBufferView))
    {
        vkDestroyImageView(m_pDevice->GetHandle(), m_DepthBufferView, nullptr);
        VKHANDLE_SET_NULL(m_DepthBufferView);
    }

    if (VKHANDLE_IS_NOT_NULL(m_DepthBuffer))
    {
        vkDestroyImage(m_pDevice->GetHandle(), m_DepthBuffer, nullptr);
        VKHANDLE_SET_NULL(m_DepthBuffer);
    }

    if (VKHANDLE_IS_NOT_NULL(m_DepthBufferMemory))
    {
        m_pDevice->FreeMemory(m_DepthBufferMemory);
        VKHANDLE_SET_NULL(m_DepthBufferMemory);
    }
    
}
