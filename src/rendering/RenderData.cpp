#include"RenderData.h"
#include<core\Device.h>
#include<rendering\DescriptorSetManager.h>

//*************************************************************************Per Frame Data*******************************************************************************

PerFrameData::PerFrameData(Device* device)
: pDevice(device)
{
    assert(device && device->IsValid());
    assert(VKHANDLE_IS_NOT_NULL(sPipelineLayout));

    // alloc buffer
    dataUbo = pDevice->CreateBuffer(sizeof(PerFrameData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    // alloc descriptor set    
    dataSet = DescriptorSetManager::AllocDescriptorSet(PerFrame, 0);
    assert(VKHANDLE_IS_NOT_NULL(dataSet));

    // set up set & resource connection
    VkDescriptorBufferInfo dstBuf{};
    dstBuf.buffer = dataUbo->GetHandle();
    dstBuf.offset = 0;
    dstBuf.range = dataUbo->GetMemorySize();
    
    VkWriteDescriptorSet uboWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    uboWrite.dstSet = dataSet;
    uboWrite.dstBinding = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &dstBuf;
    vkUpdateDescriptorSets(pDevice->GetHandle(), 1, &uboWrite, 0, nullptr);

}

PerFrameData::~PerFrameData()
{
    Release();
}


void PerFrameData::Initailize(Device* pdevice)
{
    if(VKHANDLE_IS_NOT_NULL(sPipelineLayout))
        return;

    std::vector<VkDescriptorSetLayoutBinding> setBindings{};
    setBindings.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
    DescriptorSetManager::RegisterSetLayout(PerFrame, 0, setBindings);
    VkDescriptorSetLayout setlayout = DescriptorSetManager::GetSetLayout(PerFrame, 0);

    // create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &setlayout;
    assert(VKCALL_SUCCESS(vkCreatePipelineLayout(pdevice->GetHandle(), &pipelineLayoutCreateInfo, nullptr, &sPipelineLayout)));
}


void PerFrameData::DeInitailize(Device* pdevice)
{
    if (VKHANDLE_IS_NULL(sPipelineLayout))
        return;
    
    vkDestroyPipelineLayout(pdevice->GetHandle(), sPipelineLayout, nullptr);
    DescriptorSetManager::ResetDescriptorPool(PerFrame, 0);
}



void PerFrameData::Release()
{

    if (VKHANDLE_IS_NOT_NULL(dataSet))
    {
        DescriptorSetManager::FreeDescriptorSet(PerFrame, 0, dataSet);
        VKHANDLE_SET_NULL(dataSet);
    }
    
    if (dataUbo)
    {
        dataUbo->UnMap();
        pDevice->DestroyBuffer(dataUbo);
        dataUbo = nullptr;
    }
}


void PerFrameData::UpdateDataBuffer()
{
    if (dataUbo->IsMapped())
        mapedData = dataUbo->Map(Write);

    memcpy(mapedData, this, sizeof(PerFrameData));
}


//*************************************************************************Per Camera Data*******************************************************************************

PerCameraData::PerCameraData(Device* device)
: pDevice(device)
{
    assert(device && device->IsValid());
    assert(VKHANDLE_IS_NOT_NULL(sPipelineLayout));
    
    // alloc buffer
    dataUbo = pDevice->CreateBuffer(sizeof(PerCameraData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    // alloc descriptor set    
    dataSet = DescriptorSetManager::AllocDescriptorSet(PerCamera, 0);
    assert(VKHANDLE_IS_NOT_NULL(dataSet));

    // set up set & resource connection
    VkDescriptorBufferInfo dstBuf{};
    dstBuf.buffer = dataUbo->GetHandle();
    dstBuf.offset = 0;
    dstBuf.range = dataUbo->GetMemorySize();
    
    VkWriteDescriptorSet uboWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    uboWrite.dstSet = dataSet;
    uboWrite.dstBinding = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &dstBuf;
    vkUpdateDescriptorSets(pDevice->GetHandle(), 1, &uboWrite, 0, nullptr); 
}

PerCameraData::~PerCameraData()
{
    Release();
}


void PerCameraData::Initailize(Device* pdevice)
{
    if(VKHANDLE_IS_NOT_NULL(sPipelineLayout))
        return;

    std::vector<VkDescriptorSetLayoutBinding> setBindings{};
    setBindings.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
    DescriptorSetManager::RegisterSetLayout(PerCamera, 0, setBindings);
    VkDescriptorSetLayout setlayouts[2];
    setlayouts[0] = DescriptorSetManager::GetSetLayout(PerFrame, 0);
    setlayouts[1] = DescriptorSetManager::GetSetLayout(PerCamera, 0);

    // create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutCreateInfo.setLayoutCount = 2;
    pipelineLayoutCreateInfo.pSetLayouts = setlayouts;
    assert(VKCALL_SUCCESS(vkCreatePipelineLayout(pdevice->GetHandle(), &pipelineLayoutCreateInfo, nullptr, &sPipelineLayout)));
}


void PerCameraData::DeInitailize(Device* pdevice)
{
    if (VKHANDLE_IS_NULL(sPipelineLayout))
        return;
    
    vkDestroyPipelineLayout(pdevice->GetHandle(), sPipelineLayout, nullptr);
    DescriptorSetManager::ResetDescriptorPool(PerCamera, 0);
}



void PerCameraData::Release()
{

    if (VKHANDLE_IS_NOT_NULL(dataSet))
    {
        DescriptorSetManager::FreeDescriptorSet(PerFrame, 0, dataSet);
        VKHANDLE_SET_NULL(dataSet);
    }
    
    if (dataUbo)
    {
        dataUbo->UnMap();
        pDevice->DestroyBuffer(dataUbo);
        dataUbo = nullptr;
    }
}


void PerCameraData::UpdateDataBuffer()
{
    if (dataUbo->IsMapped())
        mapedData = dataUbo->Map(Write);

    memcpy(mapedData, this, sizeof(PerCameraData));
}



//*************************************************************************Per Object Data*******************************************************************************


PerObjectData::PerObjectData(Device* device)
: pDevice(device)
{
    assert(device && device->IsValid());
    
    // alloc buffer
    dataUbo = pDevice->CreateBuffer(sizeof(PerObjectData), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    
    // alloc descriptor set    
    dataSet = DescriptorSetManager::AllocDescriptorSet(PerObject, 0);
    assert(VKHANDLE_IS_NOT_NULL(dataSet));

    // set up set & resource connection
    VkDescriptorBufferInfo dstBuf{};
    dstBuf.buffer = dataUbo->GetHandle();
    dstBuf.offset = 0;
    dstBuf.range = dataUbo->GetMemorySize();
    
    VkWriteDescriptorSet uboWrite{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
    uboWrite.dstSet = dataSet;
    uboWrite.dstBinding = 0;
    uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboWrite.dstArrayElement = 0;
    uboWrite.descriptorCount = 1;
    uboWrite.pBufferInfo = &dstBuf;
    vkUpdateDescriptorSets(pDevice->GetHandle(), 1, &uboWrite, 0, nullptr);
  
}

PerObjectData::~PerObjectData()
{
    Release();
}


void PerObjectData::Initailize(Device* pdevice)
{
    if (DescriptorSetManager::GetSetLayout(PerObject, 0) == VK_NULL_HANDLE)
    {
        std::vector<VkDescriptorSetLayoutBinding> setBindings{};
        setBindings.push_back({0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr});
        DescriptorSetManager::RegisterSetLayout(PerObject, 0, setBindings, true, 8);
    }
}


void PerObjectData::DeInitailize(Device* pdevice)
{
    DescriptorSetManager::ResetDescriptorPool(PerObject, 0);
}



void PerObjectData::Release()
{

    if (VKHANDLE_IS_NOT_NULL(dataSet))
    {
        DescriptorSetManager::FreeDescriptorSet(PerObject, 0, dataSet);
        VKHANDLE_SET_NULL(dataSet);
    }
    
    if (dataUbo)
    {
        dataUbo->UnMap();
        pDevice->DestroyBuffer(dataUbo);
        dataUbo = nullptr;
    }
}


void PerObjectData::UpdateDataBuffer()
{
    if (dataUbo->IsMapped())
        mapedData = dataUbo->Map(Write);

    memcpy(mapedData, this, sizeof(PerObjectData));
}


