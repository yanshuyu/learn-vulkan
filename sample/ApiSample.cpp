#include"ApiSample.h"
#include<core\VulkanInstance.h>
#include<core\Device.h>
#include<core\SwapChain.h>
#include<core\CommandBuffer.h>
#include<core\ShaderProgram.h>
#include<core\GraphicPipeline.h>
#include<core\RenderPass.h>
#include<core\FrameBuffer.h>
#include"rendering\Window.h"
#include<rendering\Mesh.h>
#include<rendering\AssetsManager.h>
#include<rendering\DescriptorSetManager.h>
#include<rendering\Texture2D.h>
#include<rendering\TextureCube.h>
#include<rendering\RenderData.h>
#include<rendering\RenderTexture.h>
#include<rendering\Material.h>
#include<rendering\PipelineManager.h>
#include<glm\gtc\matrix_transform.hpp>
#include"input\InputManager.h"

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

    m_GraphicQueue = m_pDevice->GetMainQueue();
    m_PresentQueue = m_pDevice->GetMainQueue();
    assert(VKHANDLE_IS_NOT_NULL(m_GraphicQueue) && VKHANDLE_IS_NOT_NULL(m_PresentQueue));

    m_pCmdBuffer = m_pDevice->CreateCommandBuffer(m_GraphicQueue);
    assert(m_pCmdBuffer->IsValid());
    m_CmdBuffer = m_pCmdBuffer->GetHandle();


    // render pass
    _renderPass.reset(new RenderPass(m_pDevice.get()));
    _renderPass->AddColorAttachment(m_pSwapChain->GetBufferFormat().format, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    _renderPass->AddDepthStencilAttachment(VK_FORMAT_D24_UNORM_S8_UINT, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE, VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_DONT_CARE);
    size_t outputAttachments[] = {0 , 1};
    _renderPass->AddSubPass(outputAttachments, 2);
    assert(_renderPass->Apply());

    // swap chain frame buffers
    if (!CreateSwapChainFrameBuffers())
    {
        LOGE("-->ApiSample: failed to create frame buffers!");
        return false; 
    }

    _perFrameData.reset(new PerFrameData(m_pDevice.get()));
    _perCameraData.reset(new PerCameraData(m_pDevice.get()));

    _set_up_quad();
    _set_up_sky_box();

    return true;
}



void ApiSample::Release()
{
    _clean_up_quad();
    _clean_up_sky_box();

    _perCameraData->Release();
    _perFrameData->Release();
   
    _renderPass->Release();
   
    DestroySwapChainFrameBuffers();

  
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


void ApiSample::Step()
{
    GameTimer::StartFrame();
    Update();
    Draw();
    GameTimer::EndFrame();
}


void ApiSample::Update()
{
    _camera.Update();

    _perFrameData->detalTime = GameTimer::GetDeltaTime();
    _perFrameData->detalTimeOver10 = _perFrameData->detalTime / 10;
    _perFrameData->totalTime = GameTimer::GetTotalSeconds();
    _perFrameData->sinTotalTime = std::sin(_perFrameData->totalTime);
    _perFrameData->UpdateDataBuffer();


    glm::mat4 V = _camera.GetViewMatrix();
    glm::mat4 P = glm::perspectiveFov(glm::radians(30.f), (float)m_window->GetWidth(), (float)m_window->GetHeight(), 0.01f, 500.f);

    _perCameraData->viewMatrix = V;
    _perCameraData->projectionMatrix = P;
    _perCameraData->viewProjectionMatrix = P * V;
    _perCameraData->invViewMatrix = glm::inverse(V);
    _perCameraData->invViewProjectionMatrix = glm::inverse(_perCameraData->viewProjectionMatrix);
    _perCameraData->UpdateDataBuffer();

    static char _titleStr[256];
    sprintf(_titleStr, "%s - Fps: %.1f - Ave Fps: %.1f - Delta: %f", m_window->GetDesc().name,
            GameTimer::GetFps(),
            GameTimer::GetAveFps(),
            GameTimer::GetDeltaTime());
    m_window->SetTitle(_titleStr);

    static int sign = 1;
    if (_quadMainColor.g > 1 || _quadMainColor.g < 0)
    {    
        sign *= -1;
        _quadMainColor.g = std::clamp(_quadMainColor.g, 0.f, 1.f);
    }

    _quadMainColor.g += _perFrameData->detalTime * sign * 0.5f;
    //LOGI("color -> ({}, {}, {}, {})", _quadMainColor.r, _quadMainColor.g, _quadMainColor.b, _quadMainColor.a);

    _quad_mat->SetVector("_mainColor", _quadMainColor);

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
    
    Material::Update();
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
    clearDepthStencil.depth = 1;
    clearDepthStencil.stencil = 0;

    VkClearValue clearSettings[2];
    clearSettings[0].color = clearColor;
    clearSettings[1].depthStencil = clearDepthStencil;

    VkRenderPassBeginInfo renderPassBegInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassBegInfo.renderPass = _renderPass->GetHandle();
    renderPassBegInfo.framebuffer = m_SwapChainFrameBuffers[swapChainImageIdx]->GetHandle();
    renderPassBegInfo.renderArea = renderAera;
    renderPassBegInfo.clearValueCount = m_SwapChainFrameBuffers[swapChainImageIdx]->GetRenderTargetCount();
    renderPassBegInfo.pClearValues = m_SwapChainFrameBuffers[swapChainImageIdx]->GetRenderTargetClears();
    vkCmdBeginRenderPass(m_CmdBuffer, &renderPassBegInfo, VK_SUBPASS_CONTENTS_INLINE);
   
    static VkDeviceSize attrBindingZeroOffsets[MaxAttribute] = {0, 0, 0, 0, 0, 0};
    // ***************************** draw call cmds **************************************
    float w = m_window->GetDesc().windowWidth;
    float h = m_window->GetDesc().windowHeight;
    VkViewport viewport{};
    VkRect2D scissor{};
    viewport.x = 0;
    viewport.y = h;
    viewport.width = w;
    viewport.height = -h;
    viewport.minDepth = 0;
    viewport.maxDepth = 1;
    scissor.offset = {0, 0};
    scissor.extent = {(uint32_t)w, (uint32_t)h};
    vkCmdSetViewport(m_CmdBuffer, 0, 1, &viewport);
    vkCmdSetScissor(m_CmdBuffer, 0, 1, &scissor);
    //vkCmdSetRasterizationSamplesEXT(m_CmdBuffer, VK_SAMPLE_COUNT_1_BIT);
    
    vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PerFrameData::sPipelineLayout, PerFrame, 1, &_perFrameData->dataSet, 0, nullptr);
    vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, PerCameraData::sPipelineLayout, PerCamera, 1, &_perCameraData->dataSet, 0, nullptr);
        
    _skyboxMat->Bind(m_CmdBuffer);
    //vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _skyboxProgram->GetPipelineLayout(), PerMaterial, 1, &_skyboxSet, 0, nullptr);
    vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _skyboxProgram->GetPipelineLayout(), PerObject, 1, &_skyboxInstanceData->dataSet, 0, nullptr);
    vkCmdBindPipeline(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _skyboxPipeline->GetHandle());
    vkCmdBindVertexBuffers(m_CmdBuffer, 0, _cube->GetAttributeCount(), _cube->GetAttributeBindingHandls(), attrBindingZeroOffsets);
    vkCmdBindIndexBuffer(m_CmdBuffer, _cube->GetIndexBuffer()->GetHandle(), 0, _cube->GetIndexType());
    vkCmdDrawIndexed(m_CmdBuffer, _cube->GetIndicesCount(), 1, 0, 0, 0);
    
    _quad_mat->Bind(m_CmdBuffer);
    //vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vertColorProgram->GetPipelineLayout(), PerMaterial, 1, &_quadSet, 0, nullptr);
    vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vertColorProgram->GetPipelineLayout(), PerObject, 1, &_quadInstanceData->dataSet, 0, nullptr);
    vkCmdBindPipeline(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _quadPipeline->GetHandle());
    vkCmdBindVertexBuffers(m_CmdBuffer, 0, _quad->GetAttributeCount(), _quad->GetAttributeBindingHandls(), attrBindingZeroOffsets);
    vkCmdBindIndexBuffer(m_CmdBuffer, _quad->GetIndexBuffer()->GetHandle(), 0, _quad->GetIndexType());
    vkCmdDrawIndexed(m_CmdBuffer, _quad->GetIndicesCount(), 1, 0, 0, 0);
    
    vkCmdBindDescriptorSets(m_CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vertColorProgram->GetPipelineLayout(), PerObject, 1, &_skyboxInstanceData->dataSet, 0, nullptr);
    vkCmdBindVertexBuffers(m_CmdBuffer, 0, _cube->GetAttributeCount(), _cube->GetAttributeBindingHandls(), attrBindingZeroOffsets);
    vkCmdBindIndexBuffer(m_CmdBuffer, _cube->GetIndexBuffer()->GetHandle(), 0, _cube->GetIndexType());
    vkCmdDrawIndexed(m_CmdBuffer, _cube->GetIndicesCount(), 1, 0, 0, 0);


    // ***********************************************************************************

    vkCmdEndRenderPass(m_CmdBuffer);

    VKCALL_THROW_IF_FAILED(vkEndCommandBuffer(m_CmdBuffer), "-->ApiSample Draw: failed to end command buffer!");
}



bool ApiSample::CreateSwapChainFrameBuffers()
{
    // depth buffer
    /*
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

    if(VKCALL_FAILED(vkBindImageMemory(m_pDevice->GetHandle(), m_DepthBuffer, m_DepthBufferMemory,  0 )))
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
    */
    m_DepthRT.reset(new RenderTexture(m_pDevice.get(), RenderTextureDesc(VK_FORMAT_D24_UNORM_S8_UINT, m_pSwapChain->GetBufferSize().width, m_pSwapChain->GetBufferSize().height)));
    RenderTarget depthRT = RenderTarget(m_DepthRT->GetView(), m_DepthRT->GetFormat(), m_DepthRT->GetWidth(), m_DepthRT->GetHeight());
    // frame buffer
    m_SwapChainFrameBuffers.reserve(m_pSwapChain->GetBufferCount());
    for (size_t i = 0; i < m_pSwapChain->GetBufferCount(); i++)
    {
        m_SwapChainFrameBuffers.emplace_back(new FrameBuffer(m_pSwapChain->GetBufferSize().width, m_pSwapChain->GetBufferSize().height));
        RenderTarget colorRT = RenderTarget(m_pSwapChain->GetBufferView(i), m_pSwapChain->GetBufferFormat().format, m_pSwapChain->GetBufferSize().width, m_pSwapChain->GetBufferSize().height);
        m_SwapChainFrameBuffers[i]->SetRenderTarget(colorRT, depthRT);
        if(!m_SwapChainFrameBuffers[i]->Create(_renderPass.get()))
        {    
            LOGE("Create swap chian buffer({})'s frame buffer error", i); 
            return false;
        }
    }

    return true;
}

void ApiSample::DestroySwapChainFrameBuffers()
{

    m_SwapChainFrameBuffers.clear();
    m_DepthRT->Release();
    /*
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
    */
    
}



void ApiSample::_set_up_quad()
{
// quad mesh
    glm::vec3 verts[] = {
        {-0.5, -0.5, 0},
        {-0.5, 0.5, 0},
        {0.5, 0.5, 0},
        {0.5, -0.5, 0},
    };

    glm::vec4 colors[] = {
        {0.5, 1, 1, 1},
        {0.5, 1, 1, 1},
        {1, 1, 0.5, 1},
        {1, 1, 0.5, 1},
    };

    glm::vec2 uvs[] = {
        {0, 1},
        {0, 0},
        {1, 0},
        {1, 1},
    };

    index_t triIndices[] = {0, 1, 2, 2, 3, 0};

    _quad.reset(new Mesh(m_pDevice.get()));
    _quad->SetVertices(verts, 4);
    _quad->SetColors(colors, 4);
    _quad->SetUV1s(uvs, 4);
    _quad->SetIndices(triIndices, 6);
    _quad->SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    assert(_quad->Apply());
    
    // vertex color shader program
    _vertColorProgram = AssetsManager::LoadProgram("shaders/vertex_color.vert", "shaders/vertex_color.frag");

    _vkLogoTex.reset(new Texture2D(m_pDevice.get()));
    _vkLogoTex->LoadFromFile("textures/VulkanCar_678x452.jpg");

    _quad_mat.reset(new Material(_vertColorProgram));
    _quad_mat->SetTexture("_mainTex", _vkLogoTex.get());
    //_quad_mat->SetCullFace(VK_CULL_MODE_NONE);
    _quad_mat->SetColorBlendMode(BlendMode::Multiply);
    
    // triangle pipeline
    float w = m_window->GetDesc().windowWidth;
    float h = m_window->GetDesc().windowHeight;
    _quadPipeline = PipelineManager::Request(_quad.get(), _quad_mat.get(), _renderPass.get());

    //_quadPipeline->VSSetViewport({0.f, h, w, -h});
   // _quadPipeline->VSSetScissor({0.f, 0.f, w, h});
   //_quadPipeline->FBDisableBlend(0);
    //_quadPipeline->RSSetCullFace(VK_CULL_MODE_BACK_BIT);
    //_quadPipeline->RSSetFrontFaceOrder(VK_FRONT_FACE_CLOCKWISE);
    //assert(_quadPipeline->Create());

    /*
    _quadSet = DescriptorSetManager::AllocDescriptorSet(PerMaterial, DescriptorSetManager::DefaultProgramSetHash(_vertColorProgram));
    VkDescriptorImageInfo texInfo{_vkLogoTex->GetSampler(), _vkLogoTex->GetView(), _vkLogoTex->GetLayout()};
    VkWriteDescriptorSet texWriteSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    texWriteSet.dstSet = _quadSet;
    texWriteSet.dstBinding = 0;
    texWriteSet.dstArrayElement = 0;
    texWriteSet.descriptorCount = 1;
    texWriteSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    texWriteSet.pImageInfo = &texInfo;
    vkUpdateDescriptorSets(m_pDevice->GetHandle(), 1, &texWriteSet, 0, nullptr);
*/

    float texAspectRatio = _vkLogoTex->GetWidth() / (float)_vkLogoTex->GetHeight();
    glm::mat4 M{1.f};
    M = glm::scale(M, glm::vec3(texAspectRatio, 1, 1));
    M = glm::translate(M, {-1, 0, 0});
    _quadInstanceData.reset(new PerObjectData(m_pDevice.get()));
    _quadInstanceData->modelMatrix = M;
    _quadInstanceData->invModelMatrix = glm::inverse(_quadInstanceData->modelMatrix);
    _quadInstanceData->UpdateDataBuffer();
}

void ApiSample::_clean_up_quad()
{
    _quad_mat = nullptr;
    _vkLogoTex->Release();
   PipelineManager::Release(_quadPipeline);
    _quad->Release();
    _quadInstanceData->Release();
}
 
void ApiSample::_set_up_sky_box()
{
    
    _skyboxProgram = AssetsManager::LoadProgram("shaders/skybox.vert", "shaders/skybox.frag");

    _skyboxTex.reset(new TextureCube(m_pDevice.get()));
    assert(_skyboxTex->LoadFromFiles("textures/sunset_sky/sunset_neg_x.PNG",
                                     "textures/sunset_sky/sunset_pos_x.PNG",
                                     "textures/sunset_sky/sunset_neg_y.PNG",
                                     "textures/sunset_sky/sunset_pos_y.PNG",
                                     "textures/sunset_sky/sunset_neg_z.PNG",
                                     "textures/sunset_sky/sunset_pos_z.PNG"));

    glm::vec3 cubeVerts[] = {
        {-0.5, -0.5, -0.5},
        {0.5, -0.5, -0.5},
        {0.5, 0.5, -0.5},
        {-0.5, 0.5, -0.5}, // front

        {0.5, -0.5, 0.5},
        {0.5, -0.5, -0.5},
        {-0.5, -0.5, -0.5},
        {-0.5, -0.5, 0.5}, // bottom

        {-0.5, -0.5, 0.5},
        {-0.5, 0.5, 0.5},
        {0.5, 0.5, 0.5},
        {0.5, -0.5, 0.5}, // back

        {-0.5, -0.5, -0.5},
        {-0.5, 0.5, -0.5},
        {-0.5, 0.5, 0.5},
        {-0.5, -0.5, 0.5}, // left

        {0.5, -0.5, 0.5},
        {0.5, 0.5, 0.5},
        {0.5, 0.5, -0.5},
        {0.5, -0.5, -0.5}, // right

        
        {-0.5, 0.5, -0.5},
        {0.5, 0.5, -0.5},
        {0.5, 0.5, 0.5},
        {-0.5, 0.5, 0.5}, // top
    };

    glm::vec4 cubeColors[] = {
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},

        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},

        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},

        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},

        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},

        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},
        {1, 1, 1, 1},

    };

    glm::vec2 cubeUVs[] = {
        {1, 1},
        {0, 1},
        {0, 0},
        {1, 0},

        {0, 0},
        {0, 1},
        {1, 1},
        {1, 0},

        {0, 1},
        {0, 0},
        {1, 0},
        {1, 1},

        {0, 1},
        {0, 0},
        {1, 0},
        {1, 1},

        {0, 1},
        {0, 0},
        {1, 0},
        {1, 1},

        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1},
    };

    index_t cubeIndices[] = {
        0, 1, 2,
        2, 3, 0,

        4, 5, 6,
        6, 7, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20,
    };

    _cube.reset(new Mesh(m_pDevice.get()));
    _cube->SetVertices(cubeVerts, 24);
    _cube->SetUV1s(cubeUVs, 24);
    _cube->SetIndices(cubeIndices, 36);
    _cube->SetColors(cubeColors, 24);
    _cube->SetTopology(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    assert(_cube->Apply());

    _skyboxMat.reset(new Material(_skyboxProgram));
    _skyboxMat->SetTextureCube("_SkyCubeTex", _skyboxTex.get());
    _skyboxMat->SetCullFace(VK_CULL_MODE_FRONT_BIT);
    _skyboxMat->DisableZWrite();

    float w = m_window->GetDesc().windowWidth;
    float h = m_window->GetDesc().windowHeight;
    _skyboxPipeline = PipelineManager::Request(_cube.get(), _skyboxMat.get(), _renderPass.get());
    //_skyboxPipeline->VSSetScissor({0.f, 0.f, w, h});
    //_skyboxPipeline->RSSetCullFace(VK_CULL_MODE_FRONT_BIT);
    //_skyboxPipeline->RSSetFrontFaceOrder(VK_FRONT_FACE_CLOCKWISE);
    //_skyboxPipeline->FBDisableBlend(0);
    //_skyboxPipeline->DSDisableZWrite();
    //assert(_skyboxPipeline->Create());

    /*
    _skyboxSet = DescriptorSetManager::AllocDescriptorSet(PerMaterial, DescriptorSetManager::DefaultProgramSetHash(_skyboxProgram));
    VkDescriptorImageInfo skyboxTexInfo{_skyboxTex->GetSampler(), _skyboxTex->GetView(), _skyboxTex->GetLayout()};
    VkWriteDescriptorSet skyboxTexWriteInfo{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    skyboxTexWriteInfo.dstSet = _skyboxSet;
    skyboxTexWriteInfo.dstBinding = 0;
    skyboxTexWriteInfo.dstArrayElement = 0;
    skyboxTexWriteInfo.descriptorCount = 1;
    skyboxTexWriteInfo.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    skyboxTexWriteInfo.pImageInfo = &skyboxTexInfo;
    vkUpdateDescriptorSets(m_pDevice->GetHandle(), 1, &skyboxTexWriteInfo, 0, nullptr);
    */
    glm::mat4 M{1.f};
    M = glm::scale(M, {1, 1, 1});
    //M = glm::translate(M, {1, 0, 0});
    _skyboxInstanceData.reset(new PerObjectData(m_pDevice.get()));
    _skyboxInstanceData->modelMatrix = M;
    _skyboxInstanceData->invModelMatrix = glm::inverse(M);
    _skyboxInstanceData->UpdateDataBuffer();

}

void ApiSample::_clean_up_sky_box()
{
    _skyboxMat = nullptr;
    PipelineManager::Release(_skyboxPipeline);
    _skyboxInstanceData->Release();
    _skyboxTex->Release();
    _cube->Release();
}


