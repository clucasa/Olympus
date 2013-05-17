#include "System.h"

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    System systemClass(hInstance, nCmdShow);

    if(!systemClass.init())
        return 0;

    return systemClass.run();
}