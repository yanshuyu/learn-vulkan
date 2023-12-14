#include "DemoApplication.h"

int main(int, char**)
{
    DemoApplication demo{"Vulkan", 800, 600};
    
    if (!demo.Init())
        exit(EXIT_FAILURE);
      
    demo.Run();

    demo.ShutDown();

    exit(EXIT_SUCCESS);
}
