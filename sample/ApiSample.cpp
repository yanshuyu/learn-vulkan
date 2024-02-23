#include"ApiSample.h"
#include"core\VulkanInstance.h"
#include"core\Device.h"
#include"core\SwapChain.h"
#include"core\CommandBuffer.h"
#include"rendering\Window.h"

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

    return true;
}


void ApiSample::Release()
{
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

   // VkRenderPassBeginInfo renderPassBegInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    //vkCmdBeginRenderPass(m_CmdBuffer, )

    VKCALL_THROW_IF_FAILED(vkEndCommandBuffer(m_CmdBuffer), "-->ApiSample Draw: failed to end command buffer!");
    
}
