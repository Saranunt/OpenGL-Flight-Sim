#include "app/PlaneApplication.h"

// Thin entry point that forwards to the high-level application object.



int main()
{
    plane::app::PlaneApplication app;



    if (!app.Initialize())
    {
        return -1;
    }


    app.Run();
    app.Shutdown();
    return 0;
}
 