#include "statscollector.h"
#include <windows.h>
#include <thread>

#include <iostream>

static StatsCollector stats_c;

BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, PVOID fImpLoad)
{

    switch(fdwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        stats_c.server_addr = "http://tpmodstat.16mb.com/connect.php?";
        std::thread thr(&StatsCollector::start, &stats_c);
        thr.detach();
    }
        break;
    case DLL_PROCESS_DETACH:
            stats_c.stop = true;
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    default:
        break;
    }
    return TRUE;
}


__declspec(dllexport)void my_func()
{
}
