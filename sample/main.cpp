
#include<app\AppController.h>
#include<app\Application.h>
#include<core\CoreUtils.h>
#include<core\SwapChain.h>
#include<core\Device.h>
#include<core\VulkanInstance.h>
#include"sample\ApiSample.h"



Application* CreateApiSample(const AppDesc& desc)
{
    return new ApiSample(desc);
}


int main(int, char**)
{
    AppController appController;
    WindowDesc wndDesc{};
    AppDesc appDesc{};

    wndDesc.name = "Vulkan Api Sample";
    wndDesc.resizeble = false;
    wndDesc.windowWidth = 1280;
    wndDesc.windowHeight = 720;

    appDesc.debugEnabled = true;
    appDesc.enableQueueOperation = VK_QUEUE_GRAPHICS_BIT;
    appDesc.swapChainColorFormat = VK_FORMAT_B8G8R8A8_UNORM;
    appDesc.swapChainColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    appDesc.swapChainTrippleBufferEnabled = true;
    appDesc.backBufferClearColor[0] = 0;
    appDesc.backBufferClearColor[1] = 0;
    appDesc.backBufferClearColor[2] = 1;
    appDesc.backBufferClearColor[3] = 1;

    try
    {
        if(!appController.Init(wndDesc, appDesc, CreateApiSample))
        {
            throw std::runtime_error("Init  Failed, unable to Run!");
        }

        appController.MainLoop();

    }
    catch(const std::exception& e)
    {
         LOGE("-->Runing App Error: {}", e.what());

        appController.Terminate();

        exit(1);   
    }

    
    appController.Terminate();
    
    LOGI("-->Exit All Sucess.");

    exit(0);

}
