#pragma once
#include<core\CoreUtils.h>
#include<rendering\DescriptorSetManager.h>
#include<glm\glm.hpp>
#include<vector>


class Buffer;
class Device;



struct PerFrameData 
{
public:
    float detalTime{0};
    float detalTimeOver10{0};
    float totalTime{0};
    float sinTotalTime{0};

    Device* pDevice{nullptr};
    Buffer* dataUbo{nullptr};
    uint8_t* mapedData{nullptr}; 

    VkDescriptorSet dataSet{VK_NULL_HANDLE};
    static VkPipelineLayout sPipelineLayout;
    

    NONE_COPYABLE_NONE_MOVEABLE(PerFrameData)

    PerFrameData(Device* device);
    ~PerFrameData();

    static void Initailize(Device* pdevice);
    static void DeInitailize(Device* pdevice);

    void UpdateDataBuffer();
    void Release();

};



struct PerCameraData 
{
public:
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::mat4 viewProjectionMatrix;
    glm::mat4 invViewMatrix;
    glm::mat4 invViewProjectionMatrix;

    Device* pDevice{nullptr};
    Buffer* dataUbo{nullptr};
    uint8_t* mapedData{nullptr}; 

    VkDescriptorSet dataSet{VK_NULL_HANDLE};
    static VkPipelineLayout sPipelineLayout;
    

    NONE_COPYABLE_NONE_MOVEABLE(PerCameraData)

    PerCameraData(Device* device);
    ~PerCameraData();

    void UpdateDataBuffer();
    void Release();

    static void Initailize(Device* pdevice);
    static void DeInitailize(Device* pdevice);

};



struct PerObjectData 
{
public:
    glm::mat4 modelMatrix;
    glm::mat4 invModelMatrix;

    Device* pDevice{nullptr};
    Buffer* dataUbo{nullptr};
    uint8_t* mapedData{nullptr}; 

    VkDescriptorSet dataSet{VK_NULL_HANDLE};
 
    NONE_COPYABLE_NONE_MOVEABLE(PerObjectData)

    PerObjectData(Device* device);
    ~PerObjectData();

    static void Initailize(Device* pdevice);
    static void DeInitailize(Device* pdevice);

    void UpdateDataBuffer();
    void Release();

};

