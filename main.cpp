#include "DemoApplication.h"

int main(int, char**)
{
    DemoApplication demo{"Vulkan", 800, 600};
    
    if (!demo.Init())
        exit(1);
      
    demo.Run();

    demo.ShutDown();

    exit(0);
}
