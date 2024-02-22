
#include"AppController.h"
#include"Application.h"
#include"core\CoreUtils.h"
#include"core\SwapChain.h"
#include"core\Device.h"
#include"core\VulkanInstance.h"
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
