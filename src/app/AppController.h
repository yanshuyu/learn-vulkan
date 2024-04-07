#pragma once
#include<memory>
#include<functional>
#include"Application.h"
#include"rendering\Window.h"



class AppController
{
private:
    std::unique_ptr<Window> m_Window;
    std::unique_ptr<Application> m_App;

public:
    AppController() = default;
    ~AppController();
    
    bool Init(const WindowDesc& wndDesc, const AppDesc& appDesc, std::function<Application*(const AppDesc&)> appCreator);
    void MainLoop();
    void Terminate();
};

