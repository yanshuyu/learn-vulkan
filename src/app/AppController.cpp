#include"AppController.h"
#include"core\SwapChain.h"
#include"core\Device.h"
#include"core\VulkanInstance.h"
#include"input\InputManager.h"


AppController::~AppController()
{

}

bool AppController::Init(const WindowDesc& wndDesc, const AppDesc& appDesc, std::function<Application*(const AppDesc&)> appCreator)
{
    m_Window.reset(new Window(wndDesc));
    m_App.reset(appCreator(appDesc));
    return m_App->Prepare(m_Window.get());
}

void AppController::MainLoop()
{
    while (!m_Window->ShouldClose())
    {
        m_App->Step();
        InputManager::Reset();
        m_Window->ProcessEvent();
    }
}


void AppController::Terminate()
{
    m_App->Finish();
    m_App.reset(nullptr);
    m_Window.reset(nullptr);
}