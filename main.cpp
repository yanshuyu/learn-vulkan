#include<memory>
#include"core\CoreUtils.h"
#include "Application.h"



std::unique_ptr<Application> CreateDemo()
{
    return std::move(std::unique_ptr<Application>(new Application("Vukan", 800, 600)));
}


int main(int, char**)
{
    auto app = CreateDemo();
    try
    {
        app->Init();
        
        app->Run();

    }
    catch(const std::exception& e)
    {
        LOGE("Demo Application Exception: {}", e.what());
        app->ShutDown();
        exit(EXIT_FAILURE);
    }
    
    app->ShutDown();

    exit(EXIT_SUCCESS);
}
