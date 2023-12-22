#include "DemoApplication.h"
#include<iostream>

int main(int, char**)
{
    DemoApplication demo{"Vulkan", 800, 600};
    try
    {
        demo.Init();
        
        demo.Run();

    }
    catch(const std::exception& e)
    {
        std::cerr << "-->Application get Exception: \n" << e.what() << '\n'; 
        exit(EXIT_FAILURE);
    }
    
    demo.ShutDown();


    exit(EXIT_SUCCESS);
}
